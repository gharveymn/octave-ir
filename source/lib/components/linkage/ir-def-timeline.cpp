/** ir-def-timeline.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "components/linkage/ir-def-timeline.hpp"

#include "components/ir-block.hpp"
#include "utilities/ir-error.hpp"
#include "values/ir-instruction.hpp"

namespace gch
{

  ir_def_timeline::
  ir_def_timeline (ir_block& block, ir_variable& var) noexcept
    : m_incoming (incoming_alloc_t (*this)),
      m_block (block),
      m_var   (var)
  { }

  ir_def_timeline::
  ir_def_timeline (ir_block& block, ir_block& incoming_block, ir_def_timeline& pred)
    : m_incoming (incoming_alloc_t (*this)),
      m_block    (block),
      m_var      (pred.get_variable ())
  {
    append_incoming (incoming_block, pred);
  }

  void
  ir_def_timeline::
  set_block (ir_block& block) noexcept
  {
    m_block.emplace (block);
  }

  ir_def_timeline::
  ir_def_timeline (ir_def_timeline&& other) noexcept
    : successors_tracker (std::move (other)),
      m_block            (other.m_block),
      m_var              (other.m_var),
      m_incoming         (std::move (other.m_incoming), incoming_alloc_t (*this)),
      m_use_timelines    (std::move (other.m_use_timelines))
  { }

  ir_def_timeline&
  ir_def_timeline::
  operator= (ir_def_timeline&& other) noexcept
  {
    if (this == &other)
      return *this;

    // m_block stays the same
    m_var           = other.m_var;
    m_incoming      = std::move (other.m_incoming);
    m_use_timelines = std::move (other.m_use_timelines);
    successors_tracker::operator= (std::move (other));

    return *this;
  }

  template <>
  void
  ir_def_timeline::
  splice<ir_instruction_range::phi> (uts_citer, ir_def_timeline&) = delete;

  template <>
  void
  ir_def_timeline::
  splice<ir_instruction_range::body> (uts_citer pos, ir_def_timeline& other)
  {
    get_use_timelines<range::body> ().splice (pos, other.get_use_timelines<range::body> ());
  }

  template <>
  void
  ir_def_timeline::
  splice<ir_instruction_range::phi> (uts_citer, ir_def_timeline&, uts_citer, uts_citer) = delete;

  template <>
  void
  ir_def_timeline::
  splice<ir_instruction_range::body> (uts_citer pos, ir_def_timeline& other,
                                      uts_citer first, uts_citer last)
  {
    get_use_timelines<range::body> ().splice (pos, other.get_use_timelines<range::body> (),
                                              first, last);
  }

  template <>
  auto
  ir_def_timeline::
  emplace_before<ir_instruction_range::phi> (uts_citer, ir_instruction_iter) -> uts_iter = delete;

  template <>
  auto
  ir_def_timeline::
  emplace_before<ir_instruction_range::body> (const uts_citer pos,
                                              const ir_instruction_iter instr_pos)
    -> uts_iter
  {
    return get_use_timelines<range::body> ().emplace (pos, instr_pos);
  }

  template <>
  ir_use_timeline&
  ir_def_timeline::
  emplace_back<ir_instruction_range::phi> (ir_instruction_iter) = delete;

  template <>
  ir_use_timeline&
  ir_def_timeline::
  emplace_back<ir_instruction_range::body> (const ir_instruction_iter instructions_pos)
  {
    return get_use_timelines<range::body> ().emplace_back (instructions_pos);
  }

  template <>
  auto
  ir_def_timeline::
  erase<ir_instruction_range::phi> (uts_citer) -> uts_iter = delete;

  template <>
  auto
  ir_def_timeline::
  erase<ir_instruction_range::body> (uts_citer pos)
    -> uts_iter
  {
    return get_use_timelines<range::body> ().erase (pos);
  }

  template <>
  auto
  ir_def_timeline::
  erase<ir_instruction_range::phi> (uts_citer, uts_citer) -> uts_iter = delete;

  template <>
  auto
  ir_def_timeline::
  erase<ir_instruction_range::body> (const uts_citer first, const uts_citer last)
    -> uts_iter
  {
    return get_use_timelines<range::body> ().erase (first, last);
  }

  void
  ir_def_timeline::transfer_successors (ir_def_timeline& src)
  {
    successors_tracker::replace_bindings (std::move (src));
  }

  void
  ir_def_timeline::
  remove_incoming (incoming_citer pos)
  {
    m_incoming.erase (pos);
    if (m_incoming.empty ())
      destroy_incoming_timeline ();
  }

  auto
  ir_def_timeline::
  find_incoming (const ir_block& block) noexcept
    -> incoming_iter
  {
    return m_incoming.find (nonnull_ptr { block });
  }

  auto
  ir_def_timeline::
  find_incoming (const ir_block& block) const noexcept
    -> incoming_const_iterator
  {
    return as_mutable (*this).find_incoming (block);
  }

  auto
  ir_def_timeline::
  add_successor (ir_incoming_node& remote)
    -> succs_iter
  {
    return bind (remote);
  }

  auto
  ir_def_timeline::
  remove_successor (ir_incoming_node& node)
    -> succs_iter
  {
    auto find_pred = [&](const ir_incoming_node& e) { return &e == &node; };
    return successors_tracker::erase (std::find_if (successors_begin (), successors_end (),
                                                    find_pred));
  }

  [[nodiscard]]
  ir_instruction_iter
  ir_def_timeline::
  instructions_begin (uts_iter pos) noexcept
  {
    return pos->get_def_pos ();
  }

  [[nodiscard]]
  ir_instruction_citer
  ir_def_timeline::
  instructions_begin (uts_citer pos) noexcept
  {
    return pos->get_def_pos ();
  }

  [[nodiscard]]
  ir_instruction_citer
  ir_def_timeline::
  instructions_cbegin (uts_citer pos) noexcept
  {
    return instructions_begin (pos);
  }

  [[nodiscard]]
  ir_instruction_iter
  ir_def_timeline::
  instructions_end (uts_iter pos) const noexcept
  {
    if (pos != use_timelines_cend<range::body> ())
      return std::next (pos)->get_def_pos ();
    return m_block->end<range::body> ();
  }

  [[nodiscard]]
  ir_instruction_citer
  ir_def_timeline::
  instructions_end (uts_citer pos) const noexcept
  {
    if (pos != use_timelines_cend<range::body> ())
      return std::next (pos)->get_def_pos ();
    return m_block->cend<range::body> ();
  }

  [[nodiscard]]
  ir_instruction_citer
  ir_def_timeline::
  instructions_cend (uts_citer pos) const noexcept
  {
    return instructions_end (pos);
  }

  [[nodiscard]]
  ir_instruction_riter
  ir_def_timeline::
  instructions_rbegin (uts_iter pos) const noexcept
  {
    return ir_instruction_riter  { instructions_end (pos) };
  }

  [[nodiscard]]
  ir_instruction_criter
  ir_def_timeline::
  instructions_rbegin (uts_citer pos) const noexcept
  {
    return ir_instruction_criter  { instructions_cend (pos) };
  }

  [[nodiscard]]
  ir_instruction_criter
  ir_def_timeline::
  instructions_crbegin (uts_citer pos) const noexcept
  {
    return instructions_rbegin (pos);
  }

  [[nodiscard]]
  ir_instruction_riter
  ir_def_timeline::
  instructions_rend (uts_iter pos) noexcept
  {
    return ir_instruction_riter  { instructions_begin (pos) };
  }

  [[nodiscard]]
  ir_instruction_criter
  ir_def_timeline::
  instructions_rend (uts_citer pos) noexcept
  {
    return ir_instruction_criter  { instructions_cbegin (pos) };
  }

  [[nodiscard]]
  ir_instruction_criter
  ir_def_timeline::
  instructions_crend (uts_citer pos) noexcept
  {
    return instructions_rend (pos);
  }

  ir_instruction&
  ir_def_timeline::
  instructions_front (uts_iter pos) noexcept
  {
    return *instructions_begin (pos);
  }

  const ir_instruction&
  ir_def_timeline::
  instructions_front (uts_citer pos) noexcept
  {
    return *instructions_begin (pos);
  }

  ir_instruction&
  ir_def_timeline::
  instructions_back (uts_iter pos) const noexcept
  {
    return *instructions_rbegin (pos);
  }

  const ir_instruction&
  ir_def_timeline::
  instructions_back (uts_citer pos) const noexcept
  {
    return *instructions_rbegin (pos);
  }

  ir_use_timeline::iter
  ir_def_timeline::
  find_first_after (const uts_iter ut_it, const ir_instruction_citer pos) const noexcept
  {
    // we reverse iterate because the instruction is more likely to
    // be near the back of the timeline.
    // note: multiple uses may be associated to the same instruction
    //       so we need to make sure we iterate to one past that
    //       instruction, rather than just find the first instance
    //       of that instruction.
    auto rcurr = ut_it->rbegin ();
    for (auto it = std::prev (instructions_end (ut_it)); it != pos && rcurr != ut_it->rend (); --it)
    {
      while (&(*it) == &rcurr->get_instruction ())
        ++rcurr;
    }
    return rcurr.base ();
  }

  ir_use_timeline::citer
  ir_def_timeline::
  find_first_after (const uts_citer ut_it, const ir_instruction_citer pos) const noexcept
  {
    auto rcurr = ut_it->rbegin ();
    for (auto it = std::prev (instructions_end (ut_it));
         it != pos && rcurr != ut_it->rend ();
         --it)
    {
      while (&(*it) == &rcurr->get_instruction ())
        ++rcurr;
    }
    return rcurr.base ();
  }

  std::ptrdiff_t
  ir_def_timeline::
  num_instructions (uts_citer pos) const noexcept
  {
    return std::distance (instructions_begin (pos), instructions_end (pos));
  }

  bool
  ir_def_timeline::
  has_instructions (uts_citer pos) const noexcept
  {
    return instructions_begin (pos) == instructions_end (pos);
  }

  bool
  ir_def_timeline::
  has_timelines (void) const noexcept
  {
    return ! use_timelines_empty<range::all> ();
  }

  bool
  ir_def_timeline::
  has_incoming_timeline (void) const noexcept
  {
    return ! use_timelines_empty<range::phi> ();
  }

  bool
  ir_def_timeline::
  has_local_timelines (void) const noexcept
  {
    return ! use_timelines_empty<range::body> ();
  }

  ir_use_timeline&
  ir_def_timeline::
  get_incoming_timeline (void) noexcept
  {
    return use_timelines_front<range::phi> ();
  }

  const ir_use_timeline&
  ir_def_timeline::
  get_incoming_timeline (void) const noexcept
  {
    return as_mutable (*this).get_incoming_timeline ();
  }

  optional_ref<ir_use_timeline>
  ir_def_timeline::
  maybe_get_incoming_timeline (void) noexcept
  {
    if (has_incoming_timeline ())
      return optional_ref { get_incoming_timeline () };
    return nullopt;
  }

  optional_ref<const ir_use_timeline>
  ir_def_timeline::
  maybe_get_incoming_timeline (void) const noexcept
  {
    return as_mutable (*this).maybe_get_incoming_timeline ();
  }

  [[nodiscard]]
  optional_ref<ir_instruction>
  ir_def_timeline::
  maybe_get_incoming_instruction (void) noexcept
  {
    return maybe_get_incoming_timeline () >>= &ir_use_timeline::get_def_instruction;
  }

  optional_ref<const ir_instruction>
  ir_def_timeline::
  maybe_get_incoming_instruction (void) const noexcept
  {
    return as_mutable (*this).maybe_get_incoming_instruction ();
  }

  bool
  ir_def_timeline::
  has_outgoing_timeline (void) const noexcept
  {
    return ! use_timelines_empty<range::all> ();
  }

  ir_use_timeline&
  ir_def_timeline::
  get_outgoing_timeline (void) noexcept
  {
    return use_timelines_back<range::all> ();
  }

  const ir_use_timeline&
  ir_def_timeline::
  get_outgoing_timeline (void) const noexcept
  {
    return as_mutable (*this).get_outgoing_timeline ();
  }

  optional_ref<ir_use_timeline>
  ir_def_timeline::
  maybe_get_outgoing_timeline (void) noexcept
  {
    if (has_timelines ())
      return optional_ref { get_outgoing_timeline () };
    return nullopt;
  }

  optional_cref<ir_use_timeline>
  ir_def_timeline::
  maybe_get_outgoing_timeline (void) const noexcept
  {
    return as_mutable (*this).maybe_get_outgoing_timeline ();
  }

  ir_def&
  ir_def_timeline::
  get_outgoing_def (void) noexcept
  {
    return get_outgoing_timeline ().get_def ();
  }

  const ir_def&
  ir_def_timeline::
  get_outgoing_def (void) const noexcept
  {
    return as_mutable (*this).get_outgoing_def ();
  }

  optional_ref<ir_def>
  ir_def_timeline::
  maybe_get_outgoing_def (void) noexcept
  {
    return maybe_get_outgoing_timeline () >>= &ir_use_timeline::get_def;
  }

  optional_cref<ir_def>
  ir_def_timeline::
  maybe_get_outgoing_def (void) const noexcept
  {
    return as_mutable (*this).maybe_get_outgoing_def ();
  }

  ir_block&
  ir_def_timeline::
  get_block (void) noexcept
  {
    return *m_block;
  }

  const ir_block&
  ir_def_timeline::
  get_block (void) const noexcept
  {
    return as_mutable (*this).get_block ();
  }

  ir_variable&
  ir_def_timeline::
  get_variable (void) noexcept
  {
    return *m_var;
  }

  const ir_variable&
  ir_def_timeline::
  get_variable (void) const noexcept
  {
    return as_mutable (*this).get_variable ();
  }

  [[nodiscard]]
  std::size_t
  ir_def_timeline::
  num_timelines (void) const noexcept
  {
    return get_use_timelines<range::all> ().size ();
  }

  ir_use_timeline&
  ir_def_timeline::
  create_incoming_timeline (void)
  {
    ir_instruction_iter phi_it = m_block->create_phi (*m_var);
    try
    {
      return get_use_timelines<range::phi> ().emplace_front (phi_it);
    }
    catch (...)
    {
      m_block->erase_phi (*m_var);
      throw;
    }
  }

  void
  ir_def_timeline::
  destroy_incoming_timeline (void)
  {
    if (use_timelines_empty<range::phi> ())
      throw ir_exception ("could not find phi in def timeline");
    get_use_timelines<range::phi> ().clear ();
    m_block->erase_phi (*m_var);
  }

}

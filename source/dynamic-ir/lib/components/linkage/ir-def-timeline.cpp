/** ir-def-timeline.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "components/linkage/ir-def-timeline.hpp"

#include "components/ir-block.hpp"

namespace gch
{

  ir_def_timeline::
  ir_def_timeline (ir_block& block, ir_variable& var) noexcept
    : m_block (block),
      m_var   (var),
      m_incoming (incoming_alloc_t (*this))
  { }

  ir_def_timeline::
  ir_def_timeline (ir_block& block, ir_block& incoming_block, ir_def_timeline& pred)
    : m_block    (block),
      m_var      (pred.get_variable ()),
      m_incoming (incoming_alloc_t (*this))
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

  void
  ir_def_timeline::
  splice_local (ut_citer pos, ir_def_timeline& other)
  {
    get_use_timelines<range::local> ().splice (pos, other.get_use_timelines<range::local> ());
  }

  void
  ir_def_timeline::
  splice_local (ut_citer pos, ir_def_timeline& other,
                                      ut_citer first, ut_citer last)
  {
    get_use_timelines<range::local> ().splice (pos, other.get_use_timelines<range::local> (),
                                               first, last);
  }

  auto
  ir_def_timeline::
  emplace_local (const ut_citer pos, const ir_instruction_iter instr_pos)
    -> ut_iter
  {
    return get_use_timelines<range::local> ().emplace (pos, instr_pos);
  }

  ir_use_timeline&
  ir_def_timeline::
  emplace_back_local (const ir_instruction_iter instructions_pos)
  {
    return get_use_timelines<range::local> ().emplace_back (instructions_pos);
  }

  auto
  ir_def_timeline::
  erase_local (ut_citer pos)
    -> ut_iter
  {
    return get_use_timelines<range::local> ().erase (pos);
  }

  auto
  ir_def_timeline::
  erase_local (const ut_citer first, const ut_citer last)
    -> ut_iter
  {
    return get_use_timelines<range::local> ().erase (first, last);
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
  add_successor (ir_incoming_node&& remote)
    -> succs_iter
  {
    return bind (std::move (remote));
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
  instructions_begin (ut_iter pos) noexcept
  {
    return pos->get_def_pos ();
  }

  [[nodiscard]]
  ir_instruction_citer
  ir_def_timeline::
  instructions_begin (ut_citer pos) noexcept
  {
    return pos->get_def_pos ();
  }

  [[nodiscard]]
  ir_instruction_citer
  ir_def_timeline::
  instructions_cbegin (ut_citer pos) noexcept
  {
    return instructions_begin (pos);
  }

  [[nodiscard]]
  ir_instruction_iter
  ir_def_timeline::
  instructions_end (ut_iter pos) const noexcept
  {
    if (pos != local_end ())
      return std::next (pos)->get_def_pos ();
    return m_block->end<ir_block::range::body> ();
  }

  [[nodiscard]]
  ir_instruction_citer
  ir_def_timeline::
  instructions_end (ut_citer pos) const noexcept
  {
    if (pos != local_end ())
      return std::next (pos)->get_def_pos ();
    return m_block->cend<ir_block::range::body> ();
  }

  [[nodiscard]]
  ir_instruction_citer
  ir_def_timeline::
  instructions_cend (ut_citer pos) const noexcept
  {
    return instructions_end (pos);
  }

  [[nodiscard]]
  ir_instruction_riter
  ir_def_timeline::
  instructions_rbegin (ut_riter pos) const noexcept
  {
    return ir_instruction_riter { instructions_end (pos.base ()) };
  }

  [[nodiscard]]
  ir_instruction_criter
  ir_def_timeline::
  instructions_rbegin (ut_criter pos) const noexcept
  {
    return ir_instruction_criter { instructions_cend (pos.base ()) };
  }

  [[nodiscard]]
  ir_instruction_criter
  ir_def_timeline::
  instructions_crbegin (ut_criter pos) const noexcept
  {
    return instructions_rbegin (pos);
  }

  [[nodiscard]]
  ir_instruction_riter
  ir_def_timeline::
  instructions_rend (ut_riter pos) noexcept
  {
    return ir_instruction_riter { instructions_begin (pos.base ()) };
  }

  [[nodiscard]]
  ir_instruction_criter
  ir_def_timeline::
  instructions_rend (ut_criter pos) noexcept
  {
    return ir_instruction_criter { instructions_cbegin (pos.base ()) };
  }

  [[nodiscard]]
  ir_instruction_criter
  ir_def_timeline::
  instructions_crend (ut_criter pos) noexcept
  {
    return instructions_rend (pos);
  }

  ir_instruction&
  ir_def_timeline::
  instructions_front (ut_iter pos) noexcept
  {
    return *instructions_begin (pos);
  }

  const ir_instruction&
  ir_def_timeline::
  instructions_front (ut_citer pos) noexcept
  {
    return *instructions_begin (pos);
  }

  ir_instruction&
  ir_def_timeline::
  instructions_back (ut_iter pos) const noexcept
  {
    return *instructions_rbegin (ut_riter (pos));
  }

  const ir_instruction&
  ir_def_timeline::
  instructions_back (ut_citer pos) const noexcept
  {
    return *instructions_rbegin (ut_criter (pos));
  }

  ir_use_timeline::iter
  ir_def_timeline::
  first_use_after (const ir_instruction_citer pos, const ut_iter ut_it) const noexcept
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
  first_use_after (const ir_instruction_citer pos, const ut_citer ut_it) const noexcept
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
  num_instructions (ut_citer pos) const noexcept
  {
    return std::distance (instructions_begin (pos), instructions_end (pos));
  }

  bool
  ir_def_timeline::
  has_instructions (ut_citer pos) const noexcept
  {
    return instructions_begin (pos) == instructions_end (pos);
  }

  bool
  ir_def_timeline::
  has_timelines (void) const noexcept
  {
    return ! use_timelines_empty ();
  }

  bool
  ir_def_timeline::
  has_incoming_timeline (void) const noexcept
  {
    return ! get_use_timelines<range::incoming> ().empty ();
  }

  bool
  ir_def_timeline::
  has_local_timelines (void) const noexcept
  {
    return ! local_empty ();
  }

  ir_use_timeline&
  ir_def_timeline::
  get_incoming_timeline (void) noexcept
  {
    return get_use_timelines<range::incoming> ().front ();
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
    return has_timelines ();
  }

  ir_use_timeline&
  ir_def_timeline::
  get_outgoing_timeline (void) noexcept
  {
    return use_timelines_back ();
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
    assert (! has_incoming_timeline ());
    ir_instruction_iter phi_it = m_block->create_phi (*m_var);
    try
    {
      return get_use_timelines<range::incoming> ().emplace_front (phi_it);
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
    assert (has_incoming_timeline ());

    ir_instruction_iter phi_it = get_incoming_timeline ().get_def_pos ();

    get_use_timelines<range::incoming> ().clear ();
    m_block->erase_phi (phi_it);
  }

}

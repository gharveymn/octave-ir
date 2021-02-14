/*

Copyright (C) 2019 Gene Harvey

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#include <ir-block.hpp>
#include <ir-type-std.hpp>
#include <ir-variable.hpp>
#include <ir-instruction.hpp>
#include <ir-structure.hpp>
#include <ir-common-util.hpp>

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>
#include <gch/select-iterator.hpp>

#include <algorithm>
#include <numeric>
#include <utility>
#include <list>

namespace gch
{

  // ONLY FOR USE WITH ir_def_timeline
    // fix up m_parent after move!
  ir_incoming_node::
  ir_incoming_node (ir_incoming_node&& other) noexcept
    : base             (std::move (other)),
      m_parent         (other.m_parent),
      m_incoming_block (other.m_incoming_block)
  { }

  ir_incoming_node::
  ir_incoming_node (ir_incoming_node&& other, ir_def_timeline& new_parent) noexcept
    : base             (std::move (other)),
      m_parent         (new_parent),
      m_incoming_block (other.m_incoming_block)
  { }

  ir_incoming_node::
  ir_incoming_node (ir_def_timeline& parent, ir_block& incoming_block,
                                      ir_def_timeline& pred)
    : base             (tag::bind, pred),
      m_parent         (parent),
      m_incoming_block (incoming_block)
  { }

  ir_incoming_node::
  ir_incoming_node (ir_def_timeline& parent, ir_block& incoming_block)
    : m_parent         (parent),
      m_incoming_block (incoming_block)
  { }

  ir_incoming_node&
  ir_incoming_node::operator= (ir_incoming_node&& other) noexcept
  {
    m_incoming_block = other.m_incoming_block;
    base::operator= (std::move (other));
  }

  auto
  ir_incoming_node::remove_predecessor (const ir_def_timeline& dt)
    -> iter
  {
    auto find_pred = [&](const ir_def_timeline& e) { return &e == &dt; };
    return erase (std::find_if (begin (), end (), find_pred));
  }

  ir_def_timeline::ir_def_timeline (ir_def_timeline&& other) noexcept
    : succ_tracker            (std::move (other)),
      m_block                 (other.m_block),
      m_var                   (other.m_var),
      m_incoming              (std::move (other.m_incoming)),
      m_use_timelines         (std::move (other.m_use_timelines))
  {
    std::for_each (m_incoming.begin (), m_incoming.end (),
                   [this](ir_incoming_node& node) { node.set_parent (*this); });
  }

  ir_def_timeline&
  ir_def_timeline::operator= (ir_def_timeline&& other) noexcept
  {
    if (this == &other)
      return *this;

    // m_block stays the same
    m_var           = other.m_var;
    m_incoming      = std::move (other.m_incoming);
    m_use_timelines = std::move (other.m_use_timelines);
    succ_tracker::operator= (std::move (other));

    for (ir_incoming_node& node : m_incoming)
      node.set_parent (*this);

    return *this;
  }

  template <>
  void
  ir_def_timeline::
  splice<ir_instruction_range::phi> (iter, ir_def_timeline&) = delete;

  template <>
  void
  ir_def_timeline::
  splice<ir_instruction_range::body> (iter pos, ir_def_timeline& other)
  {
    get_timelines<range::body> ().splice (pos, other.get_timelines<range::body> ());
  }

  template <>
  void
  ir_def_timeline::
  splice<ir_instruction_range::phi> (iter, ir_def_timeline&, iter, iter) = delete;

  template <>
  void
  ir_def_timeline::
  splice<ir_instruction_range::body> (iter pos, ir_def_timeline& other, iter first, iter last)
  {
    get_timelines<range::body> ().splice (pos, other.get_timelines<range::body> (), first, last);
  }

  template <>
  auto
  ir_def_timeline::
  emplace_before<ir_instruction_range::phi> (citer, instr_iter) -> iter = delete;

  template <>
  auto
  ir_def_timeline::
  emplace_before<ir_instruction_range::body> (const citer pos, const instr_iter instr_pos)
    -> iter
  {
    return get_timelines<range::body> ().emplace (pos, instr_pos);
  }

  template <>
  ir_use_timeline&
  ir_def_timeline::
  emplace_back<ir_instruction_range::phi> (instr_iter) = delete;

  template <>
  ir_use_timeline&
  ir_def_timeline::
  emplace_back<ir_instruction_range::body> (const instr_iter instr_pos)
  {
    return get_timelines<range::body> ().emplace_back (instr_pos);
  }

  template <>
  auto
  ir_def_timeline::
  erase<ir_instruction_range::phi> (citer) -> iter = delete;

  template <>
  auto
  ir_def_timeline::
  erase<ir_instruction_range::body> (citer pos)
  -> iter
  {
    return get_timelines<range::body> ().erase (pos);
  }

  template <>
  auto
  ir_def_timeline::
  erase<ir_instruction_range::phi> (citer, citer) -> iter = delete;

  template <>
  auto
  ir_def_timeline::
  erase<ir_instruction_range::body> (const citer first, const citer last)
    -> iter
  {
    return get_timelines<range::body> ().erase (first, last);
  }

  void
  ir_def_timeline::transfer_successors (ir_def_timeline& src)
  {
    succ_tracker::replace_bindings (std::move (src));
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
    return std::find_if (incoming_begin (), incoming_end (),
                         [&](const ir_incoming_node& node)
                         {
                           return &node.get_incoming_block () == &block;
                         });
  }

  auto
  ir_def_timeline::
  add_successor (ir_incoming_node& remote)
    -> succ_iter
  {
    return bind (remote);
  }

  auto
  ir_def_timeline::
  remove_successor (ir_incoming_node& node)
    -> succ_iter
  {
    auto find_pred = [&](const ir_incoming_node& e) { return &e == &node; };
    return succ_tracker::erase (std::find_if (succs_begin (), succs_end (), find_pred));
  }

  [[nodiscard]]
  ir_instruction_iter
  ir_def_timeline::
  instr_begin (iter pos) noexcept
  {
    return pos->get_def_pos ();
  }

  [[nodiscard]]
  ir_instruction_citer
  ir_def_timeline::
  instr_begin (citer pos) noexcept
  {
    return pos->get_def_pos ();
  }

  [[nodiscard]]
  ir_instruction_citer
  ir_def_timeline::
  instr_cbegin (citer pos) noexcept
  {
    return instr_begin (pos);
  }

  [[nodiscard]]
  ir_instruction_iter
  ir_def_timeline::
  instr_end (iter pos) const noexcept
  {
    if (pos != timelines_cend<range::body> ())
      return std::next (pos)->get_def_pos ();
    return m_block->end<range::body> ();
  }

  [[nodiscard]]
  ir_instruction_citer
  ir_def_timeline::
  instr_end (citer pos) const noexcept
  {
    if (pos != timelines_cend<range::body> ())
      return std::next (pos)->get_def_pos ();
    return m_block->cend<range::body> ();
  }

  [[nodiscard]]
  ir_instruction_citer
  ir_def_timeline::
  instr_cend (citer pos) const noexcept
  {
    return instr_end (pos);
  }

  [[nodiscard]]
  ir_instruction_riter
  ir_def_timeline::
  instr_rbegin (iter pos) const noexcept
  {
    return instr_riter  { instr_end (pos) };
  }

  [[nodiscard]]
  ir_instruction_criter
  ir_def_timeline::
  instr_rbegin (citer pos) const noexcept
  {
    return instr_criter  { instr_cend (pos) };
  }

  [[nodiscard]]
  ir_instruction_criter
  ir_def_timeline::
  instr_crbegin (citer pos) const noexcept
  {
    return instr_rbegin (pos);
  }

  [[nodiscard]]
  ir_instruction_riter
  ir_def_timeline::
  instr_rend (iter pos) noexcept
  {
    return instr_riter  { instr_begin (pos) };
  }

  [[nodiscard]]
  ir_instruction_criter
  ir_def_timeline::
  instr_rend (citer pos) noexcept
  {
    return instr_criter  { instr_cbegin (pos) };
  }

  [[nodiscard]]
  ir_instruction_criter
  ir_def_timeline::
  instr_crend (citer pos) noexcept
  {
    return instr_rend (pos);
  }

  ir_use_timeline::iter
  ir_def_timeline::
  find_first_after (const iter ut_it, const instr_citer pos) const noexcept
  {
    // we reverse iterate because the instruction is more likely to
    // be near the back of the timeline.
    // note: multiple uses may be associated to the same instruction
    //       so we need to make sure we iterate to one past that
    //       instruction, rather than just find the first instance
    //       of that instruction.
    auto rcurr = ut_it->rbegin ();
    for (auto it = std::prev (instr_end (ut_it)); it != pos && rcurr != ut_it->rend (); --it)
    {
      while (&(*it) == &rcurr->get_instruction ())
        ++rcurr;
    }
    return rcurr.base ();
  }

  ir_use_timeline::citer
  ir_def_timeline::
  find_first_after (const citer ut_it, const instr_citer pos) const noexcept
  {
    auto rcurr = ut_it->rbegin ();
    for (auto it = std::prev (instr_end (ut_it));
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
  num_instrs (citer pos) const noexcept
  {
    return std::distance (instr_begin (pos), instr_end (pos));
  }

  bool
  ir_def_timeline::
  has_instrs (citer pos) const noexcept
  {
    return instr_begin (pos) == instr_end (pos);
  }

  bool
  ir_def_timeline::
  has_incoming (void) const noexcept
  {
    return ! incoming_empty ();
  }

  bool
  ir_def_timeline::
  has_timelines (void) const noexcept
  {
    return ! timelines_empty<range::all> ();
  }

  bool
  ir_def_timeline::
  has_incoming_timeline (void) const noexcept
  {
    return ! timelines_empty<range::phi> ();
  }

  bool
  ir_def_timeline::
  has_local_timelines (void) const noexcept
  {
    return ! timelines_empty<range::body> ();
  }

  ir_use_timeline&
  ir_def_timeline::
  get_incoming_timeline (void) noexcept
  {
    return timelines_front<range::phi> ();
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

  ir_use_timeline&
  ir_def_timeline::
  get_outgoing_timeline (void) noexcept
  {
    return timelines_back<range::all> ();
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

  ir_use_timeline&
  ir_def_timeline::
  create_incoming_timeline (void)
  {
    instr_iter phi_it = m_block->append_phi (*m_var);
    try
    {
      return get_timelines<range::phi> ().emplace_front (phi_it);
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
    if (timelines_empty<range::phi> ())
      throw ir_exception ("could not find phi in def timeline");
    get_timelines<range::phi> ().clear ();
    m_block->erase_phi (*m_var);
  }

  ir_block&
  ir_block::split_into (const citer pivot, ir_block& dest)
  {
    if (pivot == end<range::body> ())
      return dest;

    // move needed timelines into dest
    std::transform (m_timeline_map.begin (), m_timeline_map.end (),
                    std::back_inserter (dest.m_timeline_map),
                    [this, &pivot, &dest](std::pair<ir_variable * const, ir_def_timeline>& pair)
                    {
                      return std::pair { std::get<ir_variable * const> (pair),
                                         split (std::get<ir_def_timeline> (pair), pivot, dest) };
                    });

    // move the range into dest
    dest.splice<range::body> (dest.begin<range::body> (), *this, pivot, end<range::body> ());
    return dest;
  }

  ir_def_timeline
  ir_block::split (ir_def_timeline& dt, citer pivot, ir_block& dest)
  {
    ir_def_timeline ret (dest, *this, dt);
    ret.transfer_successors (dt);

    if (dt.local_empty () || pivot == end<range::body> ())
      return ret;

    if (pivot == begin<range::body> ())
    {
      // move everything
      ret.splice (ret.local_end (), dt);
      if (dt.has_single_incoming ())
        ret.swap_incoming (dt);
      return ret;
    }

    auto [tl_rit, tl_rfirst] = find_pair_latest_timeline_before (dt, pivot);

    // if we didnt stop exactly on a timeline boundary
    if (tl_rit != dt.local_rbegin () && tl_rfirst != rbegin<range::body> () &&
        &std::prev (tl_rit)->get_instruction () != &(*std::prev (tl_rfirst)))
    {
      if (tl_rit != dt.local_rend ())
      {
        ret.emplace_single_incoming (split_uses (*tl_rit, pivot, tl_rfirst.base ()), *this, dt);
      }
      else if (auto node = dt.get_single_incoming ())
      {
        ret.emplace_single_incoming (split_uses (node->get_use_timeline (), pivot, tl_rfirst.base ()),
                                     *this, dt);
      }
    }

    ret.splice (ret.local_end (), dt, tl_rit.base (), dt.local_end ());

    return ret;
  }

  ir_def_timeline::riter
  ir_block::
  find_latest_timeline (ir_def_timeline& dt) const
  {
    return dt.timelines_rbegin ();
  }

  ir_def_timeline::riter
  ir_block::
  find_latest_timeline_before (ir_def_timeline& dt, const citer pos) const
  {
    if (pos == cend<range::body> ())
      return dt.timelines_rbegin ();

    if (pos == cbegin<range::body> ())
      return dt.local_rend ();

    // reverse iterate over the instructions
    // reverse iterate over the def timeline
    // compare pos with the position of the def instruction in each use timeline
    auto dt_rcurr = dt.timelines_rbegin ();
    for (auto it = end<range::body> (); it != pos && dt_rcurr != dt.local_rend (); --it)
    {
      if (std::prev (it) == dt_rcurr->get_def_pos ())
        ++dt_rcurr;
    }
    return dt_rcurr;
  }

  optional_ref<ir_use_timeline>
  ir_block::get_latest_timeline (ir_variable& var)
  {
    if (dt.defs_empty ())
    {

    }
    return dt.defs_back ();

    if (dt.has_defs ())
      return dt.defs_back ();

    if (optional_ref<ir_def_timeline> dt = maybe_join_incoming (var))
      return dt->get_outgoing_timeline ();
    return nullopt;
  }

  ir_use_timeline&
  ir_block::get_latest_timeline (ir_variable& var)
  {
    if (optional_ref<ir_def_timeline> dt = maybe_join_incoming (var))
      return dt->get_outgoing_timeline ();
    return nullopt;
  }

  ir_use_timeline&
  ir_block::get_latest_timeline_before (ir_def_timeline& dt, citer pos)
  {
    if (pos == end<range::body> ())
      return get_latest_timeline (dt);

    if (optional_ref<ir_def_timeline> dt = maybe_join_incoming (var))
    {
      if (auto found = find_latest_timeline_before (*dt, pos) ; found != dt->local_rend ())
        return *found;
      return dt->get_entry_timeline ();
    }
    return nullopt;
  }

  ir_use_timeline&
  ir_block::get_latest_timeline_before (ir_variable& var, const citer pos)
  {
    if (pos == end<range::body> ())
      return get_latest_timeline (var);
    if (optional_ref<ir_def_timeline> dt = maybe_join_incoming (var))
    {
      if (auto found = find_latest_timeline_before (*dt, pos) ; found != dt->local_rend ())
        return *found;
      return dt->get_entry_timeline ();
    }
    return nullopt;
  }

  class ir_associated_incoming
  {
  public:
    ir_associated_incoming            (void)                              = delete;
    ir_associated_incoming            (const ir_associated_incoming&)     = default;
    ir_associated_incoming            (ir_associated_incoming&&) noexcept = default;
    ir_associated_incoming& operator= (const ir_associated_incoming&)     = default;
    ir_associated_incoming& operator= (ir_associated_incoming&&) noexcept = default;
    ~ir_associated_incoming           (void)                              = default;

    ir_associated_incoming (nonnull_ptr<ir_block> incoming_block,
                            std::vector<nonnull_ptr<ir_def_timeline>>&& timelines)
      : m_incoming_block (incoming_block),
        m_timelines      (timelines)
    { }

    [[nodiscard]] auto  begin   (void)       noexcept { return m_timelines.begin ();   }
    [[nodiscard]] auto  begin   (void) const noexcept { return m_timelines.begin ();   }
    [[nodiscard]] auto  cbegin  (void) const noexcept { return m_timelines.cbegin ();  }

    [[nodiscard]] auto  end     (void)       noexcept { return m_timelines.end ();     }
    [[nodiscard]] auto  end     (void) const noexcept { return m_timelines.end ();     }
    [[nodiscard]] auto  cend    (void) const noexcept { return m_timelines.cend ();    }

    [[nodiscard]] auto  rbegin  (void)       noexcept { return m_timelines.rbegin ();  }
    [[nodiscard]] auto  rbegin  (void) const noexcept { return m_timelines.rbegin ();  }
    [[nodiscard]] auto  crbegin (void) const noexcept { return m_timelines.crbegin (); }

    [[nodiscard]] auto  rend    (void)       noexcept { return m_timelines.rend ();    }
    [[nodiscard]] auto  rend    (void) const noexcept { return m_timelines.rend ();    }
    [[nodiscard]] auto  crend   (void) const noexcept { return m_timelines.crend ();   }

    [[nodiscard]] auto& front   (void)       noexcept { return m_timelines.front ();   }
    [[nodiscard]] auto& front   (void) const noexcept { return m_timelines.front ();   }

    [[nodiscard]] auto& back    (void)       noexcept { return m_timelines.back ();    }
    [[nodiscard]] auto& back    (void) const noexcept { return m_timelines.back ();    }

    [[nodiscard]] auto  size    (void) const noexcept { return m_timelines.size ();    }
    [[nodiscard]] auto  empty   (void) const noexcept { return m_timelines.empty ();   }

    [[nodiscard]] constexpr
    ir_block&
    get_incoming_block (void) noexcept
    {
      return *m_incoming_block;
    }

    [[nodiscard]] constexpr
    const ir_block&
    get_incoming_block (void) const noexcept
    {
      return *m_incoming_block;
    }

    void
    append_timeline (ir_def_timeline& dt)
    {
      m_timelines.emplace_back (dt);
    }

    [[nodiscard]]
    const ir_def&
    get_def (void) const noexcept
    {
      return front ()->get_outgoing_timeline ().get_def ();
    }

    [[nodiscard]]
    optional_cref<ir_def>
    maybe_get_def (void) const noexcept
    {
      if (! empty ())
        return get_def ();
      return nullopt;
    }

    [[nodiscard]]
    std::vector<nonnull_ptr<ir_def_timeline>>&&
    release_timelines (void) noexcept
    {
      return std::move (m_timelines);
    }

  private:
    nonnull_ptr<ir_block>                     m_incoming_block;
    std::vector<nonnull_ptr<ir_def_timeline>> m_timelines;
  };

  ir_use_timeline&
  ir_block::join_incoming (ir_def_timeline& dt)
  {
    // collect associated blocks and timelines
    std::vector<ir_associated_incoming> incoming_assoc;
    std::transform (preds_begin (), preds_end (), std::back_inserter (incoming_assoc),
                    [&dt](nonnull_ptr<ir_block> block) -> ir_associated_incoming
                    {
                      return { block, block->forward_outgoing_timelines (dt.get_variable ()) };
                    });
    // always create a phi here

  }

  std::vector<nonnull_ptr<ir_def_timeline>>
  ir_block::forward_incoming_timelines (ir_variable& var)
  {

    // collect associated blocks and timelines
    std::vector<ir_associated_incoming> incoming_assoc;
    std::transform (preds_begin (), preds_end (), std::back_inserter (incoming_assoc),
                    [&var](nonnull_ptr<ir_block> block) -> ir_associated_incoming
                    {
                      return { block, block->forward_outgoing_timelines (var) };
                    });

    // find the first non-empty association
    auto found = std::find_if_not (incoming_assoc.begin (), incoming_assoc.end (),
                                   &ir_associated_incoming::empty);

    if (found == incoming_assoc.end ())
      return { };

    if (std::all_of (std::next (found), incoming_assoc.end (),
                     [cmp = &found->get_def ()](const ir_associated_incoming& inc)
                     {
                       // note: all defs should already match within an associated block
                       return cmp == inc.maybe_get_def ();
                     }))
    {
      // if all of the timelines match in their associated defs
      // we can just forward the timelines without a new phi
      return std::accumulate (std::next (found), incoming_assoc.end (), found->release_timelines (),
                              [](auto& ret, ir_associated_incoming& inc)
                              {
                                std::move (inc.begin (), inc.end (), std::back_inserter (ret));
                                return ret;
                              });
    }

    // otherwise we need to create a phi

    ir_def_timeline& dt = get_def_timeline (var);
    if (dt.has_incoming ())
      throw ir_exception ("the block already has incoming defs");

    std::for_each (incoming_assoc.begin (), incoming_assoc.end (),
                   [&dt](ir_associated_incoming& inc)
                   {
                     dt.append_incoming (inc.get_incoming_block (), inc.begin (), inc.end ());
                   });

    // now fix up successors
    if (! dt.has_local_timelines ())
    {
      // Incoming timelines in successors will be all-or-nothing in regards to
      // all of the timelines collected here, so we only need to find one in a
      // successor. If found, we replace everything in the incoming_node with *this.
    }

    return { dt };
  }

  std::vector<nonnull_ptr<ir_def_timeline>>
  ir_block::
  forward_outgoing_timelines (ir_variable& var)
  {
    if (optional_ref<ir_def_timeline> dt = find_def_timeline (var))
    {
      if (dt->defs_empty ())
        throw ir_exception ("def timeline should not be empty");
      return { *dt };
    }

    switch (num_preds ())
    {
      case 0:  return { };
      case 1:  return preds_front ()->forward_outgoing_timelines (var);
      default: return collect_defs_incoming (var);
    }
  }

  std::vector<std::pair<nonnull_ptr<ir_block>, optional_ref<ir_def_timeline>>>
  ir_block::collect_outgoing (ir_variable& var)
  {
    if (optional_ref<ir_def_timeline> dt = find_def_timeline (var) ; dt->has_timelines ())
      return { { *this, dt } };
    return { { *this, nullopt } };
  }

  ir_def_timeline&
  ir_block::append_incoming (ir_variable& var, ir_def_timeline& dt,
                                   ir_block& incoming_block, ir_def_timeline& pred)
  {
    if (! pred.has_timelines ())
      throw ir_exception ("incoming timeline unexpectedly empty");

    if (auto node = dt.get_single_incoming ())
    {
      // don't mutate to phi if the incoming instruction is the same as the current one
      if (&node->get_instruction () != &pred.get_outgoing_instruction ())
        dt.mutate_to_phi (append_phi (var)).append (incoming_block, pred);
      else
        node->add_predecessor (incoming_block, pred);
    }
    else if (auto phi = dt.get_phi ())
      phi->append (incoming_block, pred);
    else
      dt.emplace_single_incoming (incoming_block, pred);

    dt.remove_undefined (incoming_block);
    return dt;
  }

  ir_def_timeline&
  ir_block::append_incoming (ir_variable& var, ir_def_timeline& dt,
                                   ir_block& incoming_block,
                                   const std::vector<nonnull_ptr<ir_def_timeline>>& preds)
  {
    if (preds.empty ())
    {
      dt.append_undefined (incoming_block);
      return dt;
    }

    ir_instruction& instr = preds.front ()->get_outgoing_instruction ();
    bool is_indet = preds.front ()->has_indeterminate_outgoing ();

    if (auto node = dt.get_single_incoming ())
    {
      // don't mutate to phi if the incoming instruction is the same as the current one
      if (&node->get_instruction () != &instr)
      {
        dt.mutate_to_phi (append_phi (var)).append (instr, is_indet, incoming_block,
                                                    preds.begin (), preds.end ());
      }
      else
        node->add_predecessor (incoming_block, preds.begin (), preds.end ());
    }
    else if (auto phi = dt.get_phi ())
      phi->append (instr, is_indet, incoming_block, preds.begin (), preds.end ());
    else
      dt.emplace_single_incoming (instr, is_indet, incoming_block, preds.begin (), preds.end ());

    dt.remove_undefined (incoming_block);
    return dt;
  }

  ir_def_timeline&
  ir_block::resolve_undefined_incoming (ir_variable& undef_var, ir_def_timeline& var_dt)
  {
    if (auto undef_dt = find_def_timeline (undef_var))
    {
      if (undef_dt->has_any_incoming ())
        return *undef_dt;
    }

    std::vector<std::pair<nonnull_ptr<ir_block>, nonnull_ptr<ir_def_timeline>>> pred_dts;
    std::transform (var_dt.undef_begin (), var_dt.undef_end (), std::back_inserter (pred_dts),
                   [&undef_var](nonnull_ptr<ir_block> blk)
                   {
                     return std::pair { blk,
                              nonnull_ptr { blk->set_undefined_state (undef_var, true) } };
                   });

    auto process_node =
      [&undef_var, &pred_dts](ir_incoming_node& node)
      {
        ir_def_timeline& origin = node.get_origin ();
        ir_block& origin_block = origin.get_block ();

        optional_ref<ir_def_timeline> undef_dt;
        if (node.is_indeterminate ())
          undef_dt = origin_block.resolve_undefined_outgoing (undef_var, origin);
        else
          undef_dt = origin_block.set_undefined_state (undef_var, false);

        std::transform (node.begin (), node.end (), std::back_inserter (pred_dts),
                        [undef_dt](ir_def_linkage& link)
                        {
                          return std::pair { nonnull_ptr { link.get_incoming_block () },
                                             nonnull_ptr { *undef_dt } };
                        });
      };

    if (auto opt_node = var_dt.get_single_incoming ())
      process_node (*opt_node);
    else if (auto phi = var_dt.get_phi ())
      std::for_each (phi->begin (), phi->end (), process_node);

    ir_def_timeline& undef_dt = get_def_timeline (undef_var);
    std::for_each (pred_dts.begin (), pred_dts.end (),
                   applied {
                             [&](nonnull_ptr<ir_block> block, nonnull_ptr<ir_def_timeline> dt)
                             {
                               append_incoming (undef_var, undef_dt, *block, *dt);
                             }
                           });
  }

  ir_def_timeline&
  ir_block::resolve_undefined_outgoing (ir_variable& undef_var, ir_def_timeline& var_dt)
  {
    if (auto undef_dt = find_def_timeline (undef_var))
    {
      if (undef_dt->has_defs ())
        return *undef_dt;
    }

    if (var_dt.has_defs ())
      return set_undefined_state (undef_var, false);
    return resolve_undefined_incoming (undef_var, var_dt);
  }

  ir_def_timeline&
  ir_block::set_undefined_state (ir_variable& undef_var, bool state)
  {
    auto found = std::find_if (get<range::undef> ().rbegin (), get<range::undef> ().rend (),
                               [&undef_var](ir_instruction& instr)
                               {
                                 return &instr.get_return ().get_variable () == &undef_var;
                               });

    if (found != get<range::undef> ().rend ())
    {
      ir_constant& arg = *get_if<ir_constant> (found->front ());
      arg.emplace<bool> (state);
      return *find_def_timeline (undef_var);
    }
    else
    {
      get<range::undef> ().push_back (ir_instruction::create<ir_opcode::assign> (
        undef_var, ir_constant (state)));

      ir_def_timeline& undef_dt = get_def_timeline (undef_var);
      if (undef_dt.has_defs ())
        throw ir_exception ("undef var ir_def_timeline unexpectedly has defs already");
      undef_dt.emplace_back (get<range::undef> ().back ());
      return undef_dt;
    }
  }

  void
  ir_block::propagate_def_timeline (ir_variable& var, ir_block& incoming_block,
                                          ir_def_timeline& remote)
  {
    optional_ref<ir_def_timeline> opt_dt = find_def_timeline (var);
    if (opt_dt)
    {
      if (auto node = opt_dt->get_single_incoming ())
      {
        node->replace_predecessor (incoming_block, remote);
        if (node->is_indeterminate () && !node->get_use_timeline ().empty ())
        {
          // we need to split the block
          ir_def_timeline& undef_dt = resolve_undefined_incoming (var.get_undef_var (), *opt_dt);
        }

      }
      else if (auto phi = opt_dt->get_phi ())
      {
        phi->replace (*opt_dt, incoming_block, remote);
      }
    }

    if (! opt_dt || (opt_dt->has_single_incoming () && opt_dt->local_empty ()))
    {
      std::for_each (succs_begin (), succs_end (),
                     [&var, &remote, this](ir_block& block)
                     {
                       block.propagate_def_timeline (var, *this, remote);
                     });
    }
  }

  ir_use_info
  ir_block::prepare_operand (citer pos, ir_variable& var)
  {
    ir_def_timeline& dt = get_def_timeline (var);
    auto found =
    auto found = get_latest_timeline_before (pos, var);
    return { *found, }
    if (auto found_pair = get_pair_latest_timeline_before (pos, var))
    {
      auto& [tl, tl_rfirst] = *found_pair;
      return { { tl, find_first_use_after (*tl, pos, tl_rfirst.base ()) } };
    }
  }

  ir_block::iter
  ir_block::remove_instruction (const citer pos)
  {
    if (will_return (*pos))
    {
      // do some stuff to relink the dependent uses
      ir_def_timeline& dt = get_def_timeline (pos->get_return ().get_variable ());
      if (! dt.has_timelines ())
      {
        // need to terminate
      }

      auto def_tl = std::find_if (dt.local_rbegin (),
                                  dt.local_rend (),
                                  [pos](const ir_use_timeline& tl)
                                  {
                                    return &tl.get_instruction () == &(*pos);
                                  });

      if (def_tl != dt.local_rend ())
      {
        std::next (def_tl)->splice_back (*def_tl);
        dt.erase (std::next (def_tl).base ());
      }
      else
        throw ir_exception ("ir_def_timeline not found in the block");
    }
    return erase<range::body> (pos);
  }

  ir_block::iter
  ir_block::remove_range (const citer first, const citer last) noexcept
  {
    std::vector<nonnull_ptr<const ir_variable>> unlinked_vars;
    std::for_each (criter { last }, criter { first },
                   [this, first, &unlinked_vars](const ir_instruction& instr)
                   {
                     if (! will_return (instr))
                       return;

                     const ir_def& d = instr.get_return ();
                     if (std::none_of (unlinked_vars.begin (), unlinked_vars.end (), &d
                       .get_variable ()))
                     {
                       ir_def_timeline& dt = *find_def_timeline (d.get_variable ());
                       if (! dt.has_timelines ())
                       {
                         // need to terminate
                       }

                       auto def_tl = std::find_if (dt.local_rbegin (),
                                                   dt.local_rend (),
                                                   [&instr](const ir_use_timeline& tl)
                                                   {
                                                     return &tl.get_instruction () == &instr;
                                                   });

                       if (ir_def_timeline::riter found = find_latest_timeline_before (dt, first) ;
                             found != dt.local_rend ())
                       {
                         found->splice_back (*def_tl);
                         dt.erase (found.base (), def_tl.base ());
                       }
                       else
                       {
                         ir_use_timeline& found_tl = *dt.get_entry_timeline ();
                         found_tl.splice_back (*def_tl);
                         dt.erase (dt.local_begin (), def_tl.base ());
                       }
                       unlinked_vars.emplace_back (d.get_variable ());
                     }
                   });
    return erase<range::body> (first, last);
  }

  // ir_def& create_def (const instr_citer pos, ir_variable& var)
  // {
  //
  // }

  auto
  ir_block::create_def_before (ir_variable& var, citer pos)
  {
    // if pos has any succeeding instructions in this block or
    // the block has any successors, then we need to repoint
    // any dominated uses to the created def.
    // if (has_succs (pos))

  }

  ir_use_timeline
  ir_block::split_uses (ir_use_timeline& src, const citer pivot, const citer last)
  {
    ir_use_timeline ret { src.get_instruction () };
    ret.transfer_back (src, find_first_use_after (src, pivot, last), src.end ());
    return ret;
  }

  ir_use_timeline::iter
  ir_block::find_first_use_after (ir_use_timeline& tl, const citer pos,
                                        const citer last)
  {
    if (last == begin<range::body> () || pos == begin<range::body> ())
      return tl.begin ();

    if (pos == last)
      return tl.end ();

    return std::find_if (tl.rbegin (), tl.rend (),
                         [rfirst = criter { last },
                          rlast  = rend<range::body> (),
                          cmp    = &(*pos)](const ir_use& u) mutable
                         {
                           const ir_instruction *tl_instr  = &u.get_instruction ();
                           for (; rfirst != rlast && &(*rfirst) != tl_instr; ++rfirst)
                           {
                             if (&(*rfirst) == cmp)
                               return true;
                           }
                           return false;
                         }).base ();
  }

  optional_ref<ir_def_timeline>
  ir_block::
  find_def_timeline (const ir_variable& var)
  {
    if (auto cit = m_timeline_map.find (&var); cit != m_timeline_map.end ())
      return optional_ref { std::get<ir_def_timeline> (*cit) };
    return nullopt;
  }

  bool
  ir_block::
  reassociate_timelines (const ir_link_set<ir_def_timeline>& old_dts, ir_def_timeline& new_dt,
                         std::vector<nonnull_ptr<ir_block>>& until)
  {
    if (auto stop_found = std::find (until.begin (), until.end (), this) ;
        stop_found != until.end ())
    {
      until.erase (stop_found);
      return true;
    }

    ir_variable& var = new_dt.get_variable ();
    if (optional_ref local_dt { find_def_timeline (var) })
    {
      assert (local_dt->has_timelines () && "def timeline should not be empty");
      return true;
    }
    return false;
  }

  void
  ir_block::reset (void) noexcept
  {
    erase (begin<range::body> (), end<range::body> ());

    // TODO remove when verified
    for (const auto& p : m_timeline_map)
    {

    }
  }

}

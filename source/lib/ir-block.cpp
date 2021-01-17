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

  ir_incoming_node::ir_incoming_node (ir_def_timeline& parent, ir_basic_block& incoming_block,
                                      ir_def_timeline& pred)
    : base     (tag::bind, pred),
      m_parent (parent),
      m_incoming_block (incoming_block)
  { }

  ir_incoming_node&
  ir_incoming_node::operator= (ir_incoming_node&& other) noexcept
  {
    m_incoming_block = other.m_incoming_block;
    base::operator= (std::move (other));
  }

  auto
  ir_incoming_node::add_predecessor (ir_def_timeline& d)
    -> iter
  {
    return bind (d);
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

  auto
  ir_def_timeline::
  emplace_before (citer pos, instr_iter instr_pos)
    -> iter
  {
    if (! local_empty () && is_a<ir_opcode::phi> (local_front ().get_def_instruction ()))
      throw ir_exception ("cannot place def before the phi def");
    return m_use_timelines.emplace (pos, instr_pos);
  }

  ir_use_timeline&
  ir_def_timeline::
  emplace_back (instr_iter instr_pos)
  {
    return m_use_timelines.emplace_back (instr_pos);
  }

  void
  ir_def_timeline::transfer_successors (ir_def_timeline& src)
  {
    succ_tracker::replace_bindings (std::move (src));
  }

  ir_incoming_node&
  ir_def_timeline::
  append_incoming (ir_basic_block& incoming_block, ir_def_timeline& pred)
  {
    // try to find a matching dt, if not found create an incoming node
    if (incoming_iter found = find_incoming (incoming_block) ; found != incoming_end ())
      found->add_predecessor (pred);
    else
    {
      // if we don't have any incoming yet we need to start up the use_timeline
      if (incoming_empty ())
        m_use_timelines.emplace_front (m_block->create_phi (*m_var));
      m_incoming.emplace_back (*this, incoming_block, pred);
    }
  }

  auto
  ir_def_timeline::
  find_incoming (const ir_basic_block& block) noexcept
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
  find_incoming (const ir_basic_block& block) const noexcept
    -> incoming_citer
  {
    return const_cast<ir_def_timeline *> (this)->find_incoming (block);
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
    if (pos != local_cend ())
      return std::next (pos)->get_def_pos ();
    return m_block->body_end ();
  }

  [[nodiscard]]
  ir_instruction_citer
  ir_def_timeline::
  instr_end (citer pos) const noexcept
  {
    if (pos != local_cend ())
      return std::next (pos)->get_def_pos ();
    return m_block->body_cend ();
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

  ir_basic_block&
  ir_basic_block::split_into (const citer pivot, ir_basic_block& dest)
  {
    if (pivot == body_end ())
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
    dest.get_body ().splice (dest.body_begin (), get_body (), pivot, body_end ());
    return dest;
  }

  ir_def_timeline
  ir_basic_block::split (ir_def_timeline& dt, citer pivot, ir_basic_block& dest)
  {
    ir_def_timeline ret (dest, *this, dt);
    ret.transfer_successors (dt);

    if (dt.local_empty () || pivot == body_end ())
      return ret;

    if (pivot == body_begin ())
    {
      // move everything
      ret.splice (ret.local_end (), dt);
      if (dt.has_single_incoming ())
        ret.swap_incoming (dt);
      return ret;
    }

    auto [tl_rit, tl_rfirst] = find_pair_latest_timeline_before (dt, pivot);

    // if we didnt stop exactly on a timeline boundary
    if (tl_rit != dt.local_rbegin () && tl_rfirst != body_rbegin () &&
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
  ir_basic_block::
  find_latest_timeline_before (ir_def_timeline& dt, const citer pos) const
  {
    if (pos == body_cbegin ())
      return dt.local_rend ();

    // reverse iterate over the instructions
    // reverse iterate over the def timeline
    // compare pos with the position of the def instruction in each use timeline
          auto   dt_rcurr = dt.local_rbegin ();
    const auto   dt_rend  = dt.has_incoming () ? std::prev (dt.local_rend ()) : dt.local_rend ();

    for (auto it = body_end (); it != pos && dt_rcurr != dt_rend; --it)
    {
      if (std::prev (it) == dt_rcurr->get_def_pos ())
        ++dt_rcurr;
    }
    return dt_rcurr;
  }

  optional_ref<ir_use_timeline>
  ir_basic_block::get_latest_timeline (ir_variable& var)
  {
    if (optional_ref<ir_def_timeline> dt = maybe_join_incoming (var))
      return dt->get_outgoing_timeline ();
    return nullopt;
  }

  optional_ref<ir_use_timeline>
  ir_basic_block::get_latest_timeline_before (const citer pos, ir_variable& var)
  {
    if (pos == get_body ().end ())
      return get_latest_timeline (var);
    if (optional_ref<ir_def_timeline> dt = maybe_join_incoming (var))
    {
      if (auto found = find_latest_timeline_before (*dt, pos) ; found != dt->local_rend ())
        return *found;
      return dt->get_entry_timeline ();
    }
    return nullopt;
  }

  std::optional<std::pair<nonnull_ptr<ir_use_timeline>, ir_basic_block::criter>>
  ir_basic_block::get_pair_latest_timeline_before (const citer pos, ir_variable& var)
  {
    if (pos == body_end ())
    {
      if (optional_ref<ir_use_timeline> tl = get_latest_timeline (var))
        return { { *tl, body_rbegin () } };
    }
    else if (optional_ref<ir_def_timeline> dt = maybe_join_incoming (var))
    {
      auto [tl_rit, tl_rfirst] = find_pair_latest_timeline_before (*dt, pos);
      if (tl_rit != dt->local_rend ())
        return { { *tl_rit, tl_rfirst } };
      else if (optional_ref<ir_use_timeline> inc = dt->get_entry_timeline ())
        return { { *inc, tl_rfirst } };
    }
    return std::nullopt;
  }

  optional_ref<ir_def_timeline>
  ir_basic_block::maybe_join_incoming (ir_variable& var)
  {
    optional_ref<ir_def_timeline> opt_dt = find_def_timeline (var);
    if (opt_dt && opt_dt->has_any_incoming ())
      return opt_dt;

    using dt_vector = std::vector<nonnull_ptr<ir_def_timeline>>;
    std::vector<std::pair<nonnull_ptr<ir_basic_block>, dt_vector>> pred_dts;
    std::transform (preds_begin (), preds_end (), std::back_inserter (pred_dts),
                    [&var](nonnull_ptr<ir_basic_block> block)
                    {
                      return std::make_pair (block, block->collect_defs_outgoing (var));
                    });

    if (opt_dt)
    {
      std::for_each (pred_dts.begin (), pred_dts.end (),
                     [&var, opt_dt, this](const auto& p)
                     {
                       append_incoming (var, *opt_dt, *p.first, p.second);
                     });
      return *opt_dt;
    }
    else if (! std::all_of (selecting<1> (pred_dts.begin ()), selecting<1> (pred_dts.end ()),
                            &dt_vector::empty))
    {
      ir_def_timeline& dt = get_def_timeline (var);
      std::for_each (pred_dts.begin (), pred_dts.end (),
                     [&var, &dt, this](const auto& p)
                     {
                       append_incoming (var, dt, *p.first, p.second);
                     });
      return dt;
    }
    return nullopt;
  }

  std::vector<nonnull_ptr<ir_def_timeline>>
  ir_basic_block::collect_defs_incoming (ir_variable& var)
  {
    using dt_vector = std::vector<nonnull_ptr<ir_def_timeline>>;
    std::vector<std::pair<nonnull_ptr<ir_basic_block>, dt_vector>> pred_dts;
    std::transform (preds_begin (), preds_end (), std::back_inserter (pred_dts),
                   [&var](nonnull_ptr<ir_basic_block> block)
                   {
                     return std::make_pair (block, block->collect_defs_outgoing (var));
                   });

    auto found = std::find_if_not (selecting<1> (pred_dts.begin ()), selecting<1> (pred_dts.end ()),
                                   &dt_vector::empty);

    if (found == pred_dts.end ())
      return { };

    if (std::all_of (std::next (found), selecting<1> (pred_dts.end ()),
                     [cmp = &found->front ()->get_outgoing_instruction ()](const dt_vector& dv)
                     {
                       return cmp == &dv.front ()->get_outgoing_instruction ();
                     }))
    {
      dt_vector& ret = *found;
      std::for_each (std::next (found), selecting<1> (pred_dts.end ()),
                     [&ret](dt_vector& dv)
                     {
                       std::move (dv.begin (), dv.end (), std::back_inserter (ret));
                     });
      return ret;
    }

    ir_def_timeline& dt = get_def_timeline (var);
    if (dt.has_any_incoming ())
      throw ir_exception ("block already has incoming defs");

    ir_def_timeline::phi_node& phi = dt.emplace_phi_node (create_phi (var));

    std::for_each (found.base (), pred_dts.end (),
                   applied
                   {
                     [&dt, &phi](nonnull_ptr<ir_basic_block> block, dt_vector& dv)
                     {
                       if (! dv.empty ())
                       {
                         phi.append (dv.front ()->get_outgoing_instruction (),
                                     dv.front ()->has_indeterminate_outgoing (),
                                     *block, dv.begin (), dv.end ());
                       }
                       else
                       {
                         dt.append_undefined(*block);
                       }
                     }
                   });

    // now fix up successors
    if (dt.local_empty ())
    {

    }

    return { dt };
  }

  std::vector<nonnull_ptr<ir_def_timeline>>
  ir_basic_block::collect_defs_outgoing (ir_variable& var)
  {
    if (optional_ref<ir_def_timeline> dt = find_def_timeline (var) ; dt->has_timelines ())
      return { *dt };

    switch (num_preds ())
    {
      case 0:  return { };
      case 1:  return preds_front ()->collect_defs_outgoing (var);
      default: return collect_defs_incoming (var);
    }
  }

  std::vector<std::pair<nonnull_ptr<ir_basic_block>, optional_ref<ir_def_timeline>>>
  ir_basic_block::collect_outgoing (ir_variable& var)
  {
    if (optional_ref<ir_def_timeline> dt = find_def_timeline (var) ; dt->has_timelines ())
      return { { *this, dt } };
    return { { *this, nullopt } };
  }

  ir_def_timeline&
  ir_basic_block::append_incoming (ir_variable& var, ir_def_timeline& dt,
                                   ir_basic_block& incoming_block, ir_def_timeline& pred)
  {
    if (! pred.has_timelines ())
      throw ir_exception ("incoming timeline unexpectedly empty");

    if (auto node = dt.get_single_incoming ())
    {
      // don't mutate to phi if the incoming instruction is the same as the current one
      if (&node->get_instruction () != &pred.get_outgoing_instruction ())
        dt.mutate_to_phi (create_phi (var)).append (incoming_block, pred);
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
  ir_basic_block::append_incoming (ir_variable& var, ir_def_timeline& dt,
                                   ir_basic_block& incoming_block,
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
        dt.mutate_to_phi (create_phi (var)).append (instr, is_indet, incoming_block,
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
  ir_basic_block::resolve_undefined_incoming (ir_variable& undef_var, ir_def_timeline& var_dt)
  {
    if (auto undef_dt = find_def_timeline (undef_var))
    {
      if (undef_dt->has_any_incoming ())
        return *undef_dt;
    }

    std::vector<std::pair<nonnull_ptr<ir_basic_block>, nonnull_ptr<ir_def_timeline>>> pred_dts;
    std::transform (var_dt.undef_begin (), var_dt.undef_end (), std::back_inserter (pred_dts),
                   [&undef_var](nonnull_ptr<ir_basic_block> blk)
                   {
                     return std::pair { blk,
                              nonnull_ptr { blk->set_undefined_state (undef_var, true) } };
                   });

    auto process_node =
      [&undef_var, &pred_dts](ir_incoming_node& node)
      {
        ir_def_timeline& origin = node.get_origin ();
        ir_basic_block& origin_block = origin.get_block ();

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
                             [&](nonnull_ptr<ir_basic_block> block, nonnull_ptr<ir_def_timeline> dt)
                             {
                               append_incoming (undef_var, undef_dt, *block, *dt);
                             }
                           });
  }

  ir_def_timeline&
  ir_basic_block::resolve_undefined_outgoing (ir_variable& undef_var, ir_def_timeline& var_dt)
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
  ir_basic_block::set_undefined_state (ir_variable& undef_var, bool state)
  {
    auto found = std::find_if (get<range::undef> ().rbegin (), get<range::undef> ().rend (),
                               [&undef_var](ir_instruction& instr)
                               {
                                 return &instr.get_return ().get_var () == &undef_var;
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
  ir_basic_block::propagate_def_timeline (ir_variable& var, ir_basic_block& incoming_block,
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
                     [&var, &remote, this](ir_basic_block& block)
                     {
                       block.propagate_def_timeline (var, *this, remote);
                     });
    }
  }

  std::optional<ir_operand_pre::use_pair>
  ir_basic_block::prepare_operand (citer pos, ir_variable& var)
  {
    if (auto found_pair = get_pair_latest_timeline_before (pos, var))
    {
      auto& [tl, tl_rfirst] = *found_pair;
      return { { tl, find_first_use_after (*tl, pos, tl_rfirst.base ()) } };
    }
    return std::nullopt;
  }

  ir_basic_block::iter
  ir_basic_block::erase (const citer pos)
  {
    if (has_return (*pos))
    {
      // do some stuff to relink the dependent uses
      ir_def_timeline& dt = get_def_timeline (pos->get_return ().get_var ());
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
    return get_body ().erase (pos);
  }

  ir_basic_block::iter
  ir_basic_block::erase (const citer first, const citer last) noexcept
  {
    std::vector<nonnull_ptr<const ir_variable>> unlinked_vars;
    std::for_each (criter { last }, criter { first },
                   [this, first, &unlinked_vars](const ir_instruction& instr)
                   {
                     if (! has_return (instr))
                       return;

                     const ir_def& d = instr.get_return ();
                     if (std::none_of (unlinked_vars.begin (), unlinked_vars.end (), &d.get_var ()))
                     {
                       ir_def_timeline& dt = *find_def_timeline (d.get_var ());
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
                       unlinked_vars.emplace_back (d.get_var ());
                     }
                   });
    return get_body ().erase (first, last);
  }

  // ir_def& create_def (const instr_citer pos, ir_variable& var)
  // {
  //
  // }

  auto
  ir_basic_block::create_def_before (ir_variable& var, citer pos)
  {
    // if pos has any succeeding instructions in this block or
    // the block has any successors, then we need to repoint
    // any dominated uses to the created def.
    // if (has_succs (pos))

  }

  ir_use_timeline
  ir_basic_block::split_uses (ir_use_timeline& src, const citer pivot, const citer last)
  {
    ir_use_timeline ret { src.get_instruction () };
    ret.transfer_back (src, find_first_use_after (src, pivot, last), src.end ());
    return ret;
  }

  ir_use_timeline::iter
  ir_basic_block::find_first_use_after (ir_use_timeline& tl, const citer pos,
                                        const citer last)
  {
    if (last == body_begin () || pos == body_begin ())
      return tl.begin ();

    if (pos == last)
      return tl.end ();

    return std::find_if (tl.rbegin (), tl.rend (),
                         [rfirst = criter { last },
                          rlast  = body_rend (),
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

  ir_component::link_iter
  ir_basic_block::preds_begin (void)
  {
    return get_parent ().preds_begin (*this);
  }

  ir_component::link_iter
  ir_basic_block::preds_end (void)
  {
    return get_parent ().preds_end (*this);
  }

  std::size_t
  ir_basic_block::num_preds (void)
  {
    return get_parent ().num_preds (*this);
  }

  ir_component::link_iter
  ir_basic_block::succs_begin (void)
  {
    return get_parent ().succs_begin (*this);
  }

  ir_component::link_iter
  ir_basic_block::succs_end (void)
  {
    return get_parent ().succs_end (*this);
  }

  std::size_t
  ir_basic_block::num_succs (void)
  {
    return get_parent ().num_succs (*this);
  }

  ir_function&
  ir_basic_block::get_function (void) noexcept
  {
    return get_parent ().get_function ();
  }

  void
  ir_basic_block::reset (void) noexcept
  {
    erase (body_begin (), body_end ());

    // TODO remove when verified
    for (const auto& p : m_timeline_map)
    {

    }
  }

}

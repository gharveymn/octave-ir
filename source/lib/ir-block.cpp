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
  //
  // use_timeline
  //
  
  ir_use_timeline
  ir_use_timeline::split (const instr_citer pivot, const instr_citer first,
                          const instr_citer last)
  {
    ir_use_timeline ret (*m_origin_instr);
    iter tl_pivot = find_relative_position (pivot, first, last);
    std::for_each (tl_pivot, end (), [&ret](ir_use& u) { u.rebind (ret); });
    return ret;
  }
  
  ir_use_timeline::iter
  ir_use_timeline::find_relative_position (const instr_citer pos, instr_citer first,
                                           const instr_citer last)
  {
    if (empty () || first == last || pos == first)
      return begin ();
    
    if (pos == last)
      return end ();
    
    return std::find_if (begin (), end (),
                         [rfirst = instr_criter { last  },
                          rlast  = instr_criter { first },
                          cmp    = &(*std::prev (pos))] (const ir_use& u) mutable
                         {
                           const ir_instruction *ut_instr  = &u.get_instruction ();
                           for (; rfirst != rlast && &(*rfirst) != ut_instr; ++rfirst)
                           {
                             if (&(*rfirst) == cmp)
                               return true;
                           }
                           return false;
                         });
  }

  //
  // ir_basic_block
  //
  
  ir_basic_block::def_timeline::phi_node::iter
  ir_basic_block::def_timeline::phi_node::repoint (nonnull_ptr<def_timeline> from, def_timeline& to)
  {
    auto found = std::find_if (begin (), end (),
                               [&from](const auto& tup)
                               {
                                 return from == std::get<nonnull_ptr<def_timeline>> (tup);
                               });
    if (found != end ())
      std::get<nonnull_ptr<def_timeline>> (*found).emplace (to);
    return found;
  }
  
  ir_basic_block::def_timeline::phi_node::iter
  ir_basic_block::def_timeline::phi_node::remove (const nonnull_ptr<def_timeline> dt)
  {
    auto found = std::find_if (begin (), end (),
                               [&dt](const auto& tup)
                               {
                                 return dt == std::get<nonnull_ptr<def_timeline>> (tup);
                               });
    if (found != end ())
    {
      m_num_indet -= static_cast<std::size_t> (std::get<bool> (*found));
      return m_incoming.erase (found);
    }
    return end ();
  }
  
  ir_basic_block::def_timeline::phi_node::iter
  ir_basic_block::def_timeline::phi_node::set_indeterminate (const nonnull_ptr<def_timeline> dt,
                                                             bool indet)
  {
    auto found = std::find_if (begin (), end (),
                               [&dt](auto& tup)
                               {
                                 return dt == std::get<nonnull_ptr<def_timeline>> (tup);
                               });
    if (found != end ())
    {
      m_num_indet = std::get<bool> (*found) ? (m_num_indet - static_cast<std::size_t> (! indet))
                                            : (m_num_indet + static_cast<std::size_t> (  indet));
      std::get<bool> (*found) = indet;
    }
    return found;
  }
  
  ir_basic_block::def_timeline::def_timeline (def_timeline&& other) noexcept
    : m_block     (other.m_block),
      m_variant_incoming (std::move (other.m_variant_incoming)),
      m_timelines (std::move (other.m_timelines)),
      m_succs     (std::move (other.m_succs))
  {
    std::for_each (m_succs.begin (), m_succs.end (),
                   [this](def_timeline& dt)
                   {
                     if (phi_node *phi = std::get_if<phi_node> (&dt.m_variant_incoming))
                     {
                       phi->repoint (dt, *this);
                     }
                   });
  }

  ir_basic_block::def_timeline&
  ir_basic_block::def_timeline::operator= (def_timeline&& other) noexcept
  {
    m_block     = other.m_block;
    m_variant_incoming      = std::move (other.m_variant_incoming);
    m_timelines = std::move (other.m_timelines);
    m_succs     = std::move (other.m_succs);
    std::for_each (m_succs.begin (), m_succs.end (),
                   [this](def_timeline& dt)
                   {
                     if (phi_node *phi = std::get_if<phi_node> (&dt.m_variant_incoming))
                     {
                       phi->repoint (dt, *this);
                     }
                   });
    return *this;
  }
  
  ir_basic_block::def_timeline
  ir_basic_block::def_timeline::split (const instr_citer pivot, ir_basic_block& dest)
  {
    def_timeline ret (dest);
    ret.m_succs.move_replace_bindings (m_succs);
    std::for_each (ret.m_succs.begin (), ret.m_succs.end (),
                   [&ret](def_timeline& dt)
                   {
                     if (phi_node *phi = std::get_if<phi_node> (&dt.m_variant_incoming))
                     {
                       phi->repoint (dt, ret);
                     }
                   });
    m_succs.bind (ret);
    
    if (empty () || m_block->body_empty () || pivot == m_block->body_end ())
      return ret;
    
    if (pivot == m_block->body_begin ())
    {
      // move everything
      ret.m_timelines.splice (ret.local_end (), m_timelines);
      if (has_single_incoming ())
      {
        ret.m_variant_incoming = std::move (m_variant_incoming);
        m_variant_incoming.emplace<std::monostate> ();
      }
      return ret;
    }
    
    instr_riter rfirst, curr_tl_rfirst = m_block->body_rbegin ();
    riter dt_rpivot = std::find_if (local_rbegin (), local_rend (),
                                [&rfirst,
                                 &curr_tl_rfirst,
                                  rlast = m_block->body_rend (),
                                  cmp = &(*std::prev (pivot))] (const ir_use_timeline& ut)
                                {
                                  curr_tl_rfirst = rfirst;
                                  const ir_instruction *ut_instr  = &ut.get_instruction ();
                                  for (; rfirst != rlast && &(*rfirst) != ut_instr; ++rfirst)
                                  {
                                    if (&(*rfirst) == cmp)
                                      return true;
                                  }
                                  return false;
                                });
  
    ret.m_timelines.splice (ret.local_end (), m_timelines, dt_rpivot.base (), local_end ());
    
    // if we didnt stop exactly on a timeline boundary
    if (rfirst != curr_tl_rfirst)
    {
      if (dt_rpivot != local_rend ())
      {
        ret.m_variant_incoming.emplace<ir_use_timeline> (
          dt_rpivot->split (pivot, m_block->body_begin (), curr_tl_rfirst.base ()));
      }
      else if (has_single_incoming ())
      {
        ret.m_variant_incoming.emplace<ir_use_timeline> (
          get_single_incoming ().split (pivot, m_block->body_begin (), curr_tl_rfirst.base ()));
      }
    }
    
    return ret;
  }
  
  optional_ref<ir_use_timeline>
  ir_basic_block::def_timeline::get_latest (void)
  {
    if (! m_timelines.empty ())
      return m_timelines.back ();
    return get_maybe_incoming ();
  }
  
  ir_basic_block::def_timeline::riter
  ir_basic_block::def_timeline::find_latest_before (const instr_citer pos)
  {
    if (empty () || m_block->body_empty () || pos == m_block->body_begin ())
      return local_rend ();
    
    return std::find_if (local_rbegin (), local_rend (),
                         [rfirst = m_block->body_rbegin (),
                          rlast  = m_block->body_rend (),
                          cmp    = &(*std::prev (pos))](const ir_use_timeline& ut) mutable
                         {
                           const ir_instruction *ut_instr  = &ut.get_instruction ();
                           for (; rfirst != rlast && &(*rfirst) != ut_instr; ++rfirst)
                           {
                             if (&(*rfirst) == cmp)
                               return true;
                           }
                           return false;
                         });
  }
  
  std::pair<ir_basic_block::def_timeline::riter, ir_basic_block::instr_riter>
  ir_basic_block::def_timeline::find_pair_latest_before (const instr_citer pos)
  {
    if (m_block->body_empty () || pos == m_block->body_begin ())
      return { local_rend (), m_block->body_rend () };
    
    if (empty ())
      return { local_rend (), m_block->body_rbegin () };
  
    instr_riter rfirst, curr_tl_rfirst = m_block->body_rbegin ();
    riter dt_rpivot = std::find_if (local_rbegin (), local_rend (),
                                    [&rfirst,
                                     &curr_tl_rfirst,
                                      rlast = m_block->body_rend (),
                                      cmp = &(*std::prev (pos))] (const ir_use_timeline& ut)
                                    {
                                      curr_tl_rfirst = rfirst;
                                      const ir_instruction *ut_instr  = &ut.get_instruction ();
                                      for (; rfirst != rlast && &(*rfirst) != ut_instr; ++rfirst)
                                      {
                                        if (&(*rfirst) == cmp)
                                          return true;
                                      }
                                      return false;
                                    });
    return { dt_rpivot, curr_tl_rfirst };
  }
  
  optional_ref<ir_use_timeline>
  ir_basic_block::def_timeline::get_latest_before (instr_citer pos)
  {
    if (riter found = find_latest_before (pos) ; found != local_rend ())
      return *found;
    return get_maybe_incoming ();
  }
  
  std::pair<optional_ref<ir_use_timeline>, ir_basic_block::instr_citer>
  ir_basic_block::def_timeline::get_pair_latest_before (instr_citer pos)
  {
    auto found_pair = find_pair_latest_before (pos);
    if (std::get<riter> (found_pair) != local_rend ())
    {
      return { *std::get<riter> (found_pair), std::get<instr_riter> (found_pair).base () };
    }
    
    return { get_maybe_incoming (), std::get<instr_riter> (found_pair).base () };
    
  }

  ir_basic_block::ir_basic_block (ir_structure& parent)
    : m_parent (parent),
      m_instr_partition ()
  { }

  ir_basic_block::~ir_basic_block (void) noexcept = default;

  ir_basic_block&
  ir_basic_block::split_into (instr_iter pivot, ir_basic_block& dest)
  {
    auto& body = get_body ();
    if (pivot == body.end ())
      return dest;

    // move needed timelines into dest
    std::transform (m_timeline_map.begin (), m_timeline_map.end (),
                    std::back_inserter (dest.m_timeline_map),
                    [&dest, &pivot](auto& pair)
                      -> std::pair<ir_variable * const, def_timeline>
                    {
                      return { std::get<ir_variable * const> (pair),
                               std::get<def_timeline> (pair).split (pivot, dest) };
                    });
  
    // move the range into dest
    dest.get_body ().splice (dest.begin (), body, pivot, body.end ());
    return dest;
  }
  
  std::insert_iterator<ir_use_timeline>
  ir_basic_block::prepare_operand (instr_citer pos, ir_variable& var)
  {
    auto found_pair = get_pair_latest_timeline_before (pos, var);
    if (auto& tl = std::get<optional_ref<ir_use_timeline>> (found_pair))
    {
      return std::insert_iterator<ir_use_timeline> (*tl,
        tl->find_relative_position (pos, body_begin (), std::get<instr_citer> (found_pair)));
    }
    return { }
  }

  ir_basic_block::instr_iter
  ir_basic_block::erase (const instr_citer pos)
  {
    (*pos)->unlink_propagate (pos);
    return m_instructions.erase (pos);
  }

  ir_basic_block::instr_iter
  ir_basic_block::erase (const instr_citer first,
                         const instr_citer last) noexcept
  {
    std::for_each (instr_criter { last }, instr_criter { first },
                   [&first] (instr_cref uptr)
                   {
                     uptr->unlink_propagate (first);
                   });
    return m_instructions.erase (first, last);
  }

  optional_ref<ir_use_timeline>
  ir_basic_block::get_latest_timeline (ir_variable& var)
  {
    // return join_incoming (var) >>= &def_timeline::get_latest;
    if (optional_ref<def_timeline> dt = join_incoming (var))
      return dt->get_latest ();
    return nullopt;
  }

  optional_ref<ir_use_timeline>
  ir_basic_block::get_latest_timeline_before (const instr_citer pos, ir_variable& var)
  {
    if (pos == get_body ().end ())
      return get_latest_timeline (var);
    
    if (optional_ref<def_timeline> dt = join_incoming (var))
      return dt->get_latest_before (pos);
    return nullopt;
  }
  
  std::pair<optional_ref<ir_use_timeline>, ir_basic_block::instr_citer>
  ir_basic_block::get_pair_latest_timeline_before (const instr_citer pos, ir_variable& var)
  {
    if (pos == get_body ().end ())
      return { get_latest_timeline (var), pos };
    
    if (optional_ref<def_timeline> dt = join_incoming (var))
      return dt->get_pair_latest_before (pos);
    return { nullopt, get_body ().begin () };
  }
  
  optional_ref<ir_basic_block::def_timeline>
  ir_basic_block::join_incoming (ir_variable& var)
  {
    if (optional_ref<def_timeline> dt = find_timeline (var))
      return dt;
    
    std::size_t npreds = num_preds ();
    
    if (npreds == 0)
      return nullopt;
    
    if (npreds == 1)
      return pred_front ()->join_incoming (var);
  
    def_timeline& dt = get_timeline (var);
    
    // only for debugging
    if (dt.has_incoming ())
      throw ir_exception ("block already has incoming");
    
    ir_instruction& phi_instr = get_phi_range ().emplace_back (
      ir_instruction::create<ir_opcode::phi> (var));
    
    def_timeline::phi_node& phi = dt.emplace_phi_node (phi_instr);
    
    // TODO: remove recursion
    std::for_each (pred_begin (), pred_end (),
                   [&var, &phi] (nonnull_ptr<ir_basic_block> block)
                   {
                     if (optional_ref<def_timeline> remote_dt = block->join_incoming (var))
                     {
                       if (remote_dt->local_empty ())
                       {
                         if (remote_dt->has_phi ())
                         {
                           def_timeline::phi_node& remote_phi = remote_dt->get_phi ();
                           phi.append (*remote_dt, remote_phi.get_instruction (),
                                       remote_phi.is_indeterminate ());
                         }
                         else if (remote_dt->has_single_incoming ())
                         {
                           ir_use_timeline& remote_tl = remote_dt->get_single_incoming ();
                           phi.append (*remote_dt, remote_tl.get_instruction (), false);
                         }
                         else
                           phi.append_undefined(*block);
                       }
                       else
                       {
                         ir_use_timeline& tl = remote_dt->local_back ();
                         phi.append (*remote_dt, tl.get_instruction (), false);
                       }
                     }
                     else
                       phi.append_undefined(*block);
                   });
    
    if (phi.empty ())
    {
      dt.reset_incoming ();
      get_phi_range ().pop_back ();
    }
    
    return dt;
  }
  
  // ir_def& create_def (const instr_citer pos, ir_variable& var)
  // {
  //
  // }

  auto
  ir_basic_block::create_def_before (ir_variable& var, instr_citer pos)
  {
    // if pos has any succeeding instructions in this block or
    // the block has any successors, then we need to repoint
    // any dominated uses to the created def.
    // if (has_succs (pos))

  }

  ir_component::link_iter
  ir_basic_block::pred_begin (void)
  {
    return m_parent.pred_begin (*this);
  }

  ir_component::link_iter
  ir_basic_block::pred_end (void)
  {
    return m_parent.pred_end (*this);
  }

  std::size_t
  ir_basic_block::num_preds (void)
  {
    return m_parent.num_preds (*this);
  }

  ir_component::link_iter
  ir_basic_block::succ_begin (void)
  {
    return m_parent.succ_begin (*this);
  }

  ir_component::link_iter
  ir_basic_block::succ_end (void)
  {
    return m_parent.succ_end (*this);
  }

  std::size_t
  ir_basic_block::num_succs (void)
  {
    return m_parent.num_succs (*this);
  }

  ir_function&
  ir_basic_block::get_function (void) noexcept
  {
    return m_parent.get_function ();
  }

  const ir_function&
  ir_basic_block::get_function (void) const noexcept
  {
    return m_parent.get_function ();
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

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
  ir_use_timeline::split (const instr_riter rpivot, const instr_riter rfirst,
                          const instr_riter rlast)
  {
    ir_use_timeline ret (*m_origin_instr);
    riter tl_rpivot = find_latest_before (rpivot, rfirst, rlast);
    std::for_each (tl_rpivot.base (), end (), [&ret](ir_use& u) { u.rebind (ret); });
    return ret;
  }
  
  ir_use_timeline::riter
  ir_use_timeline::find_latest_before (const instr_riter rpos, instr_riter rfirst,
                                       const instr_riter rlast)
  {
    if (empty () || rfirst == rlast || rpos == rlast)
      return rend ();
    
    return std::find_if (rbegin (), rend (),
                         [&rfirst, rlast, cmp = &(*rpos)] (const ir_use& u)
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
      m_head      (std::move (other.m_head)),
      m_timelines (std::move (other.m_timelines)),
      m_succs     (std::move (other.m_succs))
  {
    std::for_each (m_succs.begin (), m_succs.end (),
                   [this](def_timeline& dt)
                   {
                     if (phi_node *phi = std::get_if<phi_node> (&dt.m_head))
                     {
                       phi->repoint (dt, *this);
                     }
                   });
  }

  ir_basic_block::def_timeline&
  ir_basic_block::def_timeline::operator= (def_timeline&& other) noexcept
  {
    m_block     = other.m_block;
    m_head      = std::move (other.m_head);
    m_timelines = std::move (other.m_timelines);
    m_succs     = std::move (other.m_succs);
    std::for_each (m_succs.begin (), m_succs.end (),
                   [this](def_timeline& dt)
                   {
                     if (phi_node *phi = std::get_if<phi_node> (&dt.m_head))
                     {
                       phi->repoint (dt, *this);
                     }
                   });
    return *this;
  }
  
  ir_basic_block::def_timeline
  ir_basic_block::def_timeline::split (instr_riter rpivot, ir_basic_block& dest)
  {
    def_timeline ret (dest, m_succs);
    if (m_block->body_empty () || rpivot == m_block->body_rend ())
      return ret;
    
    instr_riter rfirst, curr_tl_rfirst = m_block->body_rbegin ();
    riter dt_rpivot = std::find_if (local_rbegin (), local_rend (),
                                    [&rfirst,
                                     &curr_tl_rfirst,
                                     rlast = m_block->body_rend (),
                                     cmp = &(*rpivot)] (const ir_use_timeline& ut)
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
    
    if (dt_rpivot != local_rend ())
    {
      ret.m_timelines.splice (ret.local_end (), m_timelines, dt_rpivot.base (), local_end ());
      ret.m_timelines.push_front (dt_rpivot->split (rpivot, curr_tl_rfirst, m_block->body_rend ()));
    }
    else
    {
      ret.m_timelines = std::move (m_timelines);
      if (has_single_incoming ())
        ret.m_head.emplace<ir_use_timeline> (dt_rpivot->split (rpivot, curr_tl_rfirst,
                                                               m_block->body_rend ()));
    }
    
    return ret;
  }
  
  optional_ref<ir_def>
  ir_basic_block::def_timeline::get_latest (void)
  {
    if (m_timelines.empty ())
    {
      return std::visit (overloaded
        {
          [](std::monostate) constexpr -> optional_ref<ir_def> { return nullopt;        },
          [](auto&& val)     constexpr -> optional_ref<ir_def> { return val.get_def (); }
        }, m_head);
    }
    return m_timelines.back ().get_def ();
  }
  
  ir_basic_block::def_timeline::riter
  ir_basic_block::def_timeline::find_latest_before (instr_citer pos)
  {
    if (empty () || m_block->body_empty () || pos == m_block->body_begin ())
      return local_rend ();
    
    return std::find_if (local_rbegin (), local_rend (),
                         [rfirst = m_block->body_rbegin (),
                          rlast  = m_block->body_rend (),
                          cmp    = &(*pos)](const ir_use_timeline& ut) mutable
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
  
  optional_ref<ir_use_timeline>
  ir_basic_block::def_timeline::get_latest_timeline_before (instr_citer pos)
  {
    if (riter found = find_latest_before (pos) ; found != local_rend ())
      return *found;
    return std::visit (overloaded
      {
        [](std::monostate)      constexpr -> optional_ref<ir_use_timeline> { return nullopt;             },
        [](phi_node& phi)       constexpr -> optional_ref<ir_use_timeline> { return phi.get_timeline (); },
        [](ir_use_timeline& tl) constexpr -> optional_ref<ir_use_timeline> { return tl;                  }
      }, m_head);
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
                    [&dest, rpivot = instr_riter (pivot)](auto& pair)
                      -> std::pair<ir_variable * const, def_timeline>
                    {
                      return { std::get<ir_variable * const> (pair),
                               std::get<def_timeline> (pair).split (rpivot, dest) };
                    });
  
    // move the range into dest
    dest.get_body ().splice (dest.begin (), body, pivot, body.end ());
    return dest;
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

  optional_ref<ir_def>
  ir_basic_block::get_latest_def (ir_variable& var)
  {
    if (auto vt = find_timeline (var))
    {
      if (auto local_latest = vt->get_latest ())
        return local_latest;
    }
    return join_incoming_defs (var);
  }

  optional_ref<ir_def>
  ir_basic_block::get_latest_def_before (const instr_citer pos, ir_variable& var)
  {
    if (pos == get_body ().end ())
      return get_latest_def (var);
    
    if (auto dt = find_timeline (var))
    {
      auto found = dt->find_latest_before (instr_criter (pos), get_body ().rbegin (),
                                           get_body ().rend ());
      if (found != dt->crend ())
        return found->get_def ();
    }
    return join_incoming_defs (var);
  }
  
  optional_ref<ir_def>
  ir_basic_block::join_incoming_defs (ir_variable& var)
  {
    std::size_t npreds = num_preds ();
    
    if (npreds == 0)
      return nullopt;
    
    if (npreds == 1)
      return pred_front ()->get_latest_def (var);
  
    auto& dt = get_timeline (var);
    
    // only for debugging
    if (dt.has_phi ())
      throw ir_exception ("block already contains a phi instruction for the variable");
    
    std::vector<std::pair<nonnull_ptr<ir_basic_block>, nonnull_ptr<ir_def>>> incoming_defs;
    std::vector<nonnull_ptr<ir_basic_block>> indet_incoming;
    
    // TODO: remove recursion
    std::for_each (pred_begin (), pred_end (),
                   [&incoming_defs, &indet_incoming, &var] (nonnull_ptr<ir_basic_block> block)
                   {
                     optional_ref<def_timeline> dt = block->get_latest_def (var);
                     if (dt.has_value ())
                     {
                       ir_use_timeline& tl = dt->back ();
                       ir_instruction& def_instr = tl.get_instruction ();
                       if (is_a<ir_opcode::phi> (def_instr) && dt->has_indet ())
                         indet_incoming.emplace_back (block);
                       incoming_defs.emplace_back (block, *def);
                     }
                     else
                     {
                       indet_incoming.emplace_back (block);
                     }
                   });
    
    // don't create a phi instruction if there are no incoming defs
    if (incoming_defs.empty ())
      return nullopt;
  
    // create phi instruction
    ir_phi& phi_instr = create_phi (var);
    
    std::vector<std::pair<nonnull_ptr<ir_basic_block>, ir_use>> phi_args;
    std::for_each (incoming_defs.begin (), incoming_defs.end (),
                   [&phi_instr, &dt = get_timeline(var)] (auto&& p)
                   {
                     ir_use_timeline& ut = dt.emplace_front (*std::get<nonnull_ptr<ir_def>> (p));
                     phi_instr.append_incoming (*std::get<nonnull_ptr<ir_basic_block>> (p), ut);
                   });
    phi_instr.set_indets (std::move (indet_incoming));
    return phi_instr.get_def ();
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
    if (has_succs (pos))

  }
  
  

  void
  ir_basic_block::set_cached_def (ir_def& d)
  {
    get_timeline (d.get_var ()).replace_cache (d);
  }

  void
  ir_basic_block::track_phi_def (ir_def& d)
  {
    get_timeline (d.get_var ()).track_phi (d);
  }

  void
  ir_basic_block::def_emplace_before (const instr_citer pos, ir_def& d)
  {
    def_timeline& vt = get_timeline (d.get_var ());
    if (pos == body_end ())
      return vt.emplace_back (d);
    vt.emplace_before (find_latest_def_before (pos, vt).base (), d);
  }

  void
  ir_basic_block::def_emplace_front (ir_def& d)
  {
    get_timeline (d.get_var ()).emplace_front (d);
  }

  void
  ir_basic_block::def_emplace_back (ir_def& d)
  {
    get_timeline (d.get_var ()).emplace_back (d);
  }

  void
  ir_basic_block::def_emplace_before (instr_citer pos, ir_def_instruction& d)
  {
    return def_emplace_before (pos, d.get_def ());
  }

  void
  ir_basic_block::def_emplace_front (ir_def_instruction& d)
  {
    return def_emplace_front (d.get_def ());
  }

  void
  ir_basic_block::def_emplace_back (ir_def_instruction& d)
  {
    return def_emplace_back (d.get_def ());
  }

  gch::optional_ref<ir_def>
  ir_basic_block::join_preceding_defs (ir_variable& var)
  {
    std::size_t npreds = num_preds ();
    if (npreds == 0)
      return nullopt;
    if (npreds == 1)
      return var.join (*pred_front ());

    ir_variable::block_def_vect pairs;
    std::vector<nonnull_ptr<ir_basic_block>> undef_blocks;
    std::for_each (pred_begin (), pred_end (),
                   [&var, &pairs, &undef_blocks] (nonnull_ptr<ir_basic_block> pred)
                    {
                      if (optional_ref<ir_def> opt_def = var.join (*pred))
                        pairs.emplace_back (pred, *opt_def);
                      else
                        undef_blocks.emplace_back (pred);
                    });

    // TODO if we have null returns here we need to create extra diversion
    //  blocks for those code paths. Otherwise the code will crash.

    auto cmp_def = std::get<nonnull_ptr<ir_def>> (pairs.front ());
    if (std::any_of (pairs.begin (), pairs.end (),
          [cmp_def] (auto&& p) { return std::get<nonnull_ptr<ir_def>> (p) != cmp_def; }))
    {
      ir_type common_ty = ir_variable::normalize_types (pairs);
      return create_phi (var, common_ty, pairs).get_def ();
    }
    return *cmp_def;
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

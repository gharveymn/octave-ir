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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "ir-block.h"
#include "ir-type-std.h"
#include "ir-variable.h"
#include "ir-instruction.h"
#include "ir-structure.h"

#include <algorithm>

namespace octave
{
  //
  // ir_basic_block
  //
  
  
  ir_basic_block::ir_basic_block (ir_structure& parent)
    : m_parent (parent),
      m_body_begin (m_instrs.end ())
  { }
  
  ir_basic_block::~ir_basic_block (void) noexcept = default;
  
  template <typename ...Args>
  ir_phi *
  ir_basic_block::create_phi (Args&&... args)
  {
    std::unique_ptr<ir_phi> u
      = create_instruction<ir_phi> (std::forward<Args> (args)...);
    ir_phi *ret = u.get ();
    m_instrs.push_front (std::move (u));
    try
      {
        def_emplace_front (ret->get_return ());
      }
    catch (const std::exception& e)
      {
        m_instrs.erase (m_instrs.begin ());
        throw e;
      }
    ++m_num_phi;
    return ret;
  }
  
  ir_basic_block::instr_iter
  ir_basic_block::erase_phi (instr_citer pos)
  {
    if (! is_phi_iter (pos))
      throw ir_exception ("cannot erase non-phi instruction");
    --m_num_phi;
    return m_instrs.erase (pos);
  }
  
  bool ir_basic_block::is_phi_iter (instr_citer cit)
  {
    return isa<ir_phi> (cit->operator-> ());
  }
  
  ir_basic_block::instr_iter
  ir_basic_block::erase (instr_citer pos)
  {
    (*pos)->unlink_propagate (pos);
    return m_instrs.erase (pos);
  }
  
  ir_basic_block::instr_iter
  ir_basic_block::erase (instr_citer first, instr_citer last) noexcept
  {
    for (instr_criter crit (last); crit != instr_criter (first); ++crit)
      (*crit)->unlink_propagate (first);
    return m_instrs.erase (first, last);
  }

  ir_def *
  ir_basic_block::fetch_cached_def (ir_variable& var) const
  {
    return fetch_proximate_def (var, end ());
  }
  
  ir_def*
  ir_basic_block::fetch_proximate_def (ir_variable& var, instr_citer pos) const
  {
    vtm_citer vtm_cit = m_vt_map.find (&var);
    if (vtm_cit == m_vt_map.end ())
      return nullptr;
    const def_timeline& dt = vtm_cit->second;
    if (dt.size () == 0 || pos == end ())
      return dt.fetch_cache ();

    instr_criter rpos (pos);
    def_timeline::criter dt_crit = dt.rbegin ();
    for (instr_criter in_crit = rbegin (); in_crit != rend (); ++in_crit)
      {
        if (dt_crit->first == in_crit.base ())
          {
            if (++dt_crit == dt.rend ())
              return nullptr;
          }
        if (rpos == in_crit)
          return dt_crit->second;
      }
    return nullptr;
  }
  
  void
  ir_basic_block::set_cached_def (ir_def& d)
  {
    m_vt_map[&d.get_var ()].set_cache (d);
  }
  
  void
  ir_basic_block::def_emplace (instr_citer pos, ir_def& d)
  {
    def_timeline& dt = m_vt_map[&d.get_var ()];
    if (pos == end ())
      return dt.emplace_back (pos, d);

    instr_criter rpos (pos);
    def_timeline::criter dt_crit = dt.rbegin ();
    for (instr_criter instr_crit = m_instrs.rbegin ();
         instr_crit != m_instrs.rend (); ++instr_crit)
      {
        if (dt_crit->first == instr_crit.base ())
          {
            if (++dt_crit == dt.rend ())
              return dt.emplace_front (pos, d);
          }
        if (rpos == instr_crit)
          return dt.emplace (dt_crit.base (), pos, d);
      }
    dt.emplace_front (pos, d);
  }
  
  void
  ir_basic_block::def_emplace_front (ir_def& d)
  {
    m_vt_map[&d.get_var ()].emplace_front (m_instrs.begin (), d);
  }
  
  void
  ir_basic_block::def_emplace_back (ir_def& d)
  {
    m_vt_map[&d.get_var ()].emplace_back (--m_instrs.end (), d);
  }

  ir_def&
  ir_basic_block::join_defs (ir_variable& var)
  {
    return join_defs (var, end ());
  }
  
  ir_def&
  ir_basic_block::join_defs (ir_variable& var, instr_citer pos)
  {
    ir_def *ret = fetch_proximate_def (var, pos);
    if (ret == nullptr)
      ret = join_pred_defs (var);
    
    // if ret is still nullptr then we need to insert a fetch instruction
    if (ret == nullptr)
      ret = &emplace_before<ir_fetch> (pos, var).get_return ();
    else
      {
        // if the ir_def was created by a phi node, there may be
      }
    
    return *ret;
  }

  ir_def *
  ir_basic_block::join_pred_defs (ir_variable& var)
  {
    
    std::size_t npreds = num_preds ();
    
    if (npreds == 0)
      return nullptr;
    
    if (npreds == 1)
      {
        ir_basic_block *pred = *pred_begin ();
        return &pred->join_defs (var);
      }

    block_def_vect pairs;
    pairs.reserve (npreds);
    
    std::for_each (pred_begin (), pred_end (),
                   [&pairs, &var](ir_basic_block *pred){
                     if (pred == nullptr)
                       throw ir_exception ("block was unexpectedly nullptr.");
                     pairs.emplace_back (*pred, &pred->join_defs (var));
                   });
    
    // TODO if we have null returns here we need to create extra diversion
    //  blocks for those code paths. Otherwise the code will crash.
    
    ir_def * cmp_def = pairs.front ().second;
    for (const block_def_pair& p : pairs)
      {
        // check if all the defs found were the same
        if (p.second != cmp_def)
          {
            // if not then we need to create a phi node
            ir_type ty = ir_variable::normalize_types (pairs);
            ir_phi *phi_node = create_phi (var, ty, pairs);
            return &phi_node->get_return ();
          }
      }
    
    return cmp_def;
  }
  
  ir_component::link_iter
  ir_basic_block::pred_begin (void)
  {
    return m_parent.pred_begin (this);
  }
  
  ir_component::link_iter
  ir_basic_block::pred_end (void)
  {
    return m_parent.pred_end (this);
  }
  
  std::size_t
  ir_basic_block::num_preds (void)
  {
    return std::distance (pred_begin (), pred_end ());
  }
  
  bool
  ir_basic_block::has_preds (void)
  {
    return pred_begin () != pred_end ();
  }
  
  bool
  ir_basic_block::has_multiple_preds (void)
  {
    return num_preds () > 1;
  }
  
  ir_component::link_iter
  ir_basic_block::succ_begin (void)
  {
    return m_parent.succ_begin (this);
  }
  
  ir_component::link_iter
  ir_basic_block::succ_end (void)
  {
    return m_parent.succ_end (this);
  }
  
  std::size_t
  ir_basic_block::num_succs (void)
  {
    return std::distance (succ_begin (), succ_end ());
  }
  
  bool
  ir_basic_block::has_succs (void)
  {
    return succ_begin () != succ_end ();
  }
  
  bool
  ir_basic_block::has_multiple_succs (void)
  {
    return num_succs () > 1;
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
    m_instrs.clear ();
  }
  
  constexpr ir_type::impl ir_type::instance<ir_basic_block *>::m_impl;
  
}

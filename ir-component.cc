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

#include "ir-component.h"
#include "ir-variable.h"
#include "ir-instruction.h"
#include "ir-structure.h"

#include <algorithm>

namespace octave
{
  
  //
  // ir_component
  //
  
  ir_component::~ir_component () noexcept
  {
  
  }
  
  //
  // ir_basic_block
  //
  
  ir_basic_block::ir_basic_block (ir_module& mod, ir_structure& parent)
    : ir_component (mod, &parent)
  {
    m_leaf.push_back (this);
  }
  
  static_assert(std::is_same<typename std::remove_cv<const ir_basic_block&>::type, const ir_basic_block&>::value, "is const");
  
  ir_basic_block::~ir_basic_block (void) = default;
  
  ir_basic_block::iter
  ir_basic_block::begin (void) { return m_instrs.begin (); }
  
  ir_basic_block::citer
  ir_basic_block::begin (void) const { return m_instrs.begin (); }
  
  ir_basic_block::iter
  ir_basic_block::end (void) { return m_instrs.end (); }
  
  ir_basic_block::citer
  ir_basic_block::end (void) const { return m_instrs.end (); }
  
  ir_basic_block::ref
  ir_basic_block::front (void) { return m_instrs.front (); }
  
  ir_basic_block::cref
  ir_basic_block::front (void) const { return m_instrs.front (); }
  
  ir_basic_block::ref
  ir_basic_block::back (void) { return m_instrs.back (); }
  
  ir_basic_block::cref
  ir_basic_block::back (void) const { return m_instrs.back (); }
  
  template <typename ...Args>
  ir_phi *
  ir_basic_block::create_phi (Args&&... args)
  {
    std::unique_ptr<ir_phi> uptr = make_unique<ir_phi> (*this,
                                             std::forward<Args> (args)...);
    ir_phi *ret = uptr.get ();
    m_instrs.push_front (std::move (uptr));
    return ret;
  }

  template <typename T, typename ...Args>
  enable_if_t<std::is_base_of<ir_instruction, T>::value, T> *
  ir_basic_block::emplace_back (Args&&... args)
  {
    std::unique_ptr<T> uptr = make_unique<T> (*this,
                                              std::forward<Args> (args)...);
    T *ret = uptr.get ();
    m_instrs.push_back (std::move (uptr));
    if (def *d = ret->get_return_def ())
      emplace_back_def (*d);
    return ret;
  }

  template <typename T, typename ...Args>
  enable_if_t<std::is_base_of<ir_instruction, T>::value, T> *
  ir_basic_block::emplace_before (citer pos, Args&&... args)
  {
    std::unique_ptr<T> u = make_unique<T> (*this,
                                           std::forward<Args> (args)...);
    T *ret = u.get ();
    citer it = m_instrs.insert (pos, std::move (u));
    if (ret->has_return ())
      emplace_def (it, ret->get_return ());
    return ret;
  }
  
  ir_basic_block::iter
  ir_basic_block::erase (citer pos)
  {
    return m_instrs.erase (pos);
  }
  
  ir_basic_block::iter
  ir_basic_block::erase (citer first, citer last)
  {
    return m_instrs.erase (first, last);
  }

  ir_variable::def *
  ir_basic_block::fetch_cached_def (ir_variable& var) const
  {
    vtm_citer cit = m_vt_map.find (&var);
    if (cit == m_vt_map.end ())
      return nullptr;
    const def_timeline& dt = cit->second;
    return dt.fetch_cache ();
  }

  ir_variable::def *
  ir_basic_block::fetch_proximate_def (ir_variable& var, citer pos) const
  {
    criter rpos (pos);
    vtm_citer vtm_cit = m_vt_map.find (&var);
    if (vtm_cit == m_vt_map.end ())
      return nullptr;
    const def_timeline& dt = vtm_cit->second;
    if (dt.size () == 0)
      return dt.fetch_cache ();

    def_timeline::criter dt_crit = dt.rbegin ();
    for (criter instr_crit = m_instrs.rbegin ();
         instr_crit != m_instrs.rend (); ++instr_crit)
      {
        if (dt_crit->first == instr_crit.base())
          {
            if (++dt_crit == dt.rend ())
              return nullptr;
          }
        if (rpos == instr_crit)
          return dt_crit->second;
      }
    return nullptr;
  }

  void
  ir_basic_block::set_cached_def (ir_variable::def& d)
  {
    m_vt_map[&d.get_var ()].set_cache (d);
  }

  void
  ir_basic_block::emplace_def (citer pos, def& d)
  {
    def_timeline& dt = m_vt_map[&d.get_var ()];

    if (pos == end ())
      {
        dt.emplace_back (pos, d);
        return;
      }

    criter rpos (pos);
    def_timeline::criter dt_crit = dt.rbegin ();
    for (criter instr_crit = m_instrs.rbegin ();
         instr_crit != m_instrs.rend (); ++instr_crit)
      {
        if (dt_crit->first == instr_crit.base ())
          {
            if (++dt_crit == dt.rend ())
              {
                dt.emplace_front (pos, d);
                return;
              }
          }
        if (rpos == instr_crit)
          {
            dt.emplace (dt_crit.base (), pos, d);
            return;
          }
      }
    dt.emplace_front (pos, d);
  }

  void
  ir_basic_block::emplace_back_def (def& d)
  {
    m_vt_map[&d.get_var ()].emplace_back (--m_instrs.end (), d);
  }

  ir_variable::def *
  ir_basic_block::get_latest_def (ir_variable& var)
  {
    if (def *d = fetch_cached_def (var))
      return d;
    return join_pred_defs (var);
  }

  ir_variable::def *
  ir_basic_block::get_latest_def_before (ir_variable& var, citer pos)
  {
    def *ret = nullptr;
    if (pos == end ())
      ret = get_latest_def (var);
    else
      {
        ret = fetch_proximate_def (var, pos);
        if (ret == nullptr)
          ret = join_pred_defs (var);
      }

    // if ret is still nullptr then we need to insert a fetch instruction
    if (ret == nullptr)
      {
        ir_fetch *finstr = emplace_before<ir_fetch> (pos, var);

      }

    return nullptr;
  }

  ir_variable::def *
  ir_basic_block::join_pred_defs (ir_variable& var)
  {

    std::size_t npreds = num_preds ();
    
    if (npreds == 0)
      return nullptr;

    if (npreds == 1)
      {
        ir_basic_block *pred = *pred_begin ();
        return pred->get_latest_def (var);
      }

    ir_variable::block_def_vec pairs;
    pairs.reserve (npreds);

    std::for_each (pred_begin (), pred_end (),
      [&pairs, &var](ir_basic_block *pred){
          if (pred == nullptr)
            throw ir_exception ("block was unexpectedly nullptr.");
          pairs.emplace_back (*pred, pred->get_latest_def (var));
      });

    // TODO if we have null returns here we need to create extra diversion
    //  blocks for those code paths. Otherwise the code will crash.

    def * cmp_def = pairs.front ().second;
    for (const ir_variable::block_def_pair& p : pairs)
      {
        if (p.second != cmp_def)
          {
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
    return get_parent ()->pred_begin (this);
  }

  ir_component::link_iter
  ir_basic_block::pred_end (void)
  {
    return get_parent ()->pred_end (this);
  }

  std::size_t
  ir_basic_block::num_preds (void)
  {
    return std::distance (pred_begin (), pred_end ());
  }

  bool
  ir_basic_block::has_preds (void)
  {
    return num_preds () != 0;
  }

  bool
  ir_basic_block::has_multiple_preds (void)
  {
    return num_preds () > 1;
  }
  
  

}

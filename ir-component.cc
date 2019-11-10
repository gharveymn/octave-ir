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

  ir_component::~ir_component (void) noexcept = default;

  //
  // ir_basic_block
  //

  ir_basic_block::ir_basic_block (ir_module& mod, ir_structure& parent)
    : ir_component (mod),
      m_parent (parent),
      m_body_begin (m_instrs.end ())
  { }

  ir_basic_block::~ir_basic_block (void) noexcept = default;

  template <typename ...Args>
  ir_phi *
  ir_basic_block::create_phi (Args&&... args)
  {
    std::unique_ptr<ir_phi> uptr = octave::make_unique<ir_phi> (*this,
                                             std::forward<Args> (args)...);
    ir_phi *ret = uptr.get ();
    m_instrs.push_front (std::move (uptr));
    try
      {
        def_emplace_front (ret->get_return ());
      }
    catch (const std::exception& e)
      {
        erase (m_instrs.begin ());
        throw e;
      }
    ++m_num_phi;
    return ret;
  }

  template <typename T, typename ...Args>
  enable_if_t<ir_basic_block::is_nonphi_instruction<T>::value
              && ir_basic_block::has_return<T>::value, T>&
  ir_basic_block::emplace_front (Args&&... args)
  {
    std::unique_ptr<T> u = octave::make_unique<T> (*this,
                                           std::forward<Args> (args)...);
    T *ret = u.get ();
    m_body_begin = m_instrs.insert (m_body_begin, std::move (u));
    try
      {
        def_emplace (m_body_begin, ret->get_return ());
      }
    catch (const std::exception& e)
      {
        m_body_begin = erase (m_body_begin);
        throw e;
      }
    return *ret;
  }

  template <typename T, typename ...Args>
  enable_if_t<ir_basic_block::is_nonphi_instruction<T>::value
              && ir_basic_block::has_return<T>::value, T>&
  ir_basic_block::emplace_back (Args&&... args)
  {
    std::unique_ptr<T> u = octave::make_unique<T> (*this,
                                           std::forward<Args> (args)...);
    T *ret = u.get ();
    iter it = m_instrs.insert (body_end (), std::move (u));
    try
      {
        def_emplace (it, ret->get_return());
      }
    catch (const std::exception& e)
      {
        erase (it);
        throw e;
      }
    if (m_body_begin == body_end ())
      --m_body_begin;
    return *ret;
  }

  template ir_convert& ir_basic_block::emplace_back<ir_convert, ir_variable&, ir_type, ir_variable::def&> (ir_variable&, ir_type&&, ir_variable::def&);

  template <typename T, typename ...Args>
  enable_if_t<ir_basic_block::is_nonphi_instruction<T>::value
              && ir_basic_block::has_return<T>::value, T>&
  ir_basic_block::emplace_before (citer pos, Args&&... args)
  {
    if (pos == end () || isa<ir_phi> (pos->get ()))
      throw ir_exception ("instruction must be placed within the body");
    std::unique_ptr<T> u = octave::make_unique<T> (*this,
                                           std::forward<Args> (args)...);
    T *ret = u.get ();
    iter it = m_instrs.insert (pos, std::move (u));
    try
      {
        def_emplace (it, ret->get_return());
      }
    catch (const std::exception& e)
      {
        erase (it);
        throw e;
      }
    if (m_body_begin == pos)
      m_body_begin = it;
    return *ret;
  }

  template <typename T, typename ...Args>
  enable_if_t<ir_basic_block::is_nonphi_instruction<T>::value
              && ! ir_basic_block::has_return<T>::value, T>&
  ir_basic_block::emplace_front (Args&&... args)
  {
    std::unique_ptr<T> u = octave::make_unique<T> (*this,
                                                std::forward<Args> (args)...);
    T *ret = u.get ();
    m_body_begin = m_instrs.insert (m_body_begin, std::move (u));
    return *ret;
  }

  template <typename T, typename ...Args>
  enable_if_t<ir_basic_block::is_nonphi_instruction<T>::value
              && ! ir_basic_block::has_return<T>::value, T>&
  ir_basic_block::emplace_back (Args&&... args)
  {
    std::unique_ptr<T> u = octave::make_unique<T> (*this,
                                                std::forward<Args> (args)...);
    T *ret = u.get ();
    m_instrs.push_back (std::move (u));
    if (m_body_begin == body_end ())
      --m_body_begin;
    return *ret;
  }

  template <typename T, typename ...Args>
  enable_if_t<ir_basic_block::is_nonphi_instruction<T>::value
              && ! ir_basic_block::has_return<T>::value, T>&
  ir_basic_block::emplace_before (citer pos, Args&&... args)
  {
    if (pos == end () || isa<ir_phi> (pos->get ()))
      throw ir_exception ("instruction must be placed within the body");
    std::unique_ptr<T> u = octave::make_unique<T> (*this,
                                                std::forward<Args> (args)...);
    T *ret = u.get ();
    iter it = m_instrs.insert (pos, std::move (u));
    if (m_body_begin == pos)
      m_body_begin = it;
    return *ret;
  }

  ir_basic_block::iter
  ir_basic_block::erase (citer pos) noexcept
  {
    return m_instrs.erase (pos);
  }

  ir_basic_block::iter
  ir_basic_block::erase (citer first, citer last) noexcept
  {
    return m_instrs.erase (first, last);
  }

  ir_variable::def *
  ir_basic_block::fetch_cached_def (ir_variable& var) const
  {
    return fetch_proximate_def (var, end ());
  }

  ir_variable::def *
  ir_basic_block::fetch_proximate_def (ir_variable& var, citer pos) const
  {
    vtm_citer vtm_cit = m_vt_map.find (&var);
    if (vtm_cit == m_vt_map.end ())
      return nullptr;
    const def_timeline& dt = vtm_cit->second;
    if (dt.size () == 0 || pos == end ())
      return dt.fetch_cache ();

    criter rpos (pos);
    def_timeline::criter dt_crit = dt.rbegin ();
    for (criter instr_crit = m_instrs.rbegin ();
         instr_crit != m_instrs.rend (); ++instr_crit)
      {
        if (dt_crit->first == instr_crit.base ())
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
  ir_basic_block::def_emplace (citer pos, def& d)
  {
    def_timeline& dt = m_vt_map[&d.get_var ()];

    if (pos == end ())
      return dt.emplace_back (pos, d);

    criter rpos (pos);
    def_timeline::criter dt_crit = dt.rbegin ();
    for (criter instr_crit = m_instrs.rbegin ();
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
  ir_basic_block::def_emplace_front (def& d)
  {
    m_vt_map[&d.get_var ()].emplace_front (m_instrs.begin (), d);
  }

  void
  ir_basic_block::def_emplace_back (def& d)
  {
    m_vt_map[&d.get_var ()].emplace_back (--m_instrs.end (), d);
  }

  ir_variable::def *
  ir_basic_block::join_defs (ir_variable& var)
  {
    return join_defs (var, end ());
  }

  ir_variable::def *
  ir_basic_block::join_defs (ir_variable& var, citer pos)
  {
    def *ret = fetch_proximate_def (var, pos);
    if (ret == nullptr)
      ret = join_pred_defs (var);

    // if ret is still nullptr then we need to insert a fetch instruction
    if (ret == nullptr)
      ret = &emplace_before<ir_fetch> (pos, var).get_return ();
    else
      {
        // if the def was created by a phi node, there may be
      }

    return ret;
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
        return pred->join_defs (var);
      }

    ir_variable::block_def_vec pairs;
    pairs.reserve (npreds);

    std::for_each (pred_begin (), pred_end (),
      [&pairs, &var](ir_basic_block *pred){
          if (pred == nullptr)
            throw ir_exception ("block was unexpectedly nullptr.");
          pairs.emplace_back (*pred, pred->join_defs (var));
      });

    // TODO if we have null returns here we need to create extra diversion
    //  blocks for those code paths. Otherwise the code will crash.

    def * cmp_def = pairs.front ().second;
    for (const ir_variable::block_def_pair& p : pairs)
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

  constexpr ir_type::impl ir_type::instance<ir_basic_block>::m_impl;
  template struct ir_type::instance<ir_basic_block *>;

}

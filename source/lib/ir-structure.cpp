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

#include <ir-structure.hpp>
#include <ir-component.hpp>
#include <ir-block.hpp>

#include <algorithm>

namespace gch
{

  //
  // ir_structure
  //

  ir_structure::~ir_structure (void) noexcept = default;

  void
  ir_structure::leaf_push_back (ir_basic_block& blk)
  {
    m_leaf_cache.emplace_back (blk);
  }

  template <typename It>
  void
  ir_structure::leaf_push_back (It first, It last)
  {
    std::copy (first, last, std::back_inserter (m_leaf_cache));
  }

  ir_structure::link_iter
  ir_structure::leaf_begin (void)
  {
    if (leaf_cache_empty ())
      generate_leaf_cache ();
    return m_leaf_cache.begin ();
  }

  ir_structure::link_iter
  ir_structure::leaf_end (void)
  {
    if (leaf_cache_empty ())
      generate_leaf_cache ();
    return m_leaf_cache.end ();
  }

  //
  // ir_fork_component
  //

  ir_fork_component::~ir_fork_component (void) noexcept = default;

  ir_component::link_iter
  ir_fork_component::pred_begin (ir_component& c)
  {
    return is_condition (&c) ? m_parent.pred_begin (*this) : link_iter (m_condition);
  }

  ir_component::link_iter
  ir_fork_component::pred_end (ir_component& c)
  {
    return is_condition (&c) ? m_parent.pred_end (*this) : ++link_iter (m_condition);
  }

  ir_component::link_iter
  ir_fork_component::succ_begin (ir_component& c)
  {
    return is_condition (&c) ? sub_entry_begin () : m_parent.succ_begin (*this);
  }

  ir_component::link_iter
  ir_fork_component::succ_end (ir_component& c)
  {
    return is_condition (&c) ? sub_entry_end () : m_parent.succ_end (*this);
  }

  bool
  ir_fork_component::is_leaf_component (ir_component *c) noexcept
  {
    // assumes that c is in the component
    return c != &m_condition;
  }

  void
  ir_fork_component::generate_leaf_cache (void)
  {
    for (comp_ref c_uptr : m_subcomponents)
      leaf_push_back (c_uptr->leaf_begin (), c_uptr->leaf_end ());
  }

  ir_component::link_iter
  ir_fork_component::sub_entry_begin (void)
  {
    if (m_sub_entry_cache.empty ())
      generate_sub_entry_cache ();
    return m_sub_entry_cache.begin ();
  }

  ir_component::link_iter
  ir_fork_component::sub_entry_end (void)
  {
    if (m_sub_entry_cache.empty ())
      generate_sub_entry_cache ();
    return m_sub_entry_cache.end ();
  }

  void
  ir_fork_component::generate_sub_entry_cache (void)
  {
    for (comp_ref c_uptr : m_subcomponents)
      leaf_push_back (c_uptr->get_entry_block ());
  }

  //
  // ir_loop_component
  //

  ir_loop_component::~ir_loop_component (void) noexcept = default;

  ir_component::link_iter
  ir_loop_component::pred_begin (ir_component& c)
  {
    if      (is_entry (&c))     return m_parent.pred_begin (*this);
    else if (is_condition (&c)) return m_cond_preds.begin ();
    else if (is_body (&c))      return m_condition;
    else                       throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::pred_end (ir_component& c)
  {
    if      (is_entry (&c))     return m_parent.pred_end (*this);
    else if (is_condition (&c)) return m_cond_preds.end ();
    else if (is_body (&c))      return ++link_iter (m_condition);
    else                       throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::succ_begin (ir_component& c)
  {
    if      (is_entry (&c))     return m_condition;
    else if (is_condition (&c)) return cond_succ_begin ();
    else if (is_body (&c))      return m_condition;
    else                       throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::succ_end (ir_component& c)
  {
    if      (is_entry (&c))     return ++link_iter (m_condition);
    else if (is_condition (&c)) return cond_succ_end ();
    else if (is_body (&c))      return ++link_iter (m_condition);
    else                       throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::cond_succ_begin (void)
  {
    m_succ_cache.front () = m_body.get_entry_block ();
    return m_succ_cache.begin ();
  }

  ir_component::link_iter
  ir_loop_component::cond_succ_end (void)
  {
    return m_succ_cache.end ();
  }

}

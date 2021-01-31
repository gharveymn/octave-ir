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

#include "ir-structure.hpp"
#include "ir-component.hpp"
#include "ir-block.hpp"

#include <algorithm>

namespace gch
{

  //
  // ir_structure
  //

  ir_structure::
  ~ir_structure (void) = default;

  ir_use_timeline&
  ir_structure::
  join_incoming (ir_block& block, ir_variable& var)
  {
    return join_incoming (block, block.get_def_timeline (var));
  }

  auto
  ir_structure::get_leaves (void)
    -> const link_vector&
  {
    if (leaf_cache_empty ())
      generate_leaf_cache ();
    return get_leaf_cache ();
  }

  //
  // ir_sequence
  //

  const ir_component_handle&
  ir_sequence::
  get_handle (const ir_component& c) const
  {
    return *find (c);
  }

  void
  ir_sequence::
  generate_leaf_cache (void)
  {
    leaves_append_range (back ()->leaves_begin (), back ()->leaves_end ());
  }

  void
  ir_sequence::
  invalidate_leaf_cache (void) noexcept
  {
    if (get_parent ().is_leaf_component (*this))
      get_parent ().invalidate_leaf_cache ();
  }

  //
  // ir_fork_component
  //

  ir_fork_component::
  ~ir_fork_component (void) noexcept = default;

  void
  ir_fork_component::
  generate_leaf_cache (void)
  {
    std::for_each (m_subcomponents.begin (), m_subcomponents.end (),
                   [this](ref comp)
                   {
                     leaves_append_range (comp->leaves_begin (), comp->leaves_end ());
                   });
  }

  void
  ir_fork_component::
  invalidate_leaf_cache (void) noexcept
  {
    if (! leaf_cache_empty ())
    {
      clear_leaf_cache ();
      if (get_parent ().is_leaf_component (*this))
        get_parent ().invalidate_leaf_cache ();
    }
  }

  ir_component::link_iter
  ir_fork_component::
  sub_entry_begin (void)
  {
    if (m_sub_entry_cache.empty ())
      generate_sub_entry_cache ();
    return m_sub_entry_cache.begin ();
  }

  ir_component::link_iter
  ir_fork_component::
  sub_entry_end (void)
  {
    if (m_sub_entry_cache.empty ())
      generate_sub_entry_cache ();
    return m_sub_entry_cache.end ();
  }

  void
  ir_fork_component::
  generate_sub_entry_cache (void)
  {
    std::transform (m_subcomponents.begin (), m_subcomponents.end (),
                    std::back_inserter (m_sub_entry_cache),
                    [](ref comp) -> nonnull_ptr<ir_block>
                    {
                      return comp->get_entry_block ();
                    });
  }

  //
  // ir_loop_component
  //

  ir_loop_component::
  ir_loop_component (ir_structure& parent)
    : ir_structure (parent),
      m_entry     (create_component<ir_block> ()),
      m_body      (create_component<ir_sequence> ()),
      m_condition (create_component<ir_condition_block> ()),
      m_exit      (create_component<ir_block> ()),
      m_cond_preds { m_entry, get_update_block () },
      m_succ_cache { m_body.get_entry_block (), m_exit }
  { }

  ir_loop_component::
  ~ir_loop_component (void) noexcept = default;

  ir_component::link_iter
  ir_loop_component::
  preds_begin (ir_component& c)
  {
    if      (is_entry (c))     return m_parent.preds_begin (*this);
    else if (is_condition (c)) return m_cond_preds.begin ();
    else if (is_body (c))      return m_condition;
    else                        throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::
  preds_end (ir_component& c)
  {
    if      (is_entry (c))     return m_parent.preds_end (*this);
    else if (is_condition (c)) return m_cond_preds.end ();
    else if (is_body (c))      return ++link_iter (m_condition);
    else                        throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::
  succs_begin (ir_component& c)
  {
    if      (is_entry (c))     return m_condition;
    else if (is_condition (c)) return cond_succ_begin ();
    else if (is_body (c))      return m_condition;
    else                        throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::
  succs_end (ir_component& c)
  {
    if      (is_entry (c))     return ++link_iter (m_condition);
    else if (is_condition (c)) return cond_succ_end ();
    else if (is_body (c))      return ++link_iter (m_condition);
    else                        throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::
  cond_succ_begin (void)
  {
    m_succ_cache.front () = m_body.get_entry_block ();
    return m_succ_cache.begin ();
  }

  ir_component::link_iter
  ir_loop_component::
  cond_succ_end (void)
  {
    return m_succ_cache.end ();
  }

}

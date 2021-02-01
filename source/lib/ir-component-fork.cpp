/** ir-fork-component.cpp.cc
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-component-fork.hpp"

#include <algorithm>

namespace gch
{

  ir_component_fork::
  ~ir_component_fork (void) noexcept = default;

  ir_component::link_iter
  ir_component_fork::
  sub_entry_begin (void)
  {
    if (m_sub_entry_cache.empty ())
      generate_sub_entry_cache ();
    return m_sub_entry_cache.begin ();
  }

  ir_component::link_iter
  ir_component_fork::
  sub_entry_end (void)
  {
    if (m_sub_entry_cache.empty ())
      generate_sub_entry_cache ();
    return m_sub_entry_cache.end ();
  }

  void
  ir_component_fork::
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
  // virtual from ir_component
  //

  void
  ir_component_fork::
  reset (void) noexcept
  {
    m_subcomponents.clear ();
  }

  //
  // virtual from ir_structure
  //

  const ir_component_handle&
  ir_component_fork::
  get_entry_component (void)
  {
    return m_condition;
  }

  auto
  ir_component_fork::
  get_preds (const ir_component_handle& comp)
    -> link_vector
  {
    if (is_entry_component (comp))
      return get_parent ().get_preds (*this);
    return copy_leaves (get_condition_component ());
  }

  void
  ir_component_fork::
  generate_leaf_cache (void)
  {
    std::for_each (m_subcomponents.begin (), m_subcomponents.end (),
                   &ir_component_fork::leaves_append);
  }

  bool
  ir_component_fork::
  is_leaf_component (const ir_component_handle& comp) noexcept
  {
    return comp != m_condition;
  }

}

/** ir-fork-component.cpp.cc
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-component-fork.hpp"
#include "ir-block.hpp"

#include <algorithm>

namespace gch
{

  ir_component_fork::
  ir_component_fork (ir_structure& parent)
    : ir_substructure { parent },
      m_condition     { create_component<ir_condition_block> () },
      m_find_cache    { make_handle (get_condition ()) }
  { }

  ir_component_fork::
  ~ir_component_fork (void) noexcept = default;

  ir_component_ptr
  ir_component_fork::
  find_case (ir_component& c) const
  {
    if (m_find_cache.contains (c))
      return m_find_cache.get ();

    ptr found = std::find_if (as_mutable (*this).cases_begin (), as_mutable (*this).cases_end (),
                              [&](const ir_component& cmp) { return &cmp == &c; });

    if (found != cases_end ())
      m_find_cache.emplace (make_handle (found));

    return found;
  }

  ir_component_ptr
  ir_component_fork::
  find (ir_component& c) const
  {
    if (is_condition (c))
      return as_mutable (*this).get_condition ();
    return find_case (c);
  }

  //
  // virtual from ir_component
  //

  bool
  ir_component_fork::
  reassociate_timelines (const ir_link_set<ir_def_timeline>& old_dts, ir_def_timeline& new_dt,
                         std::vector<nonnull_ptr<ir_block>>& until)
  {
    return get_condition ()->reassociate_timelines (old_dts, new_dt, until)
       ||  std::all_of (cases_begin (), cases_end (),
                        [&](ir_component& c)
                        {
                          return c.reassociate_timelines (old_dts, new_dt, until);
                        });
  }

  void
  ir_component_fork::
  reset (void) noexcept
  {
    m_cases.clear ();
  }

  //
  // virtual from ir_structure
  //

  ir_component_ptr
  ir_component_fork::
  get_entry_ptr (void)
  {
    return get_condition ();
  }

  ir_link_set<ir_block>
  ir_component_fork::
  get_predecessors (ir_component_cptr comp)
  {
    if (is_entry (comp))
      return get_parent ().get_predecessors (*this);
    return copy_leaves (get_condition ());
  }

  bool
  ir_component_fork::
  is_leaf (ir_component_cptr comp) noexcept
  {
    return comp != get_condition ();
  }

  void
  ir_component_fork::
  generate_leaf_cache (void)
  {
    std::for_each (cases_begin (), cases_end (), &ir_component_fork::leaves_append);
  }

  void
  ir_component_fork::
  recursive_flatten (void)
  {
    maybe_get_as<ir_structure> (get_condition ()) >>= &ir_structure::recursive_flatten;

    std::for_each (cases_begin (), cases_end (),
      [](ir_component& c) { maybe_cast<ir_structure> (c) >>= &ir_structure::recursive_flatten; });
  }

}

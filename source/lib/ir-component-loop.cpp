/** ir-loop-component.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-block.hpp"

namespace gch
{

  ir_component_loop::
  ir_component_loop (ir_structure& parent)
    : ir_substructure { parent },
      m_start         { create_component<ir_block> ()                                            },
      m_condition     { create_component<ir_condition_block> ()                                  },
      m_body          { create_component<ir_component_sequence> (ir_subcomponent_type<ir_block>) },
      m_update        { create_component<ir_block> ()                                            }
  { }

  ir_component_loop::
  ~ir_component_loop (void) noexcept = default;

  //
  // virtual from ir_component
  //

  bool
  ir_component_loop::
  reassociate_timelines (const std::vector<nonnull_ptr<ir_def_timeline>>& old_dts,
                         ir_def_timeline& new_dt, std::vector<nonnull_ptr<ir_block>>& until)
  {
    return get_start ()    ->reassociate_timelines (old_dts, new_dt, until)
       ||  get_condition ()->reassociate_timelines (old_dts, new_dt, until)
       ||  get_body ()     ->reassociate_timelines (old_dts, new_dt, until)
       ||  get_update ()   ->reassociate_timelines (old_dts, new_dt, until);
  }

  //
  // virtual from ir_structure
  //

  ir_component_ptr
  ir_component_loop::
  get_ptr (ir_component& c) const
  {
    if (is_start (c))
      return as_mutable (*this).get_start ();
    if (is_condition (c))
      return as_mutable (*this).get_condition ();
    if (is_body (c))
      return as_mutable (*this).get_body ();
    if (is_update (c))
      return as_mutable (*this).get_update ();

    throw ir_exception ("could not find the specified component in the loop");
  }

  ir_component_ptr
  ir_component_loop::
  get_entry_ptr (void)
  {
    return get_start ();
  }

  ir_link_set
  ir_component_loop::
  get_predecessors (ir_component_cptr comp)
  {
    if (is_start (comp))
      return get_parent ().get_predecessors (*this);

    if (is_condition (comp))
      return copy_leaves (get_start (), get_update ());

    if (is_body (comp))
      return copy_leaves (get_condition ());

    if (is_update (comp))
      return copy_leaves (get_body ());

    throw ir_exception ("specified component was not in the loop component");
  }

  ir_link_set
  ir_component_loop::
  get_successors (ir_component_cptr comp)
  {
    if (is_start (comp))
      return { nonnull_ptr { get_entry_block (get_condition ()) } };

    if (is_condition (comp))
    {
      ir_link_set ret { get_parent ().get_successors (*this) };
      ret.emplace (get_entry_block (get_body ()));
      return ret;
    }

    if (is_body (comp))
      return { nonnull_ptr { get_entry_block (get_update ()) } };

    if (is_update (comp))
      return { nonnull_ptr { get_entry_block (get_condition ()) } };

    throw ir_exception ("specified component was not in the loop component");
  }

  bool
  ir_component_loop::
  is_leaf (ir_component_cptr comp) noexcept
  {
    return is_condition (comp);
  }

  void
  ir_component_loop::
  generate_leaf_cache (void)
  {
    assert (leaf_cache_empty ());
    leaves_append (get_condition ());
  }

  void
  ir_component_loop::
  recursive_flatten (void)
  {
    maybe_get_as<ir_structure> (get_start ())     >>= &ir_structure::recursive_flatten;
    maybe_get_as<ir_structure> (get_condition ()) >>= &ir_structure::recursive_flatten;
    maybe_get_as<ir_structure> (get_body ())      >>= &ir_structure::recursive_flatten;
    maybe_get_as<ir_structure> (get_update ())    >>= &ir_structure::recursive_flatten;
  }

}

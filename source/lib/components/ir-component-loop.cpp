/** ir-loop-component.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-block.hpp"

namespace gch
{

  ir_component_loop::
  ir_component_loop (ir_structure& parent)
    : ir_substructure (parent),
      m_start  { allocate_subcomponent<ir_block> () },
      m_condition (*this),
      m_body   { allocate_subcomponent<ir_component_sequence> (ir_subcomponent_type<ir_block>) },
      m_update { allocate_subcomponent<ir_block> () }
  { }

  ir_component_loop::
  ~ir_component_loop (void) noexcept = default;

  bool
  ir_component_loop::
  is_start (const ir_subcomponent& sub) const noexcept
  {
    return &sub == &get_start ();
  }

  bool
  ir_component_loop::
  is_condition (const ir_subcomponent& sub) const noexcept
  {
    return &sub == &get_condition ();
  }

  bool
  ir_component_loop::
  is_body (const ir_subcomponent& sub) const noexcept
  {
    return &sub == &get_body ();
  }

  bool
  ir_component_loop::
  is_update (const ir_subcomponent& sub) const noexcept
  {
    return &sub == &get_update ();
  }

  auto
  ir_component_loop::
  get_id (const ir_subcomponent& c) const
    -> subcomponent_id
  {
    if (is_start     (c)) return subcomponent_id::start;
    if (is_condition (c)) return subcomponent_id::condition;
    if (is_body      (c)) return subcomponent_id::body;
    if (is_update    (c)) return subcomponent_id::update;
    throw ir_exception ("Could not find the specified component in the loop.");
  }

}

/** ir-loop-component.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-all-substructure-visitors.hpp"

#include "ir-error.hpp"

namespace gch
{

  ir_component_loop::
  ir_component_loop (ir_structure& parent, ir_variable& condition_var)
    : ir_substructure (parent),
      m_start     { allocate_subcomponent<ir_block> () },
      m_condition (*this, condition_var),
      m_body      { allocate_subcomponent<ir_component_sequence> () },
      m_update    { allocate_subcomponent<ir_block> () },
      m_after     { allocate_subcomponent<ir_block> () }
  { }

  ir_component_loop::
  ir_component_loop (ir_structure& parent, ir_variable& condition_var, ir_component_mover init)
    : ir_substructure (parent),
      m_start     { init },
      m_condition (*this, condition_var),
      m_body      { allocate_subcomponent<ir_component_sequence> () },
      m_update    { allocate_subcomponent<ir_block> () },
      m_after     { allocate_subcomponent<ir_block> () }
  { }

  ir_component_loop::
  ~ir_component_loop (void) noexcept = default;

  ir_subcomponent&
  ir_component_loop::
  get_start (void) noexcept
  {
    return *m_start;
  }

  const ir_subcomponent&
  ir_component_loop::
  get_start (void) const noexcept
  {
    return as_mutable (*this).get_start ();
  }

  ir_block&
  ir_component_loop::
  get_condition (void) noexcept
  {
    return m_condition;
  }

  const ir_block&
  ir_component_loop::
  get_condition (void) const noexcept
  {
    return as_mutable (*this).get_condition ();
  }

  ir_subcomponent&
  ir_component_loop::
  get_body (void) noexcept
  {
    return *m_body;
  }

  const ir_subcomponent&
  ir_component_loop::
  get_body (void) const noexcept
  {
    return as_mutable (*this).get_body ();
  }

  ir_subcomponent&
  ir_component_loop::
  get_update (void) noexcept
  {
    return *m_update;
  }

  const ir_subcomponent&
  ir_component_loop::
  get_update (void) const noexcept
  {
    return as_mutable (*this).get_update ();
  }

  ir_subcomponent&
  ir_component_loop::
  get_after (void) noexcept
  {
    return *m_after;
  }

  const ir_subcomponent&
  ir_component_loop::
  get_after (void) const noexcept
  {
    return as_mutable (*this).get_after ();
  }

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

  bool
  ir_component_loop::
  is_after (const ir_subcomponent& sub) const noexcept
  {
    return &sub == &get_after ();
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
    if (is_after     (c)) return subcomponent_id::after;
    abort<reason::logic_error> ("Could not find the specified component in the loop.");
  }

}

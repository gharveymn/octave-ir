/** ir-leaf-inspector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/inspectors/ir-leaf-inspector.hpp"

#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"
#include "visitors/ir-visitor.hpp"

namespace gch
{

  /* acceptors */

  template <>
  auto
  acceptor<ir_component_fork, ir_leaf_inspector>::
  accept (visitor_reference v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<ir_component_loop, ir_leaf_inspector>::
  accept (visitor_reference v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<ir_component_sequence, ir_leaf_inspector>::
  accept (visitor_reference v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<ir_function, ir_leaf_inspector>::
  accept (visitor_reference v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  /* ir_leaf_inspector */

  auto
  ir_leaf_inspector::
  operator() (void) const noexcept
    -> result_type
  {
    return get_parent ().accept (*this);
  }

  auto
  ir_leaf_inspector::
  visit (const ir_component_fork& fork) const noexcept
    -> result_type
  {
    return &get_subcomponent () != &fork.get_condition ();
  }

  auto
  ir_leaf_inspector::
  visit (const ir_component_loop& loop) const noexcept
    -> result_type
  {
    return loop.is_condition (get_subcomponent ());
  }

  auto
  ir_leaf_inspector::
  visit (const ir_component_sequence& seq) const noexcept
    -> result_type
  {
    auto found = seq.find (get_subcomponent ());
    assert (found != seq.end ());
    return found == seq.last ();
  }

}

/** ir-leaf-inspector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "structure/inspectors/ir-leaf-inspector.hpp"

#include "ir-component-fork.hpp"
#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-function.hpp"
#include "ir-visitor.hpp"

namespace gch
{

  template class acceptor<ir_component_fork,     inspector_type<ir_leaf_inspector>>;
  template class acceptor<ir_component_loop,     inspector_type<ir_leaf_inspector>>;
  template class acceptor<ir_component_sequence, inspector_type<ir_leaf_inspector>>;
  template class acceptor<ir_function,           inspector_type<ir_leaf_inspector>>;

  template <>
  auto
  ir_leaf_inspector::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_leaf_inspector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_leaf_inspector::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_leaf_inspector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_leaf_inspector::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_leaf_inspector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_leaf_inspector::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_leaf_inspector> v) const
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
    return ! fork.is_condition (get_subcomponent ());
  }

  auto
  ir_leaf_inspector::
  visit (const ir_component_loop& loop) const noexcept
    -> result_type
  {
    return loop.is_after (get_subcomponent ());
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

/** ir-abstract-component-inspector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/component/inspectors/ir-abstract-component-inspector.hpp"

#include "components/ir-all-components.hpp"

namespace gch
{

  template <>
  auto
  ir_abstract_component_inspector::acceptor_type<ir_block>::
  accept (visitor_reference_t<ir_abstract_component_inspector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_abstract_component_inspector::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_abstract_component_inspector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_abstract_component_inspector::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_abstract_component_inspector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_abstract_component_inspector::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_abstract_component_inspector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_abstract_component_inspector::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_abstract_component_inspector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

}

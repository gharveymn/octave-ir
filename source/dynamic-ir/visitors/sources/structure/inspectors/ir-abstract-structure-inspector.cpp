/** ir-abstract-structure-inspector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "structure/inspectors/ir-abstract-structure-inspector.hpp"

#include "ir-all-components.hpp"

namespace gch
{

  template class acceptor<ir_component_fork,     inspector_type<ir_abstract_structure_inspector>>;
  template class acceptor<ir_component_loop,     inspector_type<ir_abstract_structure_inspector>>;
  template class acceptor<ir_component_sequence, inspector_type<ir_abstract_structure_inspector>>;
  template class acceptor<ir_function,           inspector_type<ir_abstract_structure_inspector>>;

  template <>
  auto
  ir_abstract_structure_inspector::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_abstract_structure_inspector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_abstract_structure_inspector::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_abstract_structure_inspector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_abstract_structure_inspector::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_abstract_structure_inspector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_abstract_structure_inspector::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_abstract_structure_inspector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

}

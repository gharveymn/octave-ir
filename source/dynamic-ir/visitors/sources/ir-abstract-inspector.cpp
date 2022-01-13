/** ir-abstract-inspector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-abstract-inspector.hpp"

#include "ir-component-fwd.hpp"

namespace gch
{

  template <>
  abstract_inspector<ir_block>::
  ~abstract_inspector (void)
  { };

  template <>
  abstract_inspector<ir_component_fork>::
  ~abstract_inspector (void)
  { };

  template <>
  abstract_inspector<ir_component_loop>::
  ~abstract_inspector (void)
  { };

  template <>
  abstract_inspector<ir_component_sequence>::
  ~abstract_inspector (void)
  { };

  template <>
  abstract_inspector<ir_function>::
  ~abstract_inspector (void)
  { };

}

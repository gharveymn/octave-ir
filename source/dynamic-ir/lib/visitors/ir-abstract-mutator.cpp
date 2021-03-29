/** ir-abstract-mutator.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/ir-abstract-mutator.hpp"

#include "components/ir-component-fwd.hpp"

namespace gch
{

  template <>
  abstract_mutator<ir_block>::
  ~abstract_mutator (void) = default;

  template <>
  abstract_mutator<ir_component_fork>::
  ~abstract_mutator (void) = default;

  template <>
  abstract_mutator<ir_component_loop>::
  ~abstract_mutator (void) = default;

  template <>
  abstract_mutator<ir_component_sequence>::
  ~abstract_mutator (void) = default;

  template <>
  abstract_mutator<ir_function>::
  ~abstract_mutator (void) = default;

}

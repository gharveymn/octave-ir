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

  template class abstract_inspector<ir_block>;
  template class abstract_inspector<ir_component_fork>;
  template class abstract_inspector<ir_component_loop>;
  template class abstract_inspector<ir_component_sequence>;
  template class abstract_inspector<ir_function>;

}

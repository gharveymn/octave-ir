/** ir-structure-inspectors.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_STRUCTURE_INSPECTORS_HPP
#define OCTAVE_IR_IR_STRUCTURE_INSPECTORS_HPP

#include "ir-entry-collector.hpp"
#include "ir-leaf-inspector.hpp"
#include "ir-predecessor-collector.hpp"
#include "ir-successor-collector.hpp"

namespace gch
{

  extern template class acceptor<ir_component_fork,     inspector_type<ir_entry_collector>>;
  extern template class acceptor<ir_component_loop,     inspector_type<ir_entry_collector>>;
  extern template class acceptor<ir_component_sequence, inspector_type<ir_entry_collector>>;
  extern template class acceptor<ir_function,           inspector_type<ir_entry_collector>>;

  extern template class acceptor<ir_component_fork,     inspector_type<ir_leaf_inspector>>;
  extern template class acceptor<ir_component_loop,     inspector_type<ir_leaf_inspector>>;
  extern template class acceptor<ir_component_sequence, inspector_type<ir_leaf_inspector>>;
  extern template class acceptor<ir_function,           inspector_type<ir_leaf_inspector>>;

  extern template class acceptor<ir_component_fork,     inspector_type<ir_predecessor_collector>>;
  extern template class acceptor<ir_component_loop,     inspector_type<ir_predecessor_collector>>;
  extern template class acceptor<ir_component_sequence, inspector_type<ir_predecessor_collector>>;
  extern template class acceptor<ir_function,           inspector_type<ir_predecessor_collector>>;

  extern template class acceptor<ir_component_fork,     inspector_type<ir_successor_collector>>;
  extern template class acceptor<ir_component_loop,     inspector_type<ir_successor_collector>>;
  extern template class acceptor<ir_component_sequence, inspector_type<ir_successor_collector>>;
  extern template class acceptor<ir_function,           inspector_type<ir_successor_collector>>;

}

#endif // OCTAVE_IR_IR_STRUCTURE_INSPECTORS_HPP

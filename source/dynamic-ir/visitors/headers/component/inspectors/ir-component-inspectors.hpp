/** ir-component-inspectors.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_INSPECTORS_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_INSPECTORS_HPP

#include "ir-block-counter.hpp"
#include "ir-leaf-collector.hpp"
#include "ir-abstract-component-inspector.hpp"
#include "ir-static-function-generator.hpp"

namespace gch
{

  extern template class acceptor<ir_block,              inspector_type<ir_abstract_component_inspector>>;
  extern template class acceptor<ir_component_fork,     inspector_type<ir_abstract_component_inspector>>;
  extern template class acceptor<ir_component_loop,     inspector_type<ir_abstract_component_inspector>>;
  extern template class acceptor<ir_component_sequence, inspector_type<ir_abstract_component_inspector>>;
  extern template class acceptor<ir_function,           inspector_type<ir_abstract_component_inspector>>;

  extern template class acceptor<ir_block,              inspector_type<ir_block_counter>>;
  extern template class acceptor<ir_component_fork,     inspector_type<ir_block_counter>>;
  extern template class acceptor<ir_component_loop,     inspector_type<ir_block_counter>>;
  extern template class acceptor<ir_component_sequence, inspector_type<ir_block_counter>>;
  extern template class acceptor<ir_function,           inspector_type<ir_block_counter>>;

  extern template class acceptor<ir_block,              inspector_type<ir_leaf_collector>>;
  extern template class acceptor<ir_component_fork,     inspector_type<ir_leaf_collector>>;
  extern template class acceptor<ir_component_loop,     inspector_type<ir_leaf_collector>>;
  extern template class acceptor<ir_component_sequence, inspector_type<ir_leaf_collector>>;
  extern template class acceptor<ir_function,           inspector_type<ir_leaf_collector>>;

  extern template class acceptor<ir_block,              inspector_type<ir_const_leaf_collector>>;
  extern template class acceptor<ir_component_fork,     inspector_type<ir_const_leaf_collector>>;
  extern template class acceptor<ir_component_loop,     inspector_type<ir_const_leaf_collector>>;
  extern template class acceptor<ir_component_sequence, inspector_type<ir_const_leaf_collector>>;
  extern template class acceptor<ir_function,           inspector_type<ir_const_leaf_collector>>;

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_INSPECTORS_HPP

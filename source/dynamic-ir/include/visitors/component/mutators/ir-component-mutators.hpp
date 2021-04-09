/** ir-component-mutators.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_MUTATORS_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_MUTATORS_HPP

#include "ir-descending-def-propagator.hpp"
#include "ir-descending-def-resolution-builder.hpp"
#include "ir-descending-forward-mutator.hpp"

namespace gch
{

  extern template class acceptor<ir_block,              mutator_type<ir_descending_def_propagator>>;
  extern template class acceptor<ir_component_fork,     mutator_type<ir_descending_def_propagator>>;
  extern template class acceptor<ir_component_loop,     mutator_type<ir_descending_def_propagator>>;
  extern template class acceptor<ir_component_sequence, mutator_type<ir_descending_def_propagator>>;
  extern template class acceptor<ir_function,           mutator_type<ir_descending_def_propagator>>;

  extern template class acceptor<ir_block,              mutator_type<ir_descending_def_resolution_builder>>;
  extern template class acceptor<ir_component_fork,     mutator_type<ir_descending_def_resolution_builder>>;
  extern template class acceptor<ir_component_loop,     mutator_type<ir_descending_def_resolution_builder>>;
  extern template class acceptor<ir_component_sequence, mutator_type<ir_descending_def_resolution_builder>>;
  extern template class acceptor<ir_function,           mutator_type<ir_descending_def_resolution_builder>>;

  extern template class acceptor<ir_block,              mutator_type<ir_descending_forward_mutator>>;
  extern template class acceptor<ir_component_fork,     mutator_type<ir_descending_forward_mutator>>;
  extern template class acceptor<ir_component_loop,     mutator_type<ir_descending_forward_mutator>>;
  extern template class acceptor<ir_component_sequence, mutator_type<ir_descending_forward_mutator>>;
  extern template class acceptor<ir_function,           mutator_type<ir_descending_forward_mutator>>;

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_MUTATORS_HPP

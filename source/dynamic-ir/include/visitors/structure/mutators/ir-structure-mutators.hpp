/** ir-structure-mutators.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_MUTATORS_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_MUTATORS_HPP

#include "ir-ascending-def-propagator.hpp"
#include "ir-ascending-def-resolution-builder.hpp"
#include "ir-ascending-forward-mutator.hpp"
#include "ir-structure-flattener.hpp"

namespace gch
{

  extern template class acceptor<ir_component_fork,     mutator_type<ir_ascending_def_propagator>>;
  extern template class acceptor<ir_component_loop,     mutator_type<ir_ascending_def_propagator>>;
  extern template class acceptor<ir_component_sequence, mutator_type<ir_ascending_def_propagator>>;
  extern template class acceptor<ir_function,           mutator_type<ir_ascending_def_propagator>>;

  extern template class acceptor<ir_component_fork,     mutator_type<ir_ascending_def_resolution_builder>>;
  extern template class acceptor<ir_component_loop,     mutator_type<ir_ascending_def_resolution_builder>>;
  extern template class acceptor<ir_component_sequence, mutator_type<ir_ascending_def_resolution_builder>>;
  extern template class acceptor<ir_function,           mutator_type<ir_ascending_def_resolution_builder>>;

  extern template class acceptor<ir_component_fork,     mutator_type<ir_ascending_forward_mutator>>;
  extern template class acceptor<ir_component_loop,     mutator_type<ir_ascending_forward_mutator>>;
  extern template class acceptor<ir_component_sequence, mutator_type<ir_ascending_forward_mutator>>;
  extern template class acceptor<ir_function,           mutator_type<ir_ascending_forward_mutator>>;

  extern template class acceptor<ir_component_fork,     mutator_type<ir_structure_flattener>>;
  extern template class acceptor<ir_component_loop,     mutator_type<ir_structure_flattener>>;
  extern template class acceptor<ir_component_sequence, mutator_type<ir_structure_flattener>>;
  extern template class acceptor<ir_function,           mutator_type<ir_structure_flattener>>;

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_MUTATORS_HPP

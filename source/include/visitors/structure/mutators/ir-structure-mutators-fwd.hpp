/** ir-structure-mutators-fwd.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_STRUCTURE_MUTATORS_FWD_HPP
#define OCTAVE_IR_IR_STRUCTURE_MUTATORS_FWD_HPP

#include "visitors/ir-visitor-fwd.hpp"

namespace gch
{

  class ir_structure_flattener;
  class ir_ascending_def_resolution_builder;
  class ir_ascending_forward_mutator;
  class ir_ascending_def_propagator;

  template <>
  struct exclusive_mutators<ir_structure>
  {
    using type = visitor_types<ir_ascending_def_propagator,
                               ir_ascending_def_resolution_builder,
                               ir_ascending_forward_mutator,
                               ir_structure_flattener>;
  };

  class ir_def_resolution_build_result;

  template <>
  struct visitor_traits<ir_ascending_def_propagator>
    : acceptor_trait<ir_ascending_def_propagator>
  {
    using result_type      = void;
    using visitor_category = const_mutator_tag;
  };

  template <>
  struct visitor_traits<ir_ascending_def_resolution_builder>
    : acceptor_trait<ir_ascending_def_resolution_builder>
  {
    using result_type      = ir_def_resolution_build_result;
    using visitor_category = const_mutator_tag;
  };

  template <>
  struct visitor_traits<ir_ascending_forward_mutator>
    : acceptor_trait<ir_ascending_forward_mutator>
  {
    using result_type      = void;
    using visitor_category = const_mutator_tag;
  };

  template <>
  struct visitor_traits<ir_structure_flattener>
    : acceptor_trait<ir_structure_flattener>
  {
    using result_type      = void;
    using visitor_category = const_mutator_tag;
  };

}

#endif // OCTAVE_IR_IR_STRUCTURE_MUTATORS_FWD_HPP

/** ir-component-mutators-fwd.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_MUTATORS_FWD_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_MUTATORS_FWD_HPP

#include "ir-visitor-fwd.hpp"

namespace gch
{

  class ir_descending_def_resolution_builder;
  class ir_descending_forward_mutator;
  class ir_descending_def_propagator;

  template <>
  struct exclusive_mutators<ir_component>
  {
    using type = visitor_types<ir_descending_def_resolution_builder,
                               ir_descending_forward_mutator,
                               ir_descending_def_propagator>;
  };

  template <typename T>
  class ir_link_set;

  class ir_def_resolution_build_result;

  template <>
  struct visitor_traits<ir_descending_def_resolution_builder>
    : acceptor_trait<ir_descending_def_resolution_builder>
  {
    using result_type      = ir_def_resolution_build_result;
    using visitor_category = const_mutator_tag;
  };

  template <>
  struct visitor_traits<ir_descending_forward_mutator>
    : acceptor_trait<ir_descending_forward_mutator>
  {
    using result_type      = bool;
    using visitor_category = const_mutator_tag;
  };

  template <>
  struct visitor_traits<ir_descending_def_propagator>
    : acceptor_trait<ir_descending_def_propagator>
  {
    using result_type      = ir_link_set<ir_block>;
    using visitor_category = const_mutator_tag;
  };



}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_MUTATORS_FWD_HPP

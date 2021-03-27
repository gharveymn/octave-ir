/** ir-component-inspectors-fwd.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_COMPONENT_INSPECTORS_FWD_HPP
#define OCTAVE_IR_IR_COMPONENT_INSPECTORS_FWD_HPP

#include "visitors/ir-visitor-fwd.hpp"

namespace gch
{
  class ir_leaf_collector;
  class ir_const_leaf_collector;

  class ir_block_counter;

  class ir_abstract_component_inspector;

  class ir_block;

  template <>
  struct exclusive_inspectors<ir_component>
  {
    using type = visitor_types<ir_leaf_collector,
                               ir_const_leaf_collector,
                               ir_block_counter,
                               ir_abstract_component_inspector>;
  };

  template <typename T>
  class ir_link_set;

  template <>
  struct visitor_traits<ir_leaf_collector>
    : acceptor_trait<ir_leaf_collector>
  {
    using result_type      = ir_link_set<ir_block>;
    using visitor_category = const_inspector_tag;
  };

  template <>
  struct visitor_traits<ir_const_leaf_collector>
    : acceptor_trait<ir_const_leaf_collector>
  {
    using result_type      = ir_link_set<const ir_block>;
    using visitor_category = const_inspector_tag;
  };

  template <>
  struct visitor_traits<ir_block_counter>
    : acceptor_trait<ir_block_counter>
  {
    using result_type      = std::size_t;
    using visitor_category = const_inspector_tag;
  };

  template <>
  struct visitor_traits<ir_abstract_component_inspector>
    : acceptor_trait<ir_abstract_component_inspector>
  {
    using result_type      = void;
    using visitor_category = inspector_tag;
  };

}

#endif // OCTAVE_IR_IR_COMPONENT_INSPECTORS_FWD_HPP

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

  template <>
  struct exclusive_inspectors<ir_component>
  {
    using type = visitor_types<ir_leaf_collector>;
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



}

#endif // OCTAVE_IR_IR_COMPONENT_INSPECTORS_FWD_HPP

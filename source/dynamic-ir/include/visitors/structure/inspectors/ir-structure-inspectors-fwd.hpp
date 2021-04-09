/** ir-structure-inspectors-fwd.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_INSPECTORS_FWD_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_INSPECTORS_FWD_HPP

#include "visitors/ir-visitor-fwd.hpp"

namespace gch
{

  class ir_entry_collector;
  class ir_predecessor_collector;
  class ir_successor_collector;
  class ir_leaf_inspector;
  class ir_abstract_structure_inspector;

  template <>
  struct exclusive_inspectors<ir_structure>
  {
    using type = visitor_types<ir_entry_collector,
                               ir_predecessor_collector,
                               ir_successor_collector,
                               ir_leaf_inspector,
                               ir_abstract_structure_inspector>;
  };

  class ir_subcomponent;

  template <typename T>
  class ir_link_set;

  template <>
  struct visitor_traits<ir_entry_collector>
    : acceptor_trait<ir_entry_collector>
  {
    using result_type      = const ir_subcomponent&;
    using visitor_category = const_inspector_tag;
  };

  template <>
  struct visitor_traits<ir_predecessor_collector>
    : acceptor_trait<ir_predecessor_collector>
  {
    using result_type      = ir_link_set<ir_block>;
    using visitor_category = const_inspector_tag;
  };

  template <>
  struct visitor_traits<ir_successor_collector>
    : acceptor_trait<ir_successor_collector>
  {
    using result_type      = ir_link_set<ir_block>;
    using visitor_category = const_inspector_tag;
  };

  template <>
  struct visitor_traits<ir_leaf_inspector>
    : acceptor_trait<ir_leaf_inspector>
  {
    using result_type      = bool;
    using visitor_category = const_inspector_tag;
  };

  template <>
  struct visitor_traits<ir_abstract_structure_inspector>
    : acceptor_trait<ir_abstract_structure_inspector>
  {
    using result_type      = void;
    using visitor_category = inspector_tag;
  };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_INSPECTORS_FWD_HPP

/** ir-structure-inspectors-fwd.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_INSPECTORS_FWD_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_INSPECTORS_FWD_HPP

#include "ir-visitor-fwd.hpp"

#include <gch/small_vector.hpp>
#include <gch/nonnull_ptr.hpp>

namespace gch
{

  class ir_abstract_structure_inspector;
  class ir_ascending_def_resolution_builder;
  class ir_entry_collector;
  class ir_predecessor_collector;
  class ir_successor_collector;
  class ir_leaf_inspector;

  class ir_def_resolution_build_result;

  template <>
  struct exclusive_inspectors<ir_structure>
  {
    using type = visitor_types<ir_abstract_structure_inspector,
                               ir_ascending_def_resolution_builder,
                               ir_entry_collector,
                               ir_predecessor_collector,
                               ir_successor_collector,
                               ir_leaf_inspector>;
  };

  class ir_subcomponent;

  template <typename T>
  class ir_link_set;

  template <>
  struct visitor_traits<ir_abstract_structure_inspector>
    : acceptor_trait<ir_abstract_structure_inspector>
  {
    using result_type      = void;
    using visitor_category = inspector_tag;
  };

  template <>
  struct visitor_traits<ir_ascending_def_resolution_builder>
    : acceptor_trait<ir_ascending_def_resolution_builder>
  {
    using result_type      = ir_def_resolution_build_result;
    using visitor_category = inspector_tag;
  };

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
    using result_type      = small_vector<nonnull_ptr<ir_block>>;
    using visitor_category = const_inspector_tag;
  };

  template <>
  struct visitor_traits<ir_leaf_inspector>
    : acceptor_trait<ir_leaf_inspector>
  {
    using result_type      = bool;
    using visitor_category = const_inspector_tag;
  };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_INSPECTORS_FWD_HPP

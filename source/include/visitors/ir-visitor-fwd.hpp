/** ir-visitor-fwd.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_VISITOR_FWD_HPP
#define OCTAVE_IR_IR_VISITOR_FWD_HPP

#include "components/ir-component-fwd.hpp"

namespace gch
{

  template <typename ...Visitors>
  struct visitor_types;

  template <typename, typename, typename>
  struct acceptor;

  struct const_visitor_tag;

  struct inspector_tag;
  struct const_inspector_tag;

  struct mutator_tag;
  struct const_mutator_tag;


  template <typename Visitor>
  struct visitor_traits;


  template <typename Visitor>
  struct acceptor_trait
  {
    template <typename Concrete>
    using acceptor_type = acceptor<Concrete, Visitor, void>;
  };

  template <typename Component>
  struct abstract_visitors
  {
    using type = visitor_types<>;
  };

  template <typename Component>
  using abstract_visitors_t = typename abstract_visitors<Component>::type;

  template <typename ...Components>
  struct implemented_visitors;

  template <typename ...Components>
  using implemented_visitors_t = typename implemented_visitors<Components...>::type;

  template <typename ...Components>
  struct implemented_visitors
  {
    using type = pack_unique_t<pack_flatten_t<pack_concatenate_t<
      typename implemented_visitors<Components>::type...>>>;
  };

  template <typename Component>
  struct implemented_visitors<Component>
    : abstract_visitors<Component>
  { };

  template <typename Component, typename ...BaseComponents>
  struct merged_base_visitors
  {
    using type = pack_unique_t<pack_flatten_t<pack_concatenate_t<
      abstract_visitors_t<Component>,
      implemented_visitors_t<BaseComponents>...>>>;
  };

  //
  // component visitors
  //

  /* ir_leaf_collector */

  template <typename T>
  class ir_link_set;

  class ir_leaf_collector;
  template <>
  struct visitor_traits<ir_leaf_collector>
    : acceptor_trait<ir_leaf_collector>
  {
    using result_type      = ir_link_set<ir_block>;
    using visitor_category = const_inspector_tag;
  };

  /* component_inspector_types */

  using component_inspector_types = visitor_types<ir_leaf_collector>;

  /* ir_descending_def_resolution_builder */

  class ir_def_resolution_build_result;
  class ir_descending_def_resolution_builder;
  template <>
  struct visitor_traits<ir_descending_def_resolution_builder>
    : acceptor_trait<ir_descending_def_resolution_builder>
  {
    using result_type      = ir_def_resolution_build_result;
    using visitor_category = const_mutator_tag;
  };

  /* ir_descending_forward_mutator */

  class ir_descending_forward_mutator;
  template <>
  struct visitor_traits<ir_descending_forward_mutator>
    : acceptor_trait<ir_descending_forward_mutator>
  {
    using result_type      = bool;
    using visitor_category = const_mutator_tag;
  };

  /* ir_descending_def_propagator */

  class ir_descending_def_propagator;
  template <>
  struct visitor_traits<ir_descending_def_propagator>
    : acceptor_trait<ir_descending_def_propagator>
  {
    using result_type      = ir_link_set<ir_block>;
    using visitor_category = const_mutator_tag;
  };

  /* component_mutator_types */

  using component_mutator_types = visitor_types<ir_descending_def_resolution_builder,
                                                ir_descending_forward_mutator,
                                                ir_descending_def_propagator>;

  /* ir_component_visitors */

  template <>
  struct abstract_visitors<ir_component>
  {
    using type = visitor_types<component_inspector_types,
                               component_mutator_types>;
  };

  using ir_component_visitors_exclusive = visitor_types<component_inspector_types,
                                                        component_mutator_types>;

  //
  // subcomponent visitors
  //

  /* ir_block_counter */

  class ir_block_counter;
  template <>
  struct visitor_traits<ir_block_counter>
    : acceptor_trait<ir_block_counter>
  {
    using result_type      = std::size_t;
    using visitor_category = const_inspector_tag;
  };

  /* subcomponent_inspector_types */

  using subcomponent_inspector_types = visitor_types<ir_block_counter>;

  /* subcomponent_mutator_types */

  using subcomponent_mutator_types = visitor_types<>;

  template <>
  struct abstract_visitors<ir_subcomponent>
  {
    using type = visitor_types<subcomponent_inspector_types,
                               subcomponent_mutator_types>;
  };

  template <>
  struct implemented_visitors<ir_subcomponent>
    : merged_base_visitors<ir_subcomponent, ir_component>
  { };

  // aggregate
  using ir_subcomponent_visitors_exclusive = visitor_types<subcomponent_inspector_types,
                                                           subcomponent_mutator_types>;

  //
  // structure visitors
  //

  class ir_entry_collector;
  template <>
  struct visitor_traits<ir_entry_collector>
    : acceptor_trait<ir_entry_collector>
  {
    using result_type      = const ir_subcomponent&;
    using visitor_category = const_inspector_tag;
  };

  /* ir_predecessor_collector */

  class ir_predecessor_collector;
  template <>
  struct visitor_traits<ir_predecessor_collector>
    : acceptor_trait<ir_predecessor_collector>
  {
    using result_type      = ir_link_set<ir_block>;
    using visitor_category = const_inspector_tag;
  };

  /* ir_successor_collector */

  class ir_successor_collector;
  template <>
  struct visitor_traits<ir_successor_collector>
    : acceptor_trait<ir_successor_collector>
  {
    using result_type      = ir_link_set<ir_block>;
    using visitor_category = const_inspector_tag;
  };

  /* ir_leaf_inspector */

  class ir_leaf_inspector;
  template <>
  struct visitor_traits<ir_leaf_inspector>
    : acceptor_trait<ir_leaf_inspector>
  {
    using result_type      = bool;
    using visitor_category = const_inspector_tag;
  };

  /* structure_inspector_types */

  using structure_inspector_types = visitor_types<ir_entry_collector,
                                                  ir_predecessor_collector,
                                                  ir_successor_collector,
                                                  ir_leaf_inspector>;

  /* ir_structure_flattener */

  class ir_structure_flattener;
  template <>
  struct visitor_traits<ir_structure_flattener>
    : acceptor_trait<ir_structure_flattener>
  {
    using result_type      = void;
    using visitor_category = const_mutator_tag;
  };

  /* ir_ascending_def_resolution_builder */

  class ir_ascending_def_resolution_builder;
  template <>
  struct visitor_traits<ir_ascending_def_resolution_builder>
    : acceptor_trait<ir_ascending_def_resolution_builder>
  {
    using result_type      = ir_def_resolution_build_result;
    using visitor_category = const_mutator_tag;
  };

  /* ir_ascending_forward_mutator */

  class ir_ascending_forward_mutator;
  template <>
  struct visitor_traits<ir_ascending_forward_mutator>
    : acceptor_trait<ir_ascending_forward_mutator>
  {
    using result_type      = void;
    using visitor_category = const_mutator_tag;
  };

  /* ir_ascending_def_propagator */

  class ir_ascending_def_propagator;
  template <>
  struct visitor_traits<ir_ascending_def_propagator>
    : acceptor_trait<ir_ascending_def_propagator>
  {
    using result_type      = void;
    using visitor_category = const_mutator_tag;
  };

  /* structure_mutator_types */

  using structure_mutator_types = visitor_types<ir_structure_flattener,
                                                ir_ascending_def_resolution_builder,
                                                ir_ascending_forward_mutator,
                                                ir_ascending_def_propagator>;

  template <>
  struct abstract_visitors<ir_structure>
  {
    using type = visitor_types<structure_inspector_types,
                               structure_mutator_types>;
  };

  // aggregate
  using ir_structure_visitors_exclusive = visitor_types<structure_inspector_types,
                                                        structure_mutator_types>;

  //
  // substructure visitors
  //

  template <>
  struct abstract_visitors<ir_substructure>
  {
    using type = visitor_types<>;
  };

  template <>
  struct implemented_visitors<ir_substructure>
    : merged_base_visitors<ir_substructure, ir_structure, ir_subcomponent>
  { };

}

#endif // OCTAVE_IR_IR_VISITOR_FWD_HPP

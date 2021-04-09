/** ir-visitor-fwd.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_VISITOR_FWD_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_VISITOR_FWD_HPP

#include "components/ir-component-fwd.hpp"
#include "gch/octave-ir-utilities/ir-type-traits.hpp"

namespace gch
{

  //
  // visitor_types
  //

  template <typename ...Visitors>
  struct visitor_types;

  template <typename Visitor>
  struct inspector_type;

  template <typename Visitor>
  struct mutator_type;

  struct const_visitor_tag                                      { };

  struct inspector_tag                                          { };
  struct const_inspector_tag : inspector_tag, const_visitor_tag { };

  struct mutator_tag                                            { };
  struct const_mutator_tag : mutator_tag, const_visitor_tag     { };

  template <typename Visitor>
  struct visitor_traits
  { };

  template <typename ...Ts>
  struct type_pack;

  namespace detail
  {

    template <typename T, typename Traits = visitor_traits<T>, typename Enable = void>
    struct valid_visitor_traits
      : std::false_type
    { };

    template <typename T, typename Traits>
    struct valid_visitor_traits<T, Traits, std::void_t<typename Traits::result_type,
                                                       typename Traits::visitor_category>>
      : std::true_type
    { };

  } // namespace detail

  template <typename Visitor>
  struct visitor_result
  {
    static_assert (detail::valid_visitor_traits<Visitor>::value, "Invalid visitor traits.");
    using type = typename visitor_traits<Visitor>::result_type;
  };

  template <typename Visitor>
  using visitor_result_t = typename visitor_result<Visitor>::type;

  template <typename Visitor>
  struct visitor_category
  {
    static_assert (detail::valid_visitor_traits<Visitor>::value, "Invalid visitor traits.");
    using type = typename visitor_traits<Visitor>::visitor_category;
  };

  template <typename Visitor>
  using visitor_category_t = typename visitor_category<Visitor>::type;

  // note: visitor_arguments not used yet

  namespace detail
  {

    template <typename Visitor, typename Enable = void>
    struct visitor_arguments_impl
    {
      using type = type_pack<>;
    };

    template <typename Visitor>
    struct visitor_arguments_impl<Visitor, std::void_t<typename Visitor::visitor_arguments>>
    {
      using type = typename Visitor::visitor_arguments;
    };

  } // namespace detail

  template <typename Visitor>
  struct visitor_arguments
    : detail::visitor_arguments_impl<Visitor>
  { };

  template <typename Visitor>
  using visitor_arguments_t = typename visitor_arguments<Visitor>::type;

  template <typename Visitor>
  struct is_inspector
    : std::bool_constant<std::is_base_of_v<inspector_tag, visitor_category_t<Visitor>>>
  { };

  template <typename Visitor>
  inline constexpr bool is_inspector_v = is_inspector<Visitor>::value;

  template <typename Visitor>
  struct is_mutator
    : std::bool_constant<std::is_base_of_v<mutator_tag, visitor_category_t<Visitor>>>
  { };

  template <typename Visitor>
  inline constexpr bool is_mutator_v = is_mutator<Visitor>::value;

  template <typename Visitor>
  struct is_const_visitor
    : std::bool_constant<std::is_base_of_v<const_visitor_tag, visitor_category_t<Visitor>>>
  { };

  template <typename Visitor>
  inline constexpr bool is_const_visitor_v = is_const_visitor<Visitor>::value;

  template <typename Visitor>
  struct visitor_reference
  {
    using type = std::conditional_t<is_const_visitor_v<Visitor>, const Visitor&, Visitor&>;
  };

  template <typename Visitor>
  using visitor_reference_t = typename visitor_reference<Visitor>::type;

  template <typename Visitor>
  struct visitor_category_wrapper
    : std::conditional<is_inspector_v<Visitor>, inspector_type<Visitor>, mutator_type<Visitor>>
  {
    static_assert (is_inspector_v<Visitor> || is_mutator_v<Visitor>,
                   "Visitor should be either an inspector or a mutator.");
  };

  template <typename Visitor>
  using visitor_category_wrapper_t = typename visitor_category_wrapper<Visitor>::type;

  template <typename, typename>
  class acceptor;

  template <typename Visitor>
  struct acceptor_trait
  {
    template <typename V = Visitor>
    using category_wrapper = visitor_category_wrapper_t<V>;

    template <typename Concrete, typename V = Visitor>
    using acceptor_type = acceptor<Concrete, category_wrapper<V>>;
  };

  template <typename Component>
  struct exclusive_inspectors
  {
    using type = visitor_types<>;
  };

  template <typename Component>
  using exclusive_inspectors_t = typename exclusive_inspectors<Component>::type;

  template <typename Component>
  struct exclusive_mutators
  {
    using type = visitor_types<>;
  };

  template <typename Component>
  using exclusive_mutators_t = typename exclusive_mutators<Component>::type;

  template <typename Component>
  struct exclusive_visitors
    : pack_union<exclusive_inspectors_t<Component>, exclusive_mutators_t<Component>>
  { };

  template <typename Component>
  using exclusive_visitors_t = typename exclusive_visitors<Component>::type;

  template <typename ...Components>
  struct consolidated_visitors;

  template <typename ...Components>
  using consolidated_visitors_t = typename consolidated_visitors<Components...>::type;

  template <typename ...Components>
  struct consolidated_visitors
  {
    using type = pack_unique_t<pack_flatten_t<pack_concatenate_t<
      typename consolidated_visitors<Components>::type...>>>;
  };

  template <typename Component>
  struct consolidated_visitors<Component>
    : exclusive_visitors<Component>
  { };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_VISITOR_FWD_HPP

/** ir-visitor.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_VISITOR_HPP
#define OCTAVE_IR_IR_VISITOR_HPP

#include <type_traits>

namespace gch
{

  //
  // visitor_types
  //

  template <typename ...Visitors>
  struct visitor_types;

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

  //
  // abstract_inspector
  //

  template <typename ...Types>
  struct abstract_inspector
    : abstract_inspector<Types>...
  {
    using abstract_inspector<Types>::visit...;
  };

  template <typename T>
  struct abstract_inspector<T>
  {
    abstract_inspector            (void)                          = default;
    abstract_inspector            (const abstract_inspector&)     = default;
    abstract_inspector            (abstract_inspector&&) noexcept = default;
    abstract_inspector& operator= (const abstract_inspector&)     = default;
    abstract_inspector& operator= (abstract_inspector&&) noexcept = default;
    virtual ~abstract_inspector   (void)                          = default;

    virtual
    void
    visit (const T&) = 0;
  };

  //
  // abstract_mutator
  //

  template <typename ...Types>
  struct abstract_mutator
    : abstract_mutator<Types>...
  {
    using abstract_mutator<Types>::visit...;
  };

  template <typename T>
  struct abstract_mutator<T>
  {
    abstract_mutator            (void)                        = default;
    abstract_mutator            (const abstract_mutator&)     = default;
    abstract_mutator            (abstract_mutator&&) noexcept = default;
    abstract_mutator& operator= (const abstract_mutator&)     = default;
    abstract_mutator& operator= (abstract_mutator&&) noexcept = default;
    virtual ~abstract_mutator   (void)                        = default;

    virtual
    void
    visit (T&) = 0;
  };

  //
  // abstract_visitable
  //

  template <typename Visitor, typename Enable = void>
  struct abstract_acceptor
  {
    static_assert (   is_inspector_v<Visitor> || is_mutator_v<Visitor>,
                   "Visitor must be either an inspector or a mutator.");
    static_assert (! (is_inspector_v<Visitor> || is_mutator_v<Visitor>),
                   "SFINAE not working properly.");
  };

  template <typename Inspector>
  struct abstract_acceptor<Inspector, std::enable_if_t<is_inspector_v<Inspector>>>
  {
  protected:
    abstract_acceptor            (void)                         = default;
    abstract_acceptor            (const abstract_acceptor&)     = default;
    abstract_acceptor            (abstract_acceptor&&) noexcept = default;
    abstract_acceptor& operator= (const abstract_acceptor&)     = default;
    abstract_acceptor& operator= (abstract_acceptor&&) noexcept = default;
    ~abstract_acceptor           (void)                         = default;

  public:
    using visitor_reference = visitor_reference_t<Inspector>;
    using result_type       = visitor_result_t<Inspector>;

    virtual
    result_type
    accept (visitor_reference) const = 0;
  };

  template <typename Mutator>
  struct abstract_acceptor<Mutator, std::enable_if_t<is_mutator_v<Mutator>>>
  {
  protected:
    abstract_acceptor            (void)                         = default;
    abstract_acceptor            (const abstract_acceptor&)     = default;
    abstract_acceptor            (abstract_acceptor&&) noexcept = default;
    abstract_acceptor& operator= (const abstract_acceptor&)     = default;
    abstract_acceptor& operator= (abstract_acceptor&&) noexcept = default;
    ~abstract_acceptor           (void)                         = default;

  public:
    using visitor_reference = visitor_reference_t<Mutator>;
    using result_type       = visitor_result_t<Mutator>;

    virtual
    result_type
    accept (visitor_reference) = 0;
  };

  template <typename Visitor>
  struct abstract_visitable
    : virtual abstract_acceptor<Visitor>
  {
    using abstract_acceptor<Visitor>::accept;
  };

  template <typename ...VisitorSubTypes>
  struct abstract_visitable<visitor_types<VisitorSubTypes...>>
    : abstract_visitable<VisitorSubTypes>...
  {
    using abstract_visitable<VisitorSubTypes>::accept...;
  };

  //
  // visitable
  //

  template <typename Derived, typename Visitor, typename Enable = void>
  struct acceptor
  {
    static_assert (   is_inspector_v<Visitor> || is_mutator_v<Visitor>,
                   "Visitor must be either an inspector or a mutator.");
    static_assert (! (is_inspector_v<Visitor> || is_mutator_v<Visitor>),
                   "SFINAE not working properly.");
  };

  template <typename Derived, typename Inspector>
  struct acceptor<Derived, Inspector, std::enable_if_t<is_inspector_v<Inspector>>>
    : virtual abstract_acceptor<Inspector>
  {
    using concrete_reference = const Derived&;
    using visitor_reference  = typename abstract_acceptor<Inspector>::visitor_reference;
    using result_type        = typename abstract_acceptor<Inspector>::result_type;

    result_type
    accept (visitor_reference) const override;
  };

  template <typename Derived, typename Mutator>
  struct acceptor<Derived, Mutator, std::enable_if_t<is_mutator_v<Mutator>>>
    : virtual abstract_acceptor<Mutator>
  {
    using concrete_reference = Derived&;
    using visitor_reference  = typename abstract_acceptor<Mutator>::visitor_reference;
    using result_type        = typename abstract_acceptor<Mutator>::result_type;

    result_type
    accept (visitor_reference) override;
  };

  template <typename Derived, typename Visitor>
  struct visitable
    : acceptor<Derived, Visitor>
  {
    using acceptor<Derived, Visitor>::accept;
  };

  template <typename Derived, typename ...VisitorSubTypes>
  struct visitable<Derived, visitor_types<VisitorSubTypes...>>
    : visitable<Derived, VisitorSubTypes>...
  {
    using visitable<Derived, VisitorSubTypes>::accept...;
  };

}

#endif // OCTAVE_IR_IR_VISITOR_HPP

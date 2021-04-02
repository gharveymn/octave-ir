/** ir-type-traits.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_TYPE_TRAITS_HPP
#define OCTAVE_IR_IR_TYPE_TRAITS_HPP

#include <iterator>
#include <type_traits>

namespace gch
{

  template <typename ...Ts>
  struct type_pack
  { };

  template <typename ...Ts>
  struct all_same;

  template <typename ...Ts>
  inline constexpr
  bool
  all_same_v = all_same<Ts...>::value;

  template <typename It>
  struct is_iterator;

  template <typename It>
  inline constexpr
  bool
  is_iterator_v = is_iterator<It>::value;

  template <typename Pack>
  struct is_type_pack;

  template <typename Pack>
  inline constexpr
  bool
  is_type_pack_v = is_type_pack<Pack>::value;

  template <typename Pack>
  struct pack_size;

  template <typename Pack>
  inline constexpr
  std::size_t
  pack_size_v = pack_size<Pack>::value;

  template <typename Pack>
  struct pack_empty;

  template <typename Pack>
  inline constexpr bool pack_empty_v = pack_empty<Pack>::value;

  template <std::size_t I, typename ...Ts>
  struct pack_select_type;

  template <std::size_t I, typename ...Ts>
  using pack_select_t = typename pack_select_type<I, Ts...>::type;

  template <typename T, typename Pack>
  struct pack_index;

  template <typename T, typename Pack>
  inline constexpr
  std::size_t
  pack_index_v = pack_index<T, Pack>::value;

  template <typename T,  typename Pack>
  struct pack_contains;

  template <typename T, typename Pack>
  inline constexpr
  bool
  pack_contains_v = pack_contains<T, Pack>::value;

  template <typename ...Packs>
  struct pack_concatenate;

  template <typename ...Packs>
  using pack_concatenate_t = typename pack_concatenate<Packs...>::type;

  template <typename Pack>
  struct pack_flatten;

  template <typename Pack>
  using pack_flatten_t = typename pack_flatten<Pack>::type;

  template <typename Pack>
  struct pack_unique;

  template <typename Pack>
  using pack_unique_t = typename pack_unique<Pack>::type;

  template <typename ...Pack>
  struct pack_union;

  template <typename ...Pack>
  using pack_union_t = typename pack_union<Pack...>::type;

  template <typename ...Packs>
  struct pack_equivalent;

  template <typename ...Packs>
  inline constexpr bool pack_equivalent_v = pack_equivalent<Packs...>::value;

  template <typename Pack>
  struct pack_homogenenous;

  template <typename Pack>
  inline constexpr bool pack_homogenenous_v = pack_homogenenous<Pack>::value;

  namespace detail
  {

    template <typename T, typename ...Ts>
    struct all_same_impl : std::conjunction<std::is_same<T, Ts>...>
    { };

    template <typename Pack>
    struct is_type_pack_impl
      : std::false_type
    { };

    template <template <typename ...> typename PackT, typename ...Ts>
    struct is_type_pack_impl<PackT<Ts...>>
      : std::true_type
    { };

    template <typename Pack>
    struct pack_size_impl
    { };

    template <template <typename ...> typename PackT, typename ...Ts>
    struct pack_size_impl<PackT<Ts...>>
      : std::integral_constant<std::size_t, sizeof...(Ts)>
    { };

    template <template <auto ...> typename PackT, auto ...Vs>
    struct pack_size_impl<PackT<Vs...>>
      : std::integral_constant<std::size_t, sizeof...(Vs)>
    { };

    template <std::size_t I, typename Pack>
    struct pack_select_impl
    { };

    template <template <typename ...> typename PackT, typename Head, typename ...Tail>
    struct pack_select_impl<0, PackT<Head, Tail...>>
    {
      using type = Head;
    };

    template <std::size_t I,
              template <typename ...> typename PackT, typename Head, typename ...Tail>
    struct pack_select_impl<I, PackT<Head, Tail...>>
      : pack_select_impl<I - 1, PackT<Tail...>>
    { };

    template <typename T, typename Pack, std::size_t I = 0>
    struct pack_index_impl
    { };

    template <typename T,
              template <typename ...> typename PackT, typename ...Rest,
              std::size_t I>
    struct pack_index_impl<T, PackT<T, Rest...>, I>
      : std::integral_constant<std::size_t, I>
    { };

    template <typename T,
              template <typename ...> typename PackT, typename Head, typename ...Tail,
              std::size_t I>
    struct pack_index_impl<T, PackT<Head, Tail...>, I>
      : pack_index_impl<T, PackT<Tail...>, I + 1>
    { };

    template <typename T, typename Pack, typename = void>
    struct pack_contains_impl
      : std::false_type
    { };

    template <typename T, typename Pack>
    struct pack_contains_impl<T, Pack, std::void_t<typename pack_index<T, Pack>::type>>
      : std::true_type
    { };

    template <typename ...Packs>
    struct pack_concatenate_impl
    { };

    template <typename Pack>
    struct pack_concatenate_impl<Pack>
    {
      using type = Pack;
    };

    template <template <typename ...> typename PackT,
              typename ...LHS, typename ...RHS, typename ...Rest>
    struct pack_concatenate_impl<PackT<LHS...>, PackT<RHS...>, Rest...>
      : pack_concatenate_impl<PackT<LHS..., RHS...>, Rest...>
    { };

    template <typename Pack>
    struct pack_flatten_impl
    {
      using type = Pack;
    };

    template <template <typename ...> typename PackT, typename Head, typename ...Tail>
    struct pack_flatten_impl<PackT<Head, Tail...>>
      : pack_concatenate<PackT<Head>, pack_flatten_t<PackT<Tail...>>>
    { };

    template <template <typename ...> typename PackT, typename... Ts, typename ...Tail>
    struct pack_flatten_impl<PackT<PackT<Ts...>, Tail...>>
      : pack_concatenate<pack_flatten_t<PackT<Ts>>..., pack_flatten_t<PackT<Tail...>>>
    { };

    template <typename PackIn, typename PackOut>
    struct pack_unique_helper
    { };

    template <template <typename ...> typename PackT, typename ...OutTs>
    struct pack_unique_helper<PackT<>, PackT<OutTs...>>
    {
      using type = PackT<OutTs...>;
    };

    template <template <typename ...> typename PackT,
              typename InHead, typename ...InTail,
              typename ...OutTs>
    struct pack_unique_helper<PackT<InHead, InTail...>, PackT<OutTs...>>
      : pack_unique_helper<PackT<InTail...>,
                           std::conditional_t<pack_contains_v<InHead, PackT<OutTs...>>,
                                              PackT<OutTs...>,
                                              PackT<InHead, OutTs...>>>
    { };

    template <typename Pack>
    struct pack_unique_impl
    { };

    template <template <typename ...> typename PackT, typename ...Ts>
    struct pack_unique_impl<PackT<Ts...>>
      : pack_unique_helper<PackT<Ts...>, PackT<>>
    { };

    template <typename ...Packs>
    struct pack_equivalent_impl
      : std::false_type
    { };

    template <template <typename ...> typename PackTLHS, typename PackRHS>
    struct pack_equivalent_impl<PackTLHS<>, PackRHS>
      : pack_empty<PackRHS>
    { };

    template <template <typename ...> typename PackTLHS, typename ...TsLHS, typename PackRHS>
    struct pack_equivalent_impl<PackTLHS<TsLHS...>, PackRHS>
      : std::conjunction<pack_contains<TsLHS, PackRHS>...>
    { };

    template <typename PackLHS, typename ...Packs>
    struct pack_equivalent_impl<PackLHS, Packs...>
      : std::conjunction<pack_equivalent_impl<PackLHS, Packs>...>
    { };

    template <typename Pack>
    struct pack_homogeneous
      : std::false_type
    { };

    template <template <typename ...> typename PackT, typename Head, typename ...Tail>
    struct pack_homogeneous<PackT<Head, Tail...>>
      : std::conjunction<std::is_same<Head, Tail>...>
    { };

  }

  template <typename Pack>
  struct pack_size
    : detail::pack_size_impl<Pack>
  { };

  template <typename Pack>
  struct pack_empty
    : std::bool_constant<pack_size_v<Pack> == 0>
  { };

  template <typename ...Ts>
  struct all_same
    : detail::all_same_impl<Ts...>
  { };

  template <typename Pack>
  struct is_type_pack
    : detail::is_type_pack_impl<Pack>
  { };

  template <std::size_t I, typename ...Ts>
  struct pack_select_type
    : detail::pack_select_impl<I, Ts...>
  { };

  template <typename T, typename Pack>
  struct pack_index
    : detail::pack_index_impl<T, Pack, 0>
  { };

  template <typename T, typename Pack>
  struct pack_contains
    : detail::pack_contains_impl<T, Pack>
  { };

  template <typename ...Packs>
  struct pack_concatenate
    : detail::pack_concatenate_impl<Packs...>
  { };

  template <typename Pack>
  struct pack_flatten
    : detail::pack_flatten_impl<Pack>
  { };

  template <typename ...Pack>
  struct pack_union
    : pack_unique<pack_concatenate_t<Pack...>>
  { };

  template <typename Pack>
  struct pack_unique
    : detail::pack_unique_impl<Pack>
  { };

  template <typename ...Packs>
  struct pack_equivalent
    : detail::pack_equivalent_impl<Packs...>
  { };

  namespace detail
  {

    template <typename It, typename = void>
    struct is_iterator_impl
      : std::false_type
    { };

    template <typename It>
    struct is_iterator_impl<It, std::void_t<typename std::iterator_traits<It>::value_type>>
      : std::true_type
    { };

  } // namespace detail

  template <typename It>
  struct is_iterator
    : detail::is_iterator_impl<It>
  { };

  template <typename From, typename To>
  struct match_cv
  {
    using type = std::conditional_t<std::is_const_v<From>,
                                    std::conditional_t<std::is_volatile_v<From>,
                                                       std::add_cv_t<To>,
                                                       std::add_const_t<To>>,
                                    std::conditional_t<std::is_volatile_v<From>,
                                                       std::add_volatile_t<To>,
                                                       To>>;
  };

  template <typename From, typename To>
  using match_cv_t = typename match_cv<From, To>::type;

  template <typename From, typename To>
  struct match_ref
  {
    using type = std::conditional_t<std::is_lvalue_reference_v<From>,
                                    std::add_lvalue_reference_t<To>,
                                    std::conditional_t<std::is_rvalue_reference_v<From>,
                                                       std::add_rvalue_reference_t<To>,
                                                       To>>;
  };

  template <typename From, typename To>
  using match_ref_t = typename match_ref<From, To>::type;

  template <typename From, typename To>
  struct match_cvref
    : match_ref<From, match_cv_t<std::remove_reference_t<From>, To>>
  { };

  template <typename From, typename To>
  using match_cvref_t = typename match_cvref<From, To>::type;

  template <typename T>
  struct remove_cvref
  {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
  };

  template <typename T>
  using remove_cvref_t = typename remove_cvref<T>::type;

  template <typename T, typename U,
            template<typename> typename TQual, template<typename> typename UQual>
  struct basic_common_reference
  { };

  template <typename ...Ts>
  struct common_reference;

  template <typename ...Ts>
  using common_reference_t = typename common_reference<Ts...>::type;

  namespace detail
  {

    template <typename From>
    struct cvref_matcher
    {
      template <typename To>
      using type = match_cvref_t<From, To>;
    };

    template <typename T, typename U>
    using common_reference_ternary_result_t = decltype (false ? std::declval<T (&) (void)> () ()
                                                              : std::declval<U (&) (void)> () ());

    template <typename T, typename U>
    using resolved_basic_common_reference_t
      = typename basic_common_reference<remove_cvref_t<T>,
                                        remove_cvref_t<U>,
                                        cvref_matcher<T>::template type,
                                        cvref_matcher<U>::template type>::type;

    template <typename T, typename U, typename Enable = void>
    struct simple_common_reference
    { };

    template <typename T, typename U>
    using simple_common_reference_t = typename simple_common_reference<T, U>::type;

    template <typename X, typename Y>
    struct simple_common_reference<X&, Y&, std::void_t<common_reference_ternary_result_t<
                                                         match_cv_t<X, Y>&,
                                                         match_cv_t<Y, X>&>>>
    {
      using type = common_reference_ternary_result_t<match_cv_t<X, Y>&, match_cv_t<Y, X>&>;
    };

    template <typename X, typename Y>
    struct simple_common_reference<X&&, Y&&,
      std::enable_if_t<std::is_convertible_v<X&&, simple_common_reference_t<X&, Y&>&&>
                   &&  std::is_convertible_v<Y&&, simple_common_reference_t<X&, Y&>&&>>>
    {
      using type = simple_common_reference_t<X&, Y&>&&;
    };

    template <typename A, typename B>
    struct simple_common_reference<A&, B&&,
      std::enable_if_t<std::is_convertible_v<B&&, simple_common_reference_t<A&, const B&>>>>
    {
      using type = simple_common_reference_t<A&, const B&>;
    };

    template <typename B, typename A>
    struct simple_common_reference<B&&, A&>
      : simple_common_reference<A&, B&&>
    { };

    template <std::size_t Case, typename T, typename U, typename Enable = void>
    struct common_reference_pair_impl
      : common_reference_pair_impl<Case + 1, T, U>
    { };

    // 1
    template <typename T, typename U>
    struct common_reference_pair_impl<1,
                                      T, U,
                                      std::void_t<std::enable_if_t<std::is_reference_v<T>
                                                               &&  std::is_reference_v<T>>,
                                                  simple_common_reference_t<T, U>>>
    {
      using type = simple_common_reference_t<T, U>;
    };

    // 2
    template <typename T, typename U>
    struct common_reference_pair_impl<2,
                                     T, U,
                                     std::void_t<resolved_basic_common_reference_t<T, U>>>
    {
      using type = resolved_basic_common_reference_t<T, U>;
    };

    // 3
    template <typename T, typename U>
    struct common_reference_pair_impl<3,
                                      T, U,
                                      std::void_t<common_reference_ternary_result_t<T, U>>>
    {
      using type = common_reference_ternary_result_t<T, U>;
    };

    // 4
    template <typename T, typename U>
    struct common_reference_pair_impl<4,
                                      T, U,
                                      std::void_t<std::common_type_t<T, U>>>
    {
      using type = std::void_t<std::common_type_t<T, U>>;
    };

    // 5
    template <typename T, typename U>
    struct common_reference_pair_impl<5, T, U, void>
    { };

    template <typename T, typename U>
    struct common_reference_pair
      : common_reference_pair_impl<1, T, U>
    { };

    template <typename T, typename U>
    using common_reference_pair_t = typename common_reference_pair<T, U>::type;

    template <typename Void, typename ...Ts>
    struct common_reference_impl
    { };

    template <>
    struct common_reference_impl<void>
    { };

    template <typename T>
    struct common_reference_impl<void, T>
    {
      using type = T;
    };

    template <typename T, typename U>
    struct common_reference_impl<void, T, U>
      : common_reference_pair<T, U>
    { };

    template <typename T, typename U, typename ...Rest>
    struct common_reference_impl<std::void_t<common_reference_pair_t<T, U>>, T, U, Rest...>
      : common_reference_impl<void, common_reference_pair_t<T, U>, Rest...>
    { };

    template <typename Void, typename ...Ts>
    struct has_common_reference_impl
      : std::false_type
    { };

    template <typename ...Ts>
    struct has_common_reference_impl<std::void_t<common_reference_t<Ts...>>, Ts...>
      : std::true_type
    { };

  }

  template <typename ...Ts>
  struct common_reference
    : detail::common_reference_impl<void, Ts...>
  { };

  template <typename ...Ts>
  struct has_common_reference
    : detail::has_common_reference_impl<void, Ts...>
  { };

  template <typename ...Ts>
  inline constexpr
  bool
  has_common_reference_v = has_common_reference<Ts...>::value;

  template <typename ...>
  inline constexpr
  bool
  dependent_false = false;

  template <typename T>
  using is_pointer_ref = std::is_pointer<typename std::remove_reference<T>::type>;

  namespace detail
  {

    template <typename T>
    struct is_reference_wrapper_impl
      : std::false_type
    { };

    template <typename U>
    struct is_reference_wrapper_impl<std::reference_wrapper<U>>
      : std::true_type
    { };

  } // namespace detail

  template <typename T>
  struct is_reference_wrapper
    : detail::is_reference_wrapper_impl<typename std::remove_cv<T>::type>
  { };

  template <typename T>
  inline constexpr
  bool
  is_reference_wrapper_v = is_reference_wrapper<T>::value;

  //
  // TESTS
  //

  static_assert (0 == pack_index_v<int, type_pack<int>>);
  static_assert (0 == pack_index_v<int, type_pack<int, long>>);
  static_assert (1 == pack_index_v<long, type_pack<int, long>>);

  static_assert (! pack_contains_v<int, type_pack<>>);
  static_assert (  pack_contains_v<int, type_pack<int>>);
  static_assert (  pack_contains_v<int, type_pack<long, int>>);
  static_assert (  pack_contains_v<int, type_pack<int, long>>);
  static_assert (! pack_contains_v<int, type_pack<long>>);

  static_assert (std::is_same_v<type_pack<int, long>,
                                pack_concatenate_t<type_pack<int, long>>>);

  static_assert (std::is_same_v<type_pack<int, long, char, short>,
                                pack_concatenate_t<type_pack<int, long>,
                                                          type_pack<char, short>>>);

  static_assert (std::is_same_v<type_pack<int, long, char, short, bool, int>,
                                pack_concatenate_t<type_pack<int, long>,
                                                          type_pack<char, short>,
                                                          type_pack<bool, int>>>);

  static_assert (std::is_same_v<type_pack<int, long, void>,
                                pack_flatten_t<type_pack<int, type_pack<long, void>>>>);

  static_assert (std::is_same_v<type_pack<int, long, char, short>,
                                pack_flatten_t<type_pack<type_pack<int, long>, char, short>>>);

  static_assert (std::is_same_v<type_pack<int, long, char, short>,
                                pack_flatten_t<
                                  type_pack<type_pack<int, type_pack<long>, type_pack<>>,
                                             type_pack<type_pack<type_pack<char>>>,
                                             short>>>);

  static_assert (std::is_same_v<type_pack<int>, pack_unique_t<type_pack<int>>>);
  static_assert (std::is_same_v<type_pack<int>, pack_unique_t<type_pack<int, int>>>);
  static_assert (pack_equivalent_v<type_pack<int, long>, pack_unique_t<type_pack<int, long, int>>>);

  static_assert (  pack_equivalent_v<type_pack<>, type_pack<>>);
  static_assert (! pack_equivalent_v<type_pack<>, type_pack<int>>);
  static_assert (! pack_equivalent_v<type_pack<int>, type_pack<>>);
  static_assert (  pack_equivalent_v<type_pack<int>, type_pack<int>>);
  static_assert (! pack_equivalent_v<type_pack<int>, type_pack<long>>);
  static_assert (  pack_equivalent_v<type_pack<int, long>, type_pack<int, long>>);
  static_assert (  pack_equivalent_v<type_pack<long, int>, type_pack<int, long>>);
  static_assert (! pack_equivalent_v<type_pack<int, char>, type_pack<int, long>>);
  static_assert (! pack_equivalent_v<type_pack<char, int>, type_pack<int, long>>);
  static_assert (! pack_equivalent_v<type_pack<int, long, char>, type_pack<int, long>>);

  static_assert (std::is_same_v<match_cvref_t<               int, long>,                long>);
  static_assert (std::is_same_v<match_cvref_t<const          int, long>, const          long>);
  static_assert (std::is_same_v<match_cvref_t<      volatile int, long>,       volatile long>);
  static_assert (std::is_same_v<match_cvref_t<const volatile int, long>, const volatile long>);

  static_assert (std::is_same_v<match_cvref_t<               int&, long>,                long&>);
  static_assert (std::is_same_v<match_cvref_t<const          int&, long>, const          long&>);
  static_assert (std::is_same_v<match_cvref_t<      volatile int&, long>,       volatile long&>);
  static_assert (std::is_same_v<match_cvref_t<const volatile int&, long>, const volatile long&>);

  static_assert (std::is_same_v<match_cvref_t<               int&&, long>,                long&&>);
  static_assert (std::is_same_v<match_cvref_t<const          int&&, long>, const          long&&>);
  static_assert (std::is_same_v<match_cvref_t<      volatile int&&, long>,       volatile long&&>);
  static_assert (std::is_same_v<match_cvref_t<const volatile int&&, long>, const volatile long&&>);

}

#endif // OCTAVE_IR_IR_TYPE_TRAITS_HPP

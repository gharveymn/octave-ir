/** ir-type-traits.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_UTILITIES_IR_TYPE_TRAITS_HPP
#define OCTAVE_IR_UTILITIES_IR_TYPE_TRAITS_HPP

#include "ir-type-pack.hpp"
#include "ir-function-traits.hpp"

#include <iterator>
#include <type_traits>
#include <utility>

namespace gch
{

  template <typename T>
  struct type_identity
  {
    using type = T;
  };

  template <typename T>
  using type_identity_t = typename type_identity<T>::type;

  template <typename ...Ts>
  struct all_same;

  template <typename ...Ts>
  inline constexpr
  bool
  all_same_v = all_same<Ts...>::value;

  template <typename T>
  struct is_complete;

  template <typename T>
  inline constexpr
  bool
  is_complete_v = is_complete<T>::value;

  template <typename It>
  struct is_iterator;

  template <typename It>
  inline constexpr
  bool
  is_iterator_v = is_iterator<It>::value;

  template <typename T>
  struct remove_all_pointers;

  template <typename T>
  using remove_all_pointers_t = typename remove_all_pointers<T>::type;

  template <typename T>
  struct remove_all_const;

  template <typename T>
  using remove_all_const_t = typename remove_all_const<T>::type;

  template <typename T>
  struct remove_all_volatile;

  template <typename T>
  using remove_all_volatile_t = typename remove_all_volatile<T>::type;

  template <typename T>
  struct remove_all_cv;

  template <typename T>
  using remove_all_cv_t = typename remove_all_cv<T>::type;

  template <typename T>
  struct make_all_levels_const;

  template <typename T>
  using make_all_levels_const_t = typename make_all_levels_const<T>::type;

  namespace detail
  {

    template <typename, typename Enable = void>
    struct is_complete_impl
      : std::false_type
    { };

    template <typename U>
    struct is_complete_impl<U, decltype (static_cast<void> (sizeof (U)))>
      : std::true_type
    { };

    template <typename It, typename = void>
    struct is_iterator_impl
      : std::false_type
    { };

    template <typename It>
    struct is_iterator_impl<It, std::void_t<typename std::iterator_traits<It>::value_type>>
      : std::true_type
    { };

    template <typename T>
    struct remove_all_pointers_impl
    {
      using type = T;
    };

    template <typename T>
    struct remove_all_pointers_impl<T *>
      : remove_all_pointers_impl<T>
    { };

    template <typename T>
    struct remove_all_const_impl
    {
      using type = T;
    };

    template <typename T>
    struct remove_all_const_impl<const T>
      : remove_all_const_impl<T>
    { };

    template <typename T>
    struct remove_all_const_impl<T *>
    {
      using type = remove_all_const_t<T> *;
    };

    template <typename T>
    struct remove_all_const_impl<T&>
    {
      using type = remove_all_const_t<T>&;
    };

    template <typename T>
    struct remove_all_const_impl<T&&>
    {
      using type = remove_all_const_t<T>&&;
    };

    template <typename T>
    struct remove_all_const_impl<T[]>
    {
      using type = remove_all_const_t<T>[];
    };

    template <typename T>
    struct remove_all_const_impl<const T[]>
    {
      using type = remove_all_const_t<T>[];
    };

    template <typename T, std::size_t N>
    struct remove_all_const_impl<T[N]>
    {
      using type = remove_all_const_t<T>[N];
    };

    template <typename T, std::size_t N>
    struct remove_all_const_impl<const T[N]>
    {
      using type = remove_all_const_t<T>[N];
    };

    template <typename T>
    struct remove_all_volatile_impl
    {
      using type = T;
    };

    template <typename T>
    struct remove_all_volatile_impl<volatile T>
      : remove_all_volatile_impl<T>
    { };

    template <typename T>
    struct remove_all_volatile_impl<T *>
    {
      using type = remove_all_volatile_t<T> *;
    };

    template <typename T>
    struct remove_all_volatile_impl<T&>
    {
      using type = remove_all_volatile_t<T>&;
    };

    template <typename T>
    struct remove_all_volatile_impl<T&&>
    {
      using type = remove_all_volatile_t<T>&&;
    };

    template <typename T>
    struct remove_all_volatile_impl<T[]>
    {
      using type = remove_all_volatile_t<T>[];
    };

    template <typename T>
    struct remove_all_volatile_impl<volatile T[]>
    {
      using type = remove_all_volatile_t<T>[];
    };

    template <typename T, std::size_t N>
    struct remove_all_volatile_impl<T[N]>
    {
      using type = remove_all_volatile_t<T>[N];
    };

    template <typename T, std::size_t N>
    struct remove_all_volatile_impl<volatile T[N]>
    {
      using type = remove_all_volatile_t<T>[N];
    };

    template <typename T>
    struct remove_all_cv_impl
    {
      using type = T;
    };

    template <typename T>
    struct remove_all_cv_impl<const T>
      : remove_all_cv_impl<T>
    { };

    template <typename T>
    struct remove_all_cv_impl<volatile T>
      : remove_all_cv_impl<T>
    { };

    template <typename T>
    struct remove_all_cv_impl<const volatile T>
      : remove_all_cv_impl<T>
    { };

    template <typename T>
    struct remove_all_cv_impl<T *>
    {
      using type = remove_all_cv_t<T> *;
    };

    template <typename T>
    struct remove_all_cv_impl<T&>
    {
      using type = remove_all_cv_t<T>&;
    };

    template <typename T>
    struct remove_all_cv_impl<T&&>
    {
      using type = remove_all_cv_t<T>&&;
    };

    template <typename T>
    struct remove_all_cv_impl<T[]>
    {
      using type = remove_all_cv_t<T>[];
    };

    template <typename T>
    struct remove_all_cv_impl<const T[]>
    {
      using type = remove_all_cv_t<T>[];
    };

    template <typename T>
    struct remove_all_cv_impl<volatile T[]>
    {
      using type = remove_all_cv_t<T>[];
    };

    template <typename T>
    struct remove_all_cv_impl<const volatile T[]>
    {
      using type = remove_all_cv_t<T>[];
    };

    template <typename T, std::size_t N>
    struct remove_all_cv_impl<T[N]>
    {
      using type = remove_all_cv_t<T>[N];
    };

    template <typename T, std::size_t N>
    struct remove_all_cv_impl<const T[N]>
    {
      using type = remove_all_cv_t<T>[N];
    };

    template <typename T, std::size_t N>
    struct remove_all_cv_impl<volatile T[N]>
    {
      using type = remove_all_cv_t<T>[N];
    };

    template <typename T, std::size_t N>
    struct remove_all_cv_impl<const volatile T[N]>
    {
      using type = remove_all_cv_t<T>[N];
    };

    template <typename T>
    struct make_all_levels_const_impl
    {
      using type = const T;
    };

    template <typename T>
    struct make_all_levels_const_impl<const T>
    {
      using type = const make_all_levels_const_t<T>;
    };

    template <typename T>
    struct make_all_levels_const_impl<T *>
    {
      using type = make_all_levels_const_t<T> * const;
    };

    template <typename T>
    struct make_all_levels_const_impl<T&>
    {
      using type = make_all_levels_const_t<T>&;
    };

    template <typename T>
    struct make_all_levels_const_impl<T&&>
    {
      using type = make_all_levels_const_t<T>&&;
    };

  } // namespace gch::detail

  template <typename ...Ts>
  struct all_same
    : pack_homogenenous<type_pack<Ts...>>
  { };

  template <typename T>
  struct is_complete
    : detail::is_complete_impl<T>
  { };

  template <typename It>
  struct is_iterator
    : detail::is_iterator_impl<It>
  { };

  template <typename T>
  struct remove_all_pointers
    : detail::remove_all_pointers_impl<T>
  { };

  template <typename T>
  struct remove_all_const
    : detail::remove_all_const_impl<T>
  { };

  template <typename T>
  struct remove_all_volatile
    : detail::remove_all_volatile_impl<T>
  { };

  template <typename T>
  struct remove_all_cv
    : detail::remove_all_cv_impl<T>
  { };

  template <typename T>
  struct make_all_levels_const
    : detail::make_all_levels_const_impl<T>
  { };

  template <typename From, typename To>
  struct match_cv
    : std::conditional<std::is_const_v<From>,
                       std::conditional_t<std::is_volatile_v<From>,
                                          std::add_cv_t<To>,
                                          std::add_const_t<To>>,
                       std::conditional_t<std::is_volatile_v<From>,
                                          std::add_volatile_t<To>,
                                          To>>
  { };

  template <typename From, typename To>
  using match_cv_t = typename match_cv<From, To>::type;

  template <typename From, typename To>
  struct match_ref
    : std::conditional<std::is_lvalue_reference_v<From>,
                       std::add_lvalue_reference_t<To>,
                       std::conditional_t<std::is_rvalue_reference_v<From>,
                                          std::add_rvalue_reference_t<To>,
                                          To>>
  { };

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

  template <auto Constant>
  using make_integral_constant = std::integral_constant<decltype (Constant), Constant>;

  namespace detail
  {

    template <typename IntegerSequence, typename Wrapped>
    struct wrap_integer_sequence_impl
    { };

    template <typename T, T ...Ints, typename Wrapped>
    struct wrap_integer_sequence_impl<std::integer_sequence<T, Ints...>, Wrapped>
    {
      using type = type_pack<std::integral_constant<Wrapped, static_cast<Wrapped> (Ints)>...>;
    };

  }

  template <typename IntegerSequence, typename Wrapped = typename IntegerSequence::value_type>
  struct wrap_integer_sequence
    : detail::wrap_integer_sequence_impl<IntegerSequence, Wrapped>
  { };

  template <typename IntegerSequence, typename Wrapped = typename IntegerSequence::value_type>
  using wrap_integer_sequence_t = typename wrap_integer_sequence<IntegerSequence, Wrapped>::type;

}

#endif // OCTAVE_IR_UTILITIES_IR_TYPE_TRAITS_HPP

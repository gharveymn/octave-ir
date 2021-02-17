/*

Copyright (C) 2019 Gene Harvey

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

//! this header forward-declares relevant classes and declares some typedefs
#if ! defined (octave_ir_common_h)
#define octave_ir_common_h 1

#include <exception>
#include <string>

#define GCH_PRINT_SIZE(TYPE) \
char (*__gch__fail) (void)[sizeof(TYPE)] = 1;

#define GCH_STRCAT_(x, y) x ## y
#define GCH_STRCAT(x, y) GCH_STRCAT_(x, y)
#define GCH_PRINT_VALUE(x) \
template <int> struct GCH_STRCAT(GCH_STRCAT(value_of_, x), _is); \
static_assert(GCH_STRCAT(GCH_STRCAT(value_of_, x), _is)<x>::x, " ");

#ifndef GCH_ACCUMULATE_REF
#  if __cplusplus > 201703L
#    define GCH_ACCUMULATE_LHS(TYPE) TYPE&&
#  else
#    define GCH_ACCUMULATE_LHS(TYPE) TYPE&
#  endif
#endif

class octave_base_value;
namespace gch
{
  inline constexpr
  bool
  OCTAVE_IR_DEBUG
#ifdef NDEBUG
    = false;
#else
    = true;
#endif

  using any = octave_base_value *;
  using single = float;

  namespace detail
  {

    template <typename T, typename ...Ts>
    struct all_same : std::conjunction<std::is_same<T, Ts>...>
    { };

    template <std::size_t I, typename T, typename ...Ts>
    struct pack_select
      : pack_select<I - 1, Ts...>
    { };

    template <typename T, typename ...Ts>
    struct pack_select<0, T, Ts...>
    {
      using type = T;
    };

    template <std::size_t I, typename T, typename Head, typename ...Tail>
    struct pack_index : detail::pack_index<I + 1, T, Tail...>
    { };

    template <std::size_t I, typename T, typename ...Rest>
    struct pack_index<I, T, T, Rest...>
      : std::integral_constant<std::size_t, I>
    { };

    template <std::size_t I, typename T>
    struct pack_index<I, T, T>
      : std::integral_constant<std::size_t, I>
    { };

    template <std::size_t I, typename T, typename U>
    struct pack_index<I, T, U>
    { };

    template <typename T, typename = void, typename ...Ts>
    struct pack_contains : std::false_type
    { };

    template <typename T, typename ...Ts>
    struct pack_contains<T, std::void_t<typename pack_index<0, T, Ts...>::type>, Ts...>
      : std::true_type
    { };

    template <typename ...Packs>
    struct pack_concatenate;

    template <typename Pack>
    struct pack_concatenate<Pack>
    {
      using type = Pack;
    };

    template <template <typename ...> typename PackT,
              typename ...LHS, typename ...RHS, typename ...Rest>
    struct pack_concatenate<PackT<LHS...>, PackT<RHS...>, Rest...>
      : pack_concatenate<PackT<LHS..., RHS...>, Rest...>
    { };

    template <typename It, typename = void>
    struct is_iterator : std::false_type
    { };

    template <typename It>
    struct is_iterator<It, std::void_t<std::iterator_traits<It>>>
      : std::true_type
    { };
  }

  template <typename ...Ts>
  struct all_same
    : detail::all_same<Ts...>
  { };

  template <std::size_t I, typename ...Ts>
  struct pack_select_type
    : detail::pack_select<I, Ts...>
  { };

  template <typename T, typename ...Ts>
  struct pack_index
    : detail::pack_index<0, T, Ts...>
  { };


  template <typename T, typename ...Ts>
  struct pack_contains
    : detail::pack_contains<T, void, Ts...>
  { };

  template <typename ...Packs>
  struct pack_concatenate
    : detail::pack_concatenate<Packs...>
  { };

  template <typename ...Packs>
  using pack_concatenate_t = typename pack_concatenate<Packs...>::type;

  static_assert (std::is_same_v<std::tuple<int, long>,
                                pack_concatenate_t<std::tuple<int, long>>>);

  static_assert (std::is_same_v<std::tuple<int, long, char, short>,
                                pack_concatenate_t<std::tuple<int, long>,
                                                          std::tuple<char, short>>>);

  static_assert (std::is_same_v<std::tuple<int, long, char, short, bool, int>,
                                pack_concatenate_t<std::tuple<int, long>,
                                                          std::tuple<char, short>,
                                                          std::tuple<bool, int>>>);

  template <typename It>
  struct is_iterator
    : detail::is_iterator<It>
  { };

  template <std::size_t I, typename ...Ts>
  using pack_select_t = typename pack_select_type<I, Ts...>::type;

  template <typename ...Ts>
  inline constexpr
  bool
  all_same_v = all_same<Ts...>::value;

  template <typename T, typename ...Ts>
  inline constexpr
  std::size_t
  pack_index_v = pack_index<T, Ts...>::value;

  template <typename T, typename ...Ts>
  inline constexpr
  std::size_t
  pack_contains_v = pack_contains<T, Ts...>::value;

  template <typename It>
  inline constexpr
  bool
  is_iterator_v = is_iterator<It>::value;

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

#endif

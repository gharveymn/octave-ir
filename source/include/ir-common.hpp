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

#define PRINT_SIZE(TYPE) \
char (*__fail)(void)[sizeof(TYPE)] = 1;

#define strcat_(x, y) x ## y
#define strcat(x, y) strcat_(x, y)
#define PRINT_VALUE(x) \
template <int> struct strcat(strcat(value_of_, x), _is); \
static_assert(strcat(strcat(value_of_, x), _is)<x>::x, "");

class octave_base_value;
namespace gch
{
#ifdef NDEBUG
  inline constexpr bool OCTAVE_IR_DEBUG = false;
#else
  inline constexpr bool OCTAVE_IR_DEBUG = true;
#endif

  using any = octave_base_value *;
  using single = float;

  class ir_exception : public std::exception
  {
  public:
    explicit ir_exception (const char *str)
    : m_str (str)
    { }

    explicit ir_exception (std::string str)
    : m_str (std::move (str))
    { }

    const char* what (void) const noexcept override
    {
      return m_str.c_str ();
    }

  private:
    std::string m_str;
  };

//  template <typename T, typename = void>
//  struct is_char : std::false_type
//  { };
//
//  template <typename T>
//  struct is_char<
//    T,
//    std::enable_if<
//      disjunction<
//        std::is_same<char, typename std::remove_cv<T>::type>,
//        std::is_same<wchar_t, typename std::remove_cv<T>::type>,
//        std::is_same<char16_t, typename std::remove_cv<T>::type>,
//        std::is_same<char32_t, typename std::remove_cv<T>::type>
//      >::value
//    >
//  > : std::true_type
//  { };
//
//#if __cpp_char8_t >= 201811L
//  template <typename T>
//  struct is_char<
//           T,
//           typename std::enable_if<
//               std::is_same<char8_t, typename std::remove_cv<T>::type>>::value
//           >::type
//         > : std::true_type
//  { };
//  static_assert (is_char<const volatile char8_t>::value, "is_char not working");
//  static_assert (is_char<char8_t>::value, "is_char not working");
//#endif
//
//  template <typename T, typename = void>
//  struct is_string : std::false_type
//  { };
//
//  // check if std::basic_string
//  template <template <typename ...> class S, typename ...Ts>
//  struct is_string<
//    S<Ts...>,
//    std::enable_if_t<
//      std::is_same<std::basic_string<Ts...>, S<Ts...>>::value
//    >
//  > : std::true_type
//  { };
//
//  // check if const std::basic_string
//  template <template <typename ...> class S, typename ...Ts>
//  struct is_string<const S<Ts...>> : is_string<S<Ts...>>
//  { };
//
//  // check if c-string
//  template <typename T>
//  struct is_string<
//    T *,
//    std::enable_if_t<is_char<T>::value>
//  > : std::true_type
//  { };
//
//  // check if array c-string
//  template <typename T>
//  struct is_string<
//    T[],
//    std::enable_if_t<is_char<T>::value>
//  > : std::true_type
//  { };
//
//  // map the type to another type
//  // useful for deferring resolution
//  template <typename T, typename B>
//  struct map_to : B
//  { };
//
//  template <typename T>
//  struct map_to_true : std::true_type
//  { };
//
//  template <typename T>
//  struct map_to_false : std::false_type
//  { };

//  template <typename T> using iter   = typename T::iterator;
//  template <typename T> using citer  = typename T::const_iterator;
//  template <typename T> using riter  = typename T::reverse_iterator;
//  template <typename T> using criter = typename T::const_reverse_iterator;
//  template <typename T> using ref    = typename T::reference;
//  template <typename T> using cref   = typename T::const_reference;

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
    struct is_element : std::false_type
    { };

    template <typename T, typename ...Ts>
    struct is_element<T, std::void_t<typename pack_index<0, T, Ts...>::type>, Ts...>
      : std::true_type
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
  using all_same    = detail::all_same<Ts...>;

  template <std::size_t I, typename ...Ts>
  using pack_select      = detail::pack_select<I, Ts...>;

  template <std::size_t I, typename ...Ts>
  using pack_select_t    = typename pack_select<I, Ts...>::type;

  template <typename T, typename ...Ts>
  using pack_index       = detail::pack_index<0, T, Ts...>;

  template <typename T, typename ...Ts>
  using is_element  = detail::is_element<T, void, Ts...>;

  template <typename It>
  using is_iterator = detail::is_iterator<It>;

  template <typename ...Ts>
  inline constexpr bool        all_same_v    = all_same<Ts...>::value;

  template <typename T, typename ...Ts>
  inline constexpr std::size_t pack_index_v       = pack_index<T, Ts...>::value;

  template <typename T, typename ...Ts>
  inline constexpr std::size_t is_element_v  = is_element<T, Ts...>::value;

  template <typename It>
  inline constexpr bool        is_iterator_v = is_iterator<It>::value;

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

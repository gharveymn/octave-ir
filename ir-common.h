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

#include "octave-config.h"

#include <cpp14/type_traits>
#include <exception>
#include <string>

class octave_base_value;
namespace octave
{
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

  // C++17 needed types

//  template<bool B, typename T = void>
//  using enable_if_t = typename std::enable_if<B, T>::type;
//
//  template<class T>
//  using remove_pointer_t = typename std::remove_pointer<T>::type;

  // imitates 'void' as a unit type (added as std::monostate in C++17)
  struct monostate { };

  template<bool v>
  using bool_constant = std::integral_constant<bool, v>;

  template<class B>
  struct negation : bool_constant<! bool (B::value)>
  { };

  template <typename...>
  struct conjunction : std::true_type
  { };

  template <typename T>
  struct conjunction<T> : T
  { };

  template<typename T, typename ...Ts>
  struct conjunction<T, Ts...> : std::conditional<T::value,
                                                  conjunction<Ts...>, T>::type
  { };

  template <typename...>
  struct disjunction : std::false_type
  { };

  template <typename T>
  struct disjunction<T> : T
  { };

  template<typename T, typename... Ts>
  struct disjunction<T, Ts...> : std::conditional<T::value, T, 
                                                  disjunction<Ts...>>::type
  { };
  
  // aliases
  
//  template <typename ...Ts>
//  using conj = conjunction<Ts...>;
    
//  template <typename ...Ts>
//  using disj = conjunction<Ts...>;

  template <typename T, typename = void>
  struct is_char : std::false_type
  { };

  template <typename T>
  struct is_char<
    T,
    cpp14::enable_if_t<
      disjunction<
        std::is_same<char, typename std::remove_cv<T>::type>,
        std::is_same<wchar_t, typename std::remove_cv<T>::type>,
        std::is_same<char16_t, typename std::remove_cv<T>::type>,
        std::is_same<char32_t, typename std::remove_cv<T>::type>
      >::value
    >
  > : std::true_type
  { };

#if __cpp_char8_t >= 201811L
  template <typename T>
  struct is_char<
           T,
           typename std::enable_if<
               std::is_same<char8_t, typename std::remove_cv<T>::type>>::value
           >::type
         > : std::true_type
  { };
  static_assert (is_char<const volatile char8_t>::value, "is_char not working");
  static_assert (is_char<char8_t>::value, "is_char not working");
#endif

  template <typename T, typename = void>
  struct is_string : std::false_type
  { };

  // check if std::basic_string
  template <template <typename ...> class S, typename ...Ts>
  struct is_string<
    S<Ts...>,
    cpp14::enable_if_t<
      std::is_same<std::basic_string<Ts...>, S<Ts...>>::value
    >
  > : std::true_type
  { };

  // check if const std::basic_string
  template <template <typename ...> class S, typename ...Ts>
  struct is_string<const S<Ts...>> : is_string<S<Ts...>>
  { };

  // check if c-string
  template <typename T>
  struct is_string<
    T *,
    cpp14::enable_if_t<is_char<T>::value>
  > : std::true_type
  { };

  // check if array c-string
  template <typename T>
  struct is_string<
    T[],
    cpp14::enable_if_t<is_char<T>::value>
  > : std::true_type
  { };

  // map the type to another type
  // useful for deferring resolution
  template <typename T, typename B>
  struct map_to : B
  { };

  template <typename T>
  struct map_to_true : std::true_type
  { };

  template <typename T>
  struct map_to_false : std::false_type
  { };

//  template <typename T> using iter   = typename T::iterator;
//  template <typename T> using citer  = typename T::const_iterator;
//  template <typename T> using riter  = typename T::reverse_iterator;
//  template <typename T> using criter = typename T::const_reverse_iterator;
//  template <typename T> using ref    = typename T::reference;
//  template <typename T> using cref   = typename T::const_reference;

}

#endif

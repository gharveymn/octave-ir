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

#include <exception>
#include <string>
#include <memory>
#include <list>

class octave_base_value;
namespace octave
{
  using any = octave_base_value *;
  using single = float;

  class ir_type;
  class ir_operand;
  class ir_variable;
  class ir_instruction;
  class ir_basic_block;
  class ir_component;
  class ir_module;

  class ir_exception : public std::exception
  {
  public:

    explicit ir_exception (const char *str) noexcept
      : m_str (str)
    { }

//    template <int N>
//    explicit ir_exception (const char (&&str)[N]) noexcept
//      : m_str (str)
//    { }

    const char * what () const noexcept override
    {
      return m_str;
    }

  private:
    const char *m_str;
  };

  // C++17 needed types

  template<bool B, typename T = void>
  using enable_if_t = typename std::enable_if<B, T>::type;

  template<class T>
  using remove_pointer_t = typename std::remove_pointer<T>::type;

  // imitates 'void' as a unit type (added as std::monostate in C++17)
  struct monostate { };

  template <class...>
  struct conjunction : std::true_type
  { };

  template <class B>
  struct conjunction<B> : B
  { };

  template<class B, class... Bs>
  struct conjunction<B, Bs...> : std::conditional<B::value,
                                                  conjunction<Bs...>, B>::type
  { };

  template <class...>
  struct disjunction : std::false_type
  { };

  template <class B>
  struct disjunction<B> : B
  { };

  template<class B, class... Bs>
  struct disjunction<B, Bs...> : std::conditional<B::value,
                                                  B, disjunction<Bs...>>::type
  { };

  template <typename T, typename = void>
  struct is_char : std::false_type
  { };

  template <typename T>
  struct is_char<
    T,
    enable_if_t<
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
    enable_if_t<
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
    enable_if_t<is_char<T>::value>
  > : std::true_type
  { };

  // check if array c-string
  template <typename T>
  struct is_string<
    T[],
    enable_if_t<is_char<T>::value>
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

}

#endif

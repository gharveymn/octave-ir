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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "utilities/ir-common.hpp"

#include "utilities/ir-common-util.hpp"
#include "values/types/ir-type.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace gch
{
  template <typename T>
  std::ostream&
  ir_printer<T>::short_print (std::ostream&, const ir_class&)
  {
    throw ir_exception (std::string ("short_print not implemented for ")
                        + typeid (T).name () + ".");
  }

  template <typename T>
  std::ostream&
  ir_printer<T>::long_print (std::ostream&, const ir_class&)
  {
    throw ir_exception (std::string ("long_print not implemented for ")
                        + typeid (T).name () + ".");
  }

//  static_assert (is_char<const volatile char>::value, "is_char not working");
//  static_assert (is_char<char>::value, "is_char not working");
//  static_assert (is_char<wchar_t>::value, "is_char not working");
//  static_assert (is_char<char16_t>::value, "is_char not working");
//  static_assert (is_char<char32_t>::value, "is_char not working");
//  static_assert (! is_char<int>::value, "is_char not working");
//  static_assert (! is_char<char *>::value, "is_char not working");
//  static_assert (! is_char<char[]>::value, "is_char not working");
//
//  static_assert(is_string<const std::string>::value, "is_string not working");
//  static_assert(is_string<std::string>::value, "is_string not working");
//  static_assert(is_string<std::wstring>::value, "is_string not working");
//  static_assert(is_string<std::u32string>::value, "is_string not working");
//  static_assert(is_string<std::u16string>::value, "is_string not working");
//  static_assert(is_string<char *>::value, "is_string not working");
//  static_assert(is_string<const char *>::value, "is_string not working");
//  static_assert(is_string<const wchar_t *>::value, "is_string not working");
//  static_assert(is_string<const char32_t *>::value, "is_string not working");
//  static_assert(is_string<const char16_t *>::value, "is_string not working");
//  static_assert(is_string<char[]>::value, "is_string not working");
//  static_assert(is_string<const char[]>::value, "is_string not working");
//  static_assert(is_string<const wchar_t[]>::value, "is_string not working");
//  static_assert(is_string<const char32_t[]>::value, "is_string not working");
//  static_assert(is_string<const char16_t[]>::value, "is_string not working");
//  static_assert(! is_string<int>::value, "is_string not working");

}

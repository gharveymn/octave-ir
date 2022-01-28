/** ir-constant.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-constant.hpp"

#include "ir-type-util.hpp"

#include <ostream>
#include <iostream>

namespace gch
{

  template <typename T>
  inline constexpr
  bool
  is_wide_char_v = std::is_same_v<T, wchar_t>
               ||  std::is_same_v<T, char32_t>
               ||  std::is_same_v<T, char16_t>;

  // FIXME: We probably shouldn't just be casting wide character types to char.

  template <typename T>
  struct constant_printer
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_constant& c)
    {
      if constexpr (is_wide_char_v<T>)
        return out << static_cast<char> (as<T> (c));
      else
        return out << as<T> (c);
    }
  };

  template <typename T>
  struct constant_printer<T *>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_constant& c)
    {
      if constexpr (is_wide_char_v<T>)
      {
        auto ptr = as<T *> (c);
        while (T val = *ptr)
          out << static_cast<char> (val);
        return out;
      }
      else
        return out << as<T *> (c);
    }
  };

  template <>
  struct constant_printer<void>
  {
    static
    std::ostream&
    print (std::ostream&, const ir_constant&)
    {
      throw std::ios_base::failure { "Cannot print a constant of type `void`." };
    }
  };

  template <>
  struct constant_printer<ir_static_block_id>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_constant& c)
    {
      return out << "BLOCK" << static_cast<std::size_t> (as<ir_static_block_id> (c));
    }
  };

  template <typename T>
  struct printer_mapper
  {
    constexpr
    auto
    operator() (void) const noexcept
    {
      return constant_printer<T>::print;
    }
  };

  static constexpr
  auto
  ir_constant_printer_map = generate_ir_type_map<printer_mapper> ();

  std::ostream&
  operator<< (std::ostream& out, const ir_constant& c)
  {
    return std::invoke (ir_constant_printer_map[c.get_type ()], out, c);
  }

}

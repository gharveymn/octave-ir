/** ir-constant.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-constant.hpp"

#include "ir-type-util.hpp"

#include <ostream>

namespace gch
{

  template <typename T>
  struct constant_printer
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_constant& c)
    {
      return out << as<T> (c);
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
  struct constant_printer<wchar_t>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_constant& c)
    {
      return out << static_cast<char> (as<wchar_t> (c));
    }
  };

  template <>
  struct constant_printer<char32_t>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_constant& c)
    {
      return out << static_cast<char> (as<char32_t> (c));
    }
  };

  template <>
  struct constant_printer<char16_t>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_constant& c)
    {
      return out << static_cast<char> (as<char16_t> (c));
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

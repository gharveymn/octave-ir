/** ir-constant.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-ir-static-ir/ir-constant.hpp"

#include "gch/octave-ir-static-ir/ir-type-util.hpp"

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
      return out << cast<T> (c);
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
      return out << static_cast<char> (cast<wchar_t> (c));
    }
  };

  template <>
  struct constant_printer<char32_t>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_constant& c)
    {
      return out << static_cast<char> (cast<char32_t> (c));
    }
  };

  template <>
  struct constant_printer<char16_t>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_constant& c)
    {
      return out << static_cast<char> (cast<char16_t> (c));
    }
  };

  class ir_constant_printer_map
  {
  public:
    using print_function_type = std::ostream& (*) (std::ostream&, const ir_constant&);

  private:
    template <typename T>
    struct printer_mapper
    {
      constexpr
      print_function_type
      operator() (void) const noexcept
      {
        return constant_printer<T>::print;
      }
    };

  public:
//  ir_constant_printer_map            (void)                               = impl;
    ir_constant_printer_map            (const ir_constant_printer_map&)     = delete;
    ir_constant_printer_map            (ir_constant_printer_map&&) noexcept = delete;
    ir_constant_printer_map& operator= (const ir_constant_printer_map&)     = delete;
    ir_constant_printer_map& operator= (ir_constant_printer_map&&) noexcept = delete;
    ~ir_constant_printer_map           (void)                               = default;

    ir_constant_printer_map (void)
      : m_map (template_generate_ir_type_map<printer_mapper> ())
    { }

    print_function_type
    operator[] (ir_type ty) const noexcept
    {
      return m_map[ty];
    }

  private:
    ir_type_map<print_function_type> m_map;
  };

  std::ostream&
  operator<< (std::ostream& out, const ir_constant& c)
  {
    static ir_constant_printer_map map;
    return std::invoke (map[c.get_type ()], out, c);
  }

}

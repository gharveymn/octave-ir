/** ir-external-function-info.cpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-external-function-info.hpp"

#include <ostream>

namespace gch
{

  ir_external_function_info::
  ir_external_function_info (std::string_view name, variadic_type is_variadic)
    : m_name        (name),
      m_is_variadic (is_variadic)
  { }

  ir_external_function_info::
  ir_external_function_info (std::string_view name)
    : m_name (name)
  { }

  std::string_view
  ir_external_function_info::
  get_name (void) const noexcept
  {
    return m_name;
  }

  bool
  ir_external_function_info::
  is_variadic (void) const noexcept
  {
    return m_is_variadic;
  }

  std::ostream&
  operator<< (std::ostream& out, const ir_external_function_info& info)
  {
    out << info.get_name () << " (";
    if (info.is_variadic ())
      out << "...";
    return out << ")";
  }

}

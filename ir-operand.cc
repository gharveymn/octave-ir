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

#include "ir-operand.h"
#include "ir-common-util.h"
#include "ir-variable.h"
#include "ir-instruction.h"
#include <ostream>

namespace octave
{
  
  template <typename ...Ts>
  ir_constant<Ts...>::ir_constant (Ts... args)  noexcept
    : ir_operand (ir_type::get<value_type> ()),
      m_value {std::move (args)...}
  { }
  
  template <typename T>
  ir_constant<T>::ir_constant (value_type avalue) noexcept
  : ir_operand (ir_type::get<value_type> ()),
  m_value (std::move (avalue))
  { }

  template <>
  std::ostream&
  ir_printer<ir_operand>::short_print (std::ostream& os, const ir_operand& op)
  {
    op.print (os);
    if (isa<const ir_variable::def> (op))
      os << ": " << op.m_type;
    return os;
  }

  template <>
  std::ostream&
  ir_printer<ir_operand>::long_print (std::ostream& os, const ir_operand& op)
  {
    return op.print (os) << ": " << op.m_type;
  }

  template <typename T>
  std::ostream&
  ir_constant<T>::print (std::ostream& os) const
  {
    return os << m_value;
  }

  template <>
  std::ostream&
  ir_constant<bool>::print (std::ostream& os) const
  {
    return os << (m_value ? "true" : "false");
  }

  template <>
  std::ostream&
  ir_constant<std::string>::print (std::ostream& os) const
  {
    return os << "\"" << m_value << "\"";
  }

  template <>
  std::ostream&
  ir_constant<char *>::print (std::ostream& os) const
  {
    return os << "\"" << m_value << "\"";
  }


  template <>
  std::ostream&
  ir_constant<char>::print (std::ostream& os) const
  {
    return os << '\'' << m_value << '\'';
  }

}



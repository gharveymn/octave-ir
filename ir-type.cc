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
#include "ir-common-util.h"
#include "ir-type.h"

#include <iostream>
#include <string>

namespace octave
{
  constexpr ir_type::impl ir_type::instance<any>::m_impl;
  constexpr ir_type::impl ir_type::instance<void>::m_impl;

  /////////////////////////////////
  // pointer type instantiations //
  /////////////////////////////////

  template struct ir_type::instance<any *>;


  // prints a pointer like "double**"
  template <>
  std::ostream&
  ir_printer<ir_type>::short_print (std::ostream& os, const ir_type& ty)
  {
    ir_type::impl_p impl_ptr = ty.m_ptr;
    os << impl_ptr->get_base_name ();
    return os << std::string (impl_ptr->get_indirection_level (), '*');
  }

  template <>
  std::ostream&
  ir_printer<ir_type>::long_print (std::ostream& os, const ir_type& ty)
  {
    const ir_type_array& members = ty.m_ptr->get_members ();
    short_print (os, ty);
    if (members.get_numel () > 0)
      {
        short_print (os << " {", members[0]);
        for (int i = 1; i < members.get_numel (); i++)
          short_print (os << ", ", members[i]);
        short_print (os << "}", members[0]);
      }
    return os;
  }

  std::ostream& operator<< (std::ostream& os, const ir_type& ty)
  {
    return ir_printer<ir_type>::short_print (os, ty);
  }

}

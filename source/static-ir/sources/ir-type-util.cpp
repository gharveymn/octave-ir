/** ir-type-util.cpp.c
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-type-util.hpp"

namespace gch
{

  std::string
  get_name (ir_type ty)
  {
    return std::string (ty.get_name_base ()).append (" ").append (indirection_level (ty), '*');
  }

  std::ostream&
  operator<< (std::ostream& out, ir_type ty)
  {
    return out << get_name (ty);
  }

}

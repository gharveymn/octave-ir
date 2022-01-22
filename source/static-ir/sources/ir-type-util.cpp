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
    std::string ret (ty.get_name_base ());
    if (std::size_t lvl = indirection_level (ty))
      ret.append (" ").append (lvl, '*');
    return ret;
  }

  std::ostream&
  operator<< (std::ostream& out, ir_type ty)
  {
    return out << get_name (ty);
  }

}

/** ir-type-util.cpp.c
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-ir-static-ir/ir-type-util.hpp"

namespace gch
{

  std::ostream&
  operator<< (std::ostream& out, ir_type ty)
  {
    return out << ty.get_name_base () << std::string (indirection_level (ty), '*');
  }

}

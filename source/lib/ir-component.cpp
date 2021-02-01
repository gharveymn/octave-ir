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

#include "ir-component.hpp"
#include "ir-function.hpp"
#include "ir-structure.hpp"

namespace gch
{
  ir_component::
  ~ir_component (void) noexcept = default;

  ir_function&
  get_function (ir_component& c)
  {
    for (optional_ref curr { c }; curr.has_value (); curr = curr->maybe_get_parent ())
    {
      if (optional_ref opt_func { dynamic_cast<ir_function *> (curr.get_pointer ()) })
        return *opt_func;
    }
    throw ir_exception ("function not found");
  }
}

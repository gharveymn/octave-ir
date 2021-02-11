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
#include "ir-block.hpp"

namespace gch
{

  ir_component::
  ~ir_component (void) noexcept = default;

  ir_function&
  get_function (ir_component& c)
  {
    for (optional_ref curr { c }; curr; curr = curr->maybe_get_parent ())
    {
      if (optional_ref opt_func { maybe_cast<ir_function> (curr) })
        return *opt_func;
    }
    throw ir_exception ("function not found");
  }

  ir_block&
  get_entry_block (ir_component& c)
  {
    nonnull_ptr<ir_component> curr { c };
    while (optional_ref s { maybe_cast<ir_structure> (curr) })
      curr = *s->get_entry_ptr ();

    assert (is_a<ir_block> (curr));
    return static_cast<ir_block&> (*curr);
  }

}

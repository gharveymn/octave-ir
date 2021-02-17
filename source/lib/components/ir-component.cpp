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

#include "components/ir-component.hpp"
#include "components/ir-function.hpp"
#include "components/ir-structure.hpp"
#include "components/ir-block.hpp"

namespace gch
{

  ir_component::
  ~ir_component (void) noexcept = default;

  ir_function&
  get_function (ir_component& c)
  {
    // note: this is too simple to need to create a new visitor
    optional_ref func { maybe_cast<ir_function> (c) };
    for (nonnull_ptr<ir_component> curr { c };
         ! func.has_value ();
         func = maybe_cast<ir_function> (curr))
    {
      optional_ref sub { maybe_cast<ir_subcomponent> (c) };
      assert (sub && "Expected a subcomponent.");
      curr.emplace (sub->get_parent ());
    }
    return *func;
  }

  ir_block&
  get_entry_block (ir_component& c)
  {
    nonnull_ptr<ir_component> curr { c };
    while (optional_ref s { maybe_cast<ir_structure> (curr) })
      curr.emplace (*s->get_entry_ptr ());

    assert (is_a<ir_block> (*curr));
    return static_cast<ir_block&> (*curr);
  }

  ir_link_set<ir_block>
  copy_leaves (const ir_component& c)
  {
    // delegate if the component is an ir_structure
    if (optional_ref s { maybe_cast<ir_structure> (c) })
      return copy_leaves (*s);

    // else it should just be a block
    assert (is_a<ir_block> (c));
    return copy_leaves (static_cast<const ir_block&> (c));
  }

  ir_link_set<ir_block>
  copy_leaves (const ir_structure& s)
  {
    return s.get_leaves ();
  }

  ir_link_set<ir_block>
  copy_leaves (const ir_block& b)
  {
    return { nonnull_ptr { as_mutable (b) } };
  }

}

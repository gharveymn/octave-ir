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

#include "ir-function.hpp"

#include "ir-component.hpp"
#include "ir-block.hpp"

#include <gch/nonnull_ptr.hpp>

namespace gch
{
  ir_function::
  ir_function (void)
    : ir_structure (nullopt),
      m_body (create_component<ir_component_sequence> (ir_subcomponent_type<ir_block>))
  { }

  //
  // virtual from ir_component
  //

  bool
  ir_function::
  reassociate_timelines (const ir_link_set<ir_def_timeline>& old_dts, ir_def_timeline& new_dt,
                         std::vector<nonnull_ptr<ir_block>>& until)
  {
    get_body_component ().reassociate_timelines (old_dts, new_dt, until);
    return true;
  }

  void
  ir_function::
  reset (void) noexcept
  {
    // get_body_component ().reset ();
    // get_body_component ().emplace_back<ir_block> ();
  }

  //
  // virtual from ir_structure
  //

  ir_component_ptr
  ir_function::
  get_ptr (ir_component&) const noexcept
  {
    return as_mutable (*this).get_body ();
  }

  ir_component_ptr
  ir_function::
  get_entry_ptr (void) noexcept
  {
    return as_mutable (*this).get_body ();
  }

  ir_link_set<ir_block>
  ir_function::
  get_predecessors (ir_component_cptr comp) noexcept
  {
    return { };
  }

  ir_link_set<ir_block>
  ir_function::
  get_successors (ir_component_cptr comp) noexcept
  {
    return { };
  }

  [[nodiscard]]
  bool
  ir_function::
  is_leaf (ir_component_cptr) noexcept
  {
    return true;
  }

  void
  ir_function::
  generate_leaf_cache (void)
  {
    leaves_append (get_body ());
  }

  ir_use_timeline&
  ir_function::
  join_incoming_at (ir_component_ptr pos, ir_def_timeline& dt)
  {
    assert (is_body (pos) && "pos is not the body component");

    if (dt.has_incoming_timeline ())
      throw ir_exception ("def timeline already holds an incoming timeline");
    return dt.create_incoming_timeline ();
  }

  void
  ir_function::
  recursive_flatten (void)
  {
    get_body_component ().recursive_flatten ();
  }

}

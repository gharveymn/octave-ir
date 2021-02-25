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

#include "components/ir-function.hpp"

#include "components/ir-component.hpp"
#include "components/ir-block.hpp"

#include "utilities/ir-error.hpp"

#include <gch/nonnull_ptr.hpp>

namespace gch
{
  ir_function::
  ir_function (void)
    : m_body (allocate_subcomponent<ir_component_sequence> (ir_subcomponent_type<ir_block>))
  { }

  //
  // virtual from ir_component
  //

  bool
  ir_function::
  reassociate_timelines (const ir_link_set<ir_def_timeline>& old_dts, ir_def_timeline& new_dt,
                         std::vector<nonnull_ptr<ir_block>>& until)
  {
    ir_component_sequence& seq = static_cast<ir_component_sequence&> (get_body ());
    seq.reassociate_timelines (old_dts, new_dt, until);
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

  ir_use_timeline&
  ir_function::
  join_incoming_at (ir_component_ptr pos, ir_def_timeline& dt)
  {
    assert (is_body (*pos) && "pos is not the body component");

    if (dt.has_incoming_timeline ())
      throw ir_exception ("def timeline already holds an incoming timeline");
    return dt.create_incoming_timeline ();
  }

}

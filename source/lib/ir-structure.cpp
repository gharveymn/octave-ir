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

#include "ir-structure.hpp"
#include "ir-component.hpp"
#include "ir-block.hpp"

namespace gch
{

  ir_structure::
  ~ir_structure (void) = default;

  auto
  ir_structure::
  get_leaves (void)
    -> const link_vector&
  {
    if (leaf_cache_empty ())
      generate_leaf_cache ();
    return get_leaf_cache ();
  }

  void
  ir_structure::
  leaves_append (const ir_component_handle& comp)
  {
    if (optional_ref s { maybe_cast_to<ir_structure> (comp) })
      m_leaf_cache.insert (m_leaf_cache.end (), s->leaves_begin (), s->leaves_end ());
    else
      m_leaf_cache.emplace_back (cast_to<ir_block> (comp));
  }

  void
  ir_structure::
  invalidate_leaf_cache (void) noexcept
  {
    clear_leaf_cache ();
    if (get_parent ().is_leaf_component (*this))
      get_parent ().invalidate_leaf_cache ();
  }

  ir_use_timeline&
  ir_structure::
  join_incoming_at (ir_block& block, ir_variable& var)
  {
    return join_incoming_at (get_handle (block), block.get_def_timeline (var));
  }

  //
  // non-member functions
  //

  ir_block&
  get_entry_block (ir_structure& s)
  {
    nonnull_ptr curr_entry { s.get_entry_component () };
    while (optional_ref opt_struct { maybe_cast_to<ir_structure> (*curr_entry) })
      curr_entry.emplace (opt_struct->get_entry_component ());
    return cast_to<ir_block> (*curr_entry);
  }

  [[nodiscard]]
  ir_structure::link_vector
  copy_leaves (const ir_component_handle& comp)
  {
    // if the component is an ir_block return a singleton vector
    // containing just that component
    if (optional_ref opt_block { maybe_cast_to<ir_block> (comp) })
      return { *opt_block };

    // otherwise delegate to the ir_structure
    return cast_to<ir_structure> (comp).get_leaves ();
  }

}

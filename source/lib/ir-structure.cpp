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
#include "ir-structure.hpp"
#include "ir-component-sequence.hpp"
#include "ir-block.hpp"

#include <numeric>

namespace gch
{



  ir_structure::
  ~ir_structure (void) = default;

  auto
  ir_structure::
  get_leaves (void)
    -> const link_vector&
  {
    if (m_leaf_cache.empty ())
      generate_leaf_cache ();
    return m_leaf_cache;
  }

  void
  ir_structure::
  leaves_append (ir_component_ptr comp)
  {
    if (optional_ref s { maybe_get_as<ir_structure> (comp) })
      m_leaf_cache.insert (m_leaf_cache.end (), s->leaves_begin (), s->leaves_end ());
    else
      m_leaf_cache.emplace_back (get_as<ir_block> (comp));
  }

  void
  ir_structure::
  invalidate_leaf_cache (void) noexcept
  {
    if (! m_leaf_cache.empty ())
    {
      m_leaf_cache.clear ();
      maybe_get_parent () >>= [this](ir_structure& s)
                              {
                                if (s.is_leaf (*this))
                                  s.invalidate_leaf_cache ();
                              };
    }
  }

  std::pair<ir_component_ptr, ir_component_ptr>
  ir_structure::
  split (ir_component_ptr block_ptr, ir_instruction_iter pivot)
  {
    assert (holds_type<ir_block> (block_ptr) && "block_ptr must point to an ir_block");
    ir_block& src = get_as<ir_block> (block_ptr);

    // pivot is now in src

    ir_component_sequence& new_seq = mutate<ir_component_sequence> (make_handle (block_ptr));
    ir_block& dst = new_seq.emplace_back<ir_block> ();

    // new_seq now has size 2
    assert (new_seq.size () == 2);

    // transfer instructions which occur after the pivot
    src.split_into (pivot, dst);

    assert (&dst == &new_seq.back ());
    return { new_seq.begin (), new_seq.last () };
  }

  ir_use_timeline&
  ir_structure::
  join_incoming_at (ir_block& block, ir_variable& var)
  {
    return join_incoming_at (get_ptr (block), block.get_def_timeline (var));
  }

  //
  // non-member functions
  //

  ir_structure::link_vector
  copy_leaves (ir_component_ptr comp)
  {
    // if the component is an ir_block, return a singleton vector
    // containing just that component
    if (optional_ref opt_block { maybe_get_as<ir_block> (comp) })
      return { *opt_block };

    // otherwise delegate to the ir_structure
    return get_as<ir_structure> (comp).get_leaves ();
  }

}

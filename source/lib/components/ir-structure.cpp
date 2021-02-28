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
#include "components/ir-structure.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-block.hpp"

#include "visitors/structure/inspectors/utility/ir-subcomponent-inspector.hpp"
#include "visitors/component/inspectors/ir-leaf-collector.hpp"

#include <numeric>

namespace gch
{
  ir_structure::
  ~ir_structure (void) = default;

  ir_substructure::
  ~ir_substructure (void) = default;

  const ir_link_set<ir_block>&
  ir_structure::
  get_leaves (void) const
  {
    if (m_leaf_cache.empty ())
      m_leaf_cache = collect_leaves (*this);
    return m_leaf_cache;
  }

  void
  ir_structure::
  invalidate_leaf_cache (void) noexcept
  {
    if (! m_leaf_cache.empty ())
    {
      m_leaf_cache.clear ();

      maybe_cast<ir_subcomponent> (this)
        >>= [](ir_subcomponent& sub)
            {
              if (is_leaf (sub))
                sub.get_parent ().invalidate_leaf_cache ();
            };
    }
  }

  std::pair<ir_component_ptr, ir_component_ptr>
  ir_structure::
  split (ir_component_ptr block_ptr, ir_instruction_iter pivot)
  {
    assert (holds_type<ir_block> (block_ptr) && "block_ptr must point to an ir_block");
    ir_block& src = as_type<ir_block> (block_ptr);

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

  //
  // non-member functions
  //

}

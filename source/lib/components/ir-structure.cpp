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

#include "components/ir-component.hpp"
#include "components/ir-structure.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-block.hpp"

#include "visitors/structure/inspectors/utility/ir-subcomponent-inspector.hpp"
#include "visitors/component/inspectors/ir-leaf-collector.hpp"
#include "visitors/ir-all-structure-visitors.hpp"

#include <numeric>

namespace gch
{
  ir_structure::
  ~ir_structure (void) = default;

  ir_substructure::
  ~ir_substructure (void) = default;

  auto
  ir_structure::
  leaves_begin (void) const noexcept
    -> leaves_const_iterator
  {
   return get_leaves ().begin ();
  }

  auto
  ir_structure::
  leaves_end (void) const noexcept
    -> leaves_const_iterator
  {
   return get_leaves ().end ();
  }

  auto
  ir_structure::
  leaves_rbegin (void) const noexcept
    -> leaves_const_reverse_iterator
  {
   return get_leaves ().rbegin ();
  }

  auto
  ir_structure::
  leaves_rend (void) const noexcept
    -> leaves_const_reverse_iterator
  {
   return get_leaves ().rend ();
  }

  auto
  ir_structure::
  leaves_front (void) const noexcept
    -> leaves_value_type
  {
    return *leaves_begin ();
  }

  auto
  ir_structure::
  leaves_back (void) const noexcept
    -> leaves_value_type
  {
    return *leaves_rbegin ();
  }

  auto
  ir_structure::
  leaves_size (void) const noexcept
    -> leaves_size_type
  {
   return get_leaves ().size ();
  }

  [[nodiscard]]
  bool
  ir_structure::
  leaves_empty (void) const noexcept
  {
    return get_leaves ().empty ();
  }

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

  //
  // non-member functions
  //

  ir_link_set<ir_block>
  collect_leaves (const ir_structure& s)
  {
    return ir_leaf_collector { } (s);
  }

  ir_link_set<const ir_block>
  collect_const_leaves (const ir_structure& s)
  {
    return ir_const_leaf_collector { } (s);
  }

  void
  flatten (ir_structure& s)
  {
    return ir_structure_flattener { } (s);
  }

}

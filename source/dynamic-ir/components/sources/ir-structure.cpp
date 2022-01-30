/** ir-structure.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-component.hpp"
#include "ir-structure.hpp"
#include "ir-component-sequence.hpp"
#include "ir-block.hpp"

#include "component/inspectors/ir-leaf-collector.hpp"
#include "ir-all-structure-visitors.hpp"

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

      maybe_cast<ir_subcomponent> (this) >>= [](ir_subcomponent& sub) {
        if (is_leaf (sub))
          sub.get_parent ().invalidate_leaf_cache ();
      };
    }
  }

  ir_substructure::
  ir_substructure (ir_structure& parent)
    : ir_subcomponent { parent }
  { }

  ir_substructure::
  ir_substructure (ir_structure& parent, std::string_view base_name)
    : ir_subcomponent (parent, base_name)
  { }

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

  ir_block&
  get_entry_block (ir_structure& s)
  {
    return as_mutable (get_entry_block (std::as_const (s)));
  }

  const ir_block&
  get_entry_block (const ir_structure& s)
  {
    nonnull_ptr curr { get_entry_component (s) };

    while (optional_ref structure { maybe_cast<ir_structure> (*curr) })
      curr.emplace (get_entry_component (*structure));

    assert (is_a<ir_block> (*curr) && "Result should be a block.");
    return static_cast<const ir_block&> (*curr);
  }

  ir_subcomponent&
  get_entry_component (ir_structure& s)
  {
    return as_mutable (get_entry_component (std::as_const (s)));
  }

  const ir_subcomponent&
  get_entry_component (const ir_structure& s)
  {
    return ir_entry_collector { } (s);
  }

}

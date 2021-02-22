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

#ifndef OCTAVE_IR_IR_STRUCTURE_HPP
#define OCTAVE_IR_IR_STRUCTURE_HPP

#include <gch/small_vector.hpp>

#include "ir-component.hpp"

#include "utilities/ir-link-set.hpp"
#include "values/ir-instruction-fwd.hpp"
#include "visitors/ir-visitor.hpp"
#include "visitors/ir-visitor-fwd.hpp"

namespace gch
{
  class ir_use_timeline;

  class ir_structure
    : public abstract_visitable<ir_structure_visitors>
  {
  public:
    using leaves_container_type          = ir_link_set<ir_block>;
    using leaves_value_type              = typename leaves_container_type::value_type;
    using leaves_size_type               = typename leaves_container_type::size_type;
    using leaves_difference_type         = typename leaves_container_type::difference_type;

    using leaves_iterator                = typename leaves_container_type::iterator;
    using leaves_const_iterator          = typename leaves_container_type::const_iterator;
    using leaves_reverse_iterator        = typename leaves_container_type::reverse_iterator;
    using leaves_const_reverse_iterator  = typename leaves_container_type::const_reverse_iterator;

    using leaves_val_t   = leaves_value_type;
    using leaves_size_ty = leaves_size_type;
    using leaves_diff_ty = leaves_difference_type;

    using leaves_iter    = leaves_iterator;
    using leaves_citer   = leaves_const_iterator;
    using leaves_riter   = leaves_reverse_iterator;
    using leaves_criter  = leaves_const_reverse_iterator;

    ir_structure            (void)                    = default;
    ir_structure            (const ir_structure&)     = default;
    ir_structure            (ir_structure&&) noexcept = default;
    ir_structure& operator= (const ir_structure&)     = default;
    ir_structure& operator= (ir_structure&&) noexcept = default;
    virtual ~ir_structure   (void)                    = 0;

    [[nodiscard]]
    leaves_const_iterator
    leaves_begin (void) const noexcept
    {
     return get_leaves ().begin ();
    }

    [[nodiscard]]
    leaves_const_iterator
    leaves_end (void) const noexcept
    {
     return get_leaves ().end ();
    }

    [[nodiscard]]
    leaves_const_reverse_iterator
    leaves_rbegin (void) const noexcept
    {
     return get_leaves ().rbegin ();
    }

    [[nodiscard]]
    leaves_const_reverse_iterator
    leaves_rend (void) const noexcept
    {
     return get_leaves ().rend ();
    }

    [[nodiscard]]
    leaves_value_type
    leaves_front (void) const noexcept
    {
      return *leaves_begin ();
    }

    [[nodiscard]]
    leaves_value_type
    leaves_back (void) const noexcept
    {
      return *leaves_rbegin ();
    }

    [[nodiscard]]
    leaves_size_type
    leaves_size (void) const noexcept
    {
     return get_leaves ().size ();
    }

    [[nodiscard]]
    bool
    leaves_empty (void) const noexcept
    {
      return get_leaves ().empty ();
    }

    [[nodiscard]]
    const ir_link_set<ir_block>&
    get_leaves (void) const;

    // mutate a component inside a structure to a different type of component
    template <typename T>
    T&
    mutate (ir_component_handle comp)
    {
      comp.get_storage () = allocate_subcomponent<T> (make_mover (comp));
      return as_type<T> (comp);
    }

    void
    invalidate_leaf_cache (void) noexcept;

    // returns ptrs to the newly split blocks (inside the mutated block_ptr)
    std::pair<ir_component_ptr, ir_component_ptr>
    split (ir_component_ptr block_ptr, ir_instruction_iter pivot);

  protected:
    static
    ir_component_handle
    make_handle (ir_component_ptr comp) noexcept
    {
      return ir_component_handle { as_mutable (comp.get_storage ()) };
    }

    [[nodiscard]]
    bool
    leaf_cache_empty (void) const noexcept
    {
      return m_leaf_cache.empty ();
    }

    void
    clear_leaf_cache (void) noexcept
    {
      m_leaf_cache.clear ();
    }

    template <typename Component, typename ...Args,
              typename = std::enable_if_t<std::is_base_of_v<ir_component, Component>>>
    std::unique_ptr<Component>
    allocate_subcomponent (Args&&... args)
    {
      return std::make_unique<Component> (*this, std::forward<Args> (args)...);
    }

    //
    // virtual functions
    //

  public:
    virtual
    ir_use_timeline&
    join_incoming_at (ir_component_ptr pos, ir_def_timeline& dt) = 0;

    //
    // virtual function accessories
    //

    ir_use_timeline&
    join_incoming_at (ir_block& block, ir_variable& var);

    //
    // functions related to virtual functions
    //

  private:
    mutable ir_link_set<ir_block> m_leaf_cache;
  };

  // a substructure is any structure that isn't an function
  class ir_substructure
    : public ir_structure,
      public ir_subcomponent
  {
  public:
    explicit
    ir_substructure (ir_structure& parent)
      : ir_subcomponent { parent }
    { }
  };

  ir_link_set<ir_block>
  collect_leaves (const ir_structure& s);

  ir_subcomponent&
  get_entry_component (ir_structure& s);

  const ir_subcomponent&
  get_entry_component (const ir_structure& s);

  ir_block&
  get_entry_block (ir_structure& s);

  const ir_block&
  get_entry_block (const ir_structure& s);

  void
  flatten (ir_structure& s);

}

#endif

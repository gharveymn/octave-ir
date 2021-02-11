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

#if ! defined (ir_structure_h)
#define ir_structure_h 1

#include <gch/small_vector.hpp>

#include "ir-link-set.hpp"
#include "ir-component.hpp"
#include "ir-instruction-fwd.hpp"

#include <stack>
#include <vector>

namespace gch
{

  class ir_block;
  class ir_condition_block;
  class ir_loop_condition_block;
  class ir_function;
  class ir_use_timeline;
  // class ir_structure_descender;
  // class ir_structure_ascender;

  class ir_structure
    : public ir_component
  {
  public:
    using ptr     = ir_component_ptr;
    using cptr    = ir_component_cptr;
    using rptr    = std::reverse_iterator<ptr>;
    using crptr   = std::reverse_iterator<cptr>;
    using ref     = ir_component&;
    using cref    = const ir_component&;
    using val_t   = ir_component;
    using size_ty = std::size_t;
    using diff_ty = typename std::iterator_traits<ptr>::difference_type;

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

    static constexpr struct construct_with_parent_tag { } construct_with_parent { };

    ir_structure            (void)                    = delete;
    ir_structure            (const ir_structure&)     = default;
    ir_structure            (ir_structure&&) noexcept = default;
    ir_structure& operator= (const ir_structure&)     = default;
    ir_structure& operator= (ir_structure&&) noexcept = default;
    ~ir_structure           (void) override           = 0;

    explicit
    ir_structure (construct_with_parent_tag, ir_structure& parent)
      : ir_component (parent)
    { }

    explicit
    ir_structure (nullopt_t)
      : ir_component (nullopt)
    { }

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
    front (void) const noexcept
    {
      return *leaves_begin ();
    }

    [[nodiscard]]
    leaves_value_type
    back (void) const noexcept
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
    get_leaves (void);

    [[nodiscard]]
    const ir_link_set<ir_block>&
    get_leaves (void) const
    {
      return as_mutable (*this).get_leaves ();
    }

    // mutate a component inside a structure to a different type of component
    template <typename T>
    T&
    mutate (ir_component_handle comp)
    {
      comp.get_storage () = create_component<T> (make_mover (comp));
      return get_as<T> (comp);
    }

    void
    invalidate_leaf_cache (void) noexcept;

    void
    invalidate_entry_cache (void) noexcept;

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

    void
    leaves_append (ir_component_ptr comp);

    [[nodiscard]]
    bool
    entry_cache_empty (void) const noexcept
    {
      return ! m_entry_cache.has_value ();
    }

    ir_block&
    set_entry_cache (ir_block& block)
    {
      return m_entry_cache.emplace (block);
    }

    template <typename Component, typename ...Args,
              typename = std::enable_if_t<is_component<Component>::value>>
    std::unique_ptr<Component>
    create_component (Args&&... args)
    {
      return std::make_unique<Component> (*this, std::forward<Args> (args)...);
    }

    //
    // virtual functions
    //

  public:
    [[nodiscard]]
    virtual
    ir_component_ptr
    get_ptr (ir_component& c) const = 0;

    [[nodiscard]]
    virtual
    ir_component_ptr
    get_entry_ptr (void) = 0;

    [[nodiscard]]
    virtual
    ir_link_set
    get_predecessors (ir_component_cptr comp) = 0;

    [[nodiscard]]
    virtual
    ir_link_set
    get_successors (ir_component_cptr comp) = 0;

    [[nodiscard]]
    virtual
    bool
    is_leaf (ir_component_cptr comp) noexcept = 0;

    virtual
    void
    generate_leaf_cache (void) = 0;

    virtual
    ir_use_timeline&
    join_incoming_at (ir_component_ptr pos, ir_def_timeline& dt) = 0;

    virtual
    void
    recursive_flatten (void) = 0;

    virtual
    void
    reassociate_timelines_after (ir_component_ptr pos, ir_def_timeline& dt,
                                 std::vector<nonnull_ptr<ir_block>>& until) = 0;

    //
    // virtual function accessories
    //

    [[nodiscard]]
    ir_component_cptr
    get_ptr (const ir_component& c) const
    {
      return get_ptr (as_mutable (c));
    }

    ir_use_timeline&
    join_incoming_at (ir_block& block, ir_variable& var);

    ir_link_set
    get_predecessors (const ir_component& c)
    {
      return get_predecessors (get_ptr (c));
    }

    ir_link_set
    get_successors (const ir_component& c)
    {
      return get_successors (get_ptr (c));
    }

    bool
    is_leaf (const ir_component& c) noexcept
    {
      return is_leaf (get_ptr (c));
    }

    //
    // functions related to virtual functions
    //

    [[nodiscard]]
    ir_component_cptr
    get_entry_ptr (void) const
    {
      return as_mutable (*this).get_entry_ptr ();
    }

    [[nodiscard]]
    bool
    is_entry (ir_component_cptr comp) noexcept
    {
      return comp == get_entry_ptr ();
    }

  private:
    optional_ref<ir_block> m_entry_cache;
    ir_link_set            m_leaf_cache;
  };

  // a substructure is any structure that isn't an function
  class ir_substructure
    : public ir_structure
  {
  public:
    ir_substructure            (void)                       = delete;
    ir_substructure            (const ir_substructure&)     = default;
    ir_substructure            (ir_substructure&&) noexcept = default;
    ir_substructure& operator= (const ir_substructure&)     = default;
    ir_substructure& operator= (ir_substructure&&) noexcept = default;
    ~ir_substructure           (void) override              = 0;

    explicit
    ir_substructure (ir_structure& parent)
      : ir_structure (construct_with_parent, parent)
    { }

    [[nodiscard]] constexpr
    ir_structure&
    get_parent (void) noexcept
    {
      return *maybe_get_parent ();
    }

    [[nodiscard]] constexpr
    const ir_structure&
    get_parent (void) const noexcept
    {
      return *maybe_get_parent ();
    }
  };

  [[nodiscard]] inline
  ir_block&
  get_entry_block (ir_structure& s)
  {
    return get_entry_block (*s.get_entry_ptr ());
  }

  [[nodiscard]] inline
  const ir_block&
  get_entry_block (const ir_structure& s)
  {
    return get_entry_block (*s.get_entry_ptr ());
  }

  [[nodiscard]]
  ir_link_set
  copy_leaves (ir_component_ptr comp);

  template <typename ...Args>
  [[nodiscard]] inline
  ir_link_set
  copy_leaves (ir_component_ptr comp, Args&&... args)
  {
    return (copy_leaves (comp) | ... | copy_leaves (std::forward<Args> (args)));
  }

}

#endif

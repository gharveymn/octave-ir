/** ir-structure.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_HPP

#include "ir-component.hpp"

#include "ir-link-set.hpp"
#include "ir-instruction-fwd.hpp"
#include "ir-visitor.hpp"

#include "structure/ir-structure-visitors-fwd.hpp"
#include "substructure/ir-substructure-visitors-fwd.hpp"

#include <gch/small_vector.hpp>

namespace gch
{
  class ir_use_timeline;

  class ir_structure
    : public abstract_visitable<exclusive_visitors_t<ir_structure>>
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
    ~ir_structure           (void) override;

    [[nodiscard]]
    leaves_const_iterator
    leaves_begin (void) const noexcept;

    [[nodiscard]]
    leaves_const_iterator
    leaves_end (void) const noexcept;

    [[nodiscard]]
    leaves_const_reverse_iterator
    leaves_rbegin (void) const noexcept;

    [[nodiscard]]
    leaves_const_reverse_iterator
    leaves_rend (void) const noexcept;

    [[nodiscard]]
    leaves_value_type
    leaves_front (void) const noexcept;

    [[nodiscard]]
    leaves_value_type
    leaves_back (void) const noexcept;

    [[nodiscard]]
    leaves_size_type
    leaves_size (void) const noexcept;

    [[nodiscard]]
    bool
    leaves_empty (void) const noexcept;

    [[nodiscard]]
    const ir_link_set<ir_block>&
    get_leaves (void) const;

    void
    invalidate_leaf_cache (void) noexcept;

    // Mutate a component inside a structure to a different type of component.
    template <typename T>
    T&
    mutate (ir_component_handle comp)
    {
      comp.get_storage () = allocate_subcomponent<T> (make_mover (comp));
      return as_component<T> (comp);
    }

  protected:
    static
    ir_component_handle
    make_handle (ir_component_ptr comp) noexcept
    {
      return ir_component_handle { as_mutable (comp.get_storage ()) };
    }

    template <typename Component, typename ...Args,
              std::enable_if_t<std::is_base_of_v<ir_subcomponent, Component>
                           &&  std::is_constructible_v<Component, ir_structure&, Args...>
                               > * = nullptr>
    std::unique_ptr<Component>
    allocate_subcomponent (Args&&... args)
    {
      return std::make_unique<Component> (*this, std::forward<Args> (args)...);
    }

  private:
    mutable ir_link_set<ir_block> m_leaf_cache;
  };

  // a substructure is any structure that isn't an function
  class ir_substructure
    : public ir_structure,
      public ir_subcomponent
  {
  public:
    ir_substructure            (void)                       = delete;
    ir_substructure            (const ir_substructure&)     = default;
    ir_substructure            (ir_substructure&&) noexcept = default;
    ir_substructure& operator= (const ir_substructure&)     = default;
    ir_substructure& operator= (ir_substructure&&) noexcept = default;
    ~ir_substructure           (void) override;

    using ir_structure::accept;
    using ir_subcomponent::accept;

    explicit
    ir_substructure (ir_structure& parent);

    explicit
    ir_substructure (ir_structure& parent, std::string_view base_name);
  };

  ir_link_set<ir_block>
  collect_leaves (const ir_structure& s);

  ir_link_set<const ir_block>
  collect_const_leaves (const ir_structure& s);

  void
  flatten (ir_structure& s);

  ir_block&
  get_entry_block (ir_structure& s);

  const ir_block&
  get_entry_block (const ir_structure& s);

  ir_subcomponent&
  get_entry_component (ir_structure& s);

  const ir_subcomponent&
  get_entry_component (const ir_structure& s);

  template <typename T, typename std::enable_if_t<std::is_base_of_v<ir_structure, T>> * = nullptr>
  inline
  ir_block&
  get_entry_block (T& s)
  {
    return get_entry_block (static_cast<ir_structure&> (s));
  }

  template <typename T, typename std::enable_if_t<std::is_base_of_v<ir_structure, T>> * = nullptr>
  inline
  const ir_block&
  get_entry_block (const T& s)
  {
    return get_entry_block (static_cast<const ir_structure&> (s));
  }

}

#endif

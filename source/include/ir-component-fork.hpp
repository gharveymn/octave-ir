/** ir-fork-component.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_COMPONENT_FORK_HPP
#define OCTAVE_IR_IR_COMPONENT_FORK_HPP

#include "ir-structure.hpp"

namespace gch
{
  class ir_component_fork : public ir_structure
  {
  public:
    using component_container     = std::vector<ir_component_storage>;
    using iterator                = typename component_container::iterator;
    using const_iterator          = typename component_container::const_iterator;
    using reverse_iterator        = typename component_container::reverse_iterator;
    using const_reverse_iterator  = typename component_container::const_reverse_iterator;
    using reference               = typename component_container::reference;
    using const_reference         = typename component_container::const_reference;
    using value_type              = typename component_container::value_type;
    using allocator_type          = typename component_container::allocator_type;
    using size_type               = typename component_container::size_type;
    using difference_type         = typename component_container::difference_type;

    using iter    = iterator;
    using citer   = const_iterator;
    using riter   = reverse_iterator;
    using criter  = const_reverse_iterator;
    using ref     = reference;
    using cref    = const_reference;
    using val_t   = value_type;
    using alloc_t = allocator_type;
    using size_t  = size_type;
    using diff_t  = difference_type;

    ir_component_fork (void)                         = delete;
    ir_component_fork (const ir_component_fork&)     = delete;
    ir_component_fork (ir_component_fork&&) noexcept = delete;
    ir_component_fork& operator= (const ir_component_fork&)     = delete;
    ir_component_fork& operator= (ir_component_fork&&) noexcept = delete;
    ~ir_component_fork (void)       noexcept override;

    explicit ir_component_fork (ir_structure& parent)
      : ir_structure (parent),
        m_condition (create_component<ir_condition_block> (*this))
    { }

    [[nodiscard]] constexpr
    ir_component_handle
    get_condition_component (void) const noexcept
    {
      ir_component_handle x { m_condition };
      return m_condition;
    }

    [[nodiscard]] constexpr
    bool
    is_condition_component (ir_component_handle comp) const noexcept
    {
      return comp == get_condition_component ();
    }

    [[nodiscard]]
    std::list<nonnull_ptr<ir_def>>
    get_latest_defs (ir_variable& var) noexcept override
    {
      bool condition_visited = false;
      std::list<nonnull_ptr<ir_def>> ret;

      for (auto&& comp : m_subcomponents)
      {
        auto defs = comp->get_latest_defs (var);
        if (defs.empty ())
        {
          if (! condition_visited)
          {
            ret.splice (ret.end (), m_condition.get_latest_defs (var));
            condition_visited = true;
          }
        }
        else
        {
          ret.splice (ret.end (), defs);
        }
      }
      return ret;
    }

    [[nodiscard]]
    std::list<nonnull_ptr<ir_def>>
    get_latest_defs_before (ir_variable& var, component_handle comp) override
    {
      if (comp->get () == &m_condition)
        return { };
      return m_condition.get_latest_defs (var);
    }

    //
    // virtual from ir_component
    //

    void
    reset (void) noexcept override;

    //
    // virtual from ir_structure
    //

    [[nodiscard]]
    ir_component_handle
    get_entry_component (void) override;

    [[nodiscard]]
    ir_component_handle
    get_handle (const ir_component& c) const override;

    link_vector
    get_preds (ir_component_handle comp) override;

    link_vector
    get_succs (ir_component_handle comp) override;

    ir_use_timeline&
    join_incoming_at (ir_component_handle& block_handle, ir_def_timeline& dt) override;

    void
    generate_leaf_cache (void) override;

    [[nodiscard]]
    bool
    is_leaf_component (ir_component_handle comp) noexcept override;

  private:
    link_iter sub_entry_begin (void);
    link_iter sub_entry_end   (void);

    void generate_sub_entry_cache (void);

    ir_component_storage m_condition;
    component_container m_subcomponents;

    // holds entry blocks for subcomponents
    link_vector m_sub_entry_cache;
  };
}

#endif // OCTAVE_IR_IR_COMPONENT_FORK_HPP

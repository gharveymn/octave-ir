/** ir-fork-component.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_FORK_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_FORK_HPP

#include "ir-structure.hpp"
#include "ir-block.hpp"
#include "ir-visitor.hpp"

namespace gch
{

  class ir_component_fork final
    : public ir_substructure,
      public visitable<ir_component_fork, consolidated_visitors_t<ir_substructure>>
  {
  public:
    using cases_container = std::vector<ir_component_storage>;
    using cases_iter      = ir_component_ptr;
    using cases_citer     = ir_component_cptr;
    using cases_riter     = std::reverse_iterator<cases_iter>;
    using cases_criter    = std::reverse_iterator<cases_citer>;
    using cases_ref       = ir_subcomponent&;
    using cases_cref      = const ir_subcomponent&;
    using cases_size_ty   = typename cases_container::size_type;
    using cases_diff_ty   = typename cases_container::difference_type;

    ir_component_fork (void)                                    = delete;
    ir_component_fork (const ir_component_fork&)                = delete;
    ir_component_fork (ir_component_fork&&) noexcept            = delete;
    ir_component_fork& operator= (const ir_component_fork&)     = delete;
    ir_component_fork& operator= (ir_component_fork&&) noexcept = delete;
    ~ir_component_fork (void)       noexcept override;

    ir_component_fork (ir_structure& parent, ir_variable& condition_var);

    [[nodiscard]]
    ir_block&
    get_condition (void) noexcept;

    [[nodiscard]]
    const ir_block&
    get_condition (void) const noexcept;

    [[nodiscard]]
    bool
    is_condition (const ir_subcomponent& sub) const noexcept;

    [[nodiscard]]
    cases_iter
    cases_begin (void) noexcept;

    [[nodiscard]]
    cases_citer
    cases_begin (void) const noexcept;

    [[nodiscard]]
    cases_citer
    cases_cbegin (void) const noexcept;

    [[nodiscard]]
    cases_iter
    cases_end (void) noexcept;

    [[nodiscard]]
    cases_citer
    cases_end (void) const noexcept;

    [[nodiscard]]
    cases_citer
    cases_cend (void) const noexcept;

    [[nodiscard]]
    cases_riter
    cases_rbegin (void) noexcept;

    [[nodiscard]]
    cases_criter
    cases_rbegin (void) const noexcept;

    [[nodiscard]]
    cases_criter
    cases_crbegin (void) const noexcept;

    [[nodiscard]]
    cases_riter
    cases_rend (void) noexcept;

    [[nodiscard]]
    cases_criter
    cases_rend (void) const noexcept;

    [[nodiscard]]
    cases_criter
    cases_crend (void) const noexcept;

    [[nodiscard]]
    cases_ref
    cases_front (void);

    [[nodiscard]]
    cases_cref
    cases_front (void) const;

    [[nodiscard]]
    cases_ref
    cases_back (void);

    [[nodiscard]]
    cases_cref
    cases_back (void) const;

    [[nodiscard]]
    cases_size_ty
    num_cases (void) const noexcept;

    [[nodiscard]]
    bool
    has_cases (void) const noexcept;

    [[nodiscard]]
    cases_iter
    find_case (ir_component& c) const;

    [[nodiscard]]
    cases_citer
    find_case (const ir_component& c) const;

    template <typename Component, typename ...Args,
              typename = std::enable_if_t<is_ir_component_v<Component>>>
    Component&
    add_case (Args&&... args)
    {
      m_cases.push_back (allocate_subcomponent<Component> (std::forward<Args> (args)...));
      invalidate_leaf_cache ();
      return as_component<Component> (std::prev (cases_end ()));
    }

  private:
    ir_block        m_condition;
    cases_container m_cases;
  };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_FORK_HPP

/** ir-fork-component.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_COMPONENT_FORK_HPP
#define OCTAVE_IR_IR_COMPONENT_FORK_HPP

#include "components/ir-structure.hpp"
#include "components/ir-block.hpp"
#include "visitors/ir-visitor.hpp"

namespace gch
{

  class ir_component_fork
    : public ir_substructure,
      public visitable<ir_component_fork, implemented_visitors_t<ir_substructure>>
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

    explicit
    ir_component_fork (ir_structure& parent);

    [[nodiscard]]
    ir_block&
    get_condition (void) noexcept
    {
      return m_condition;
    }

    [[nodiscard]]
    const ir_block&
    get_condition (void) const noexcept
    {
      return as_mutable (*this).get_condition ();
    }

    [[nodiscard]]
    bool
    is_condition (const ir_subcomponent& sub) const noexcept;

    [[nodiscard]]
    cases_iter
    cases_begin (void) noexcept
    {
      return make_ptr (m_cases.begin ());
    }

    [[nodiscard]]
    cases_citer
    cases_begin (void) const noexcept
    {
      return as_mutable (*this).cases_begin ();
    }

    [[nodiscard]]
    cases_citer
    cases_cbegin (void) const noexcept
    {
      return cases_begin ();
    }

    [[nodiscard]]
    cases_iter
    cases_end (void) noexcept
    {
      return make_ptr (m_cases.end ());
    }

    [[nodiscard]]
    cases_citer
    cases_end (void) const noexcept
    {
      return as_mutable (*this).cases_end ();
    }

    [[nodiscard]]
    cases_citer
    cases_cend (void) const noexcept
    {
      return cases_end ();
    }

    [[nodiscard]]
    cases_riter
    cases_rbegin (void) noexcept
    {
      return cases_riter { cases_end () };
    }

    [[nodiscard]]
    cases_criter
    cases_rbegin (void) const noexcept
    {
      return as_mutable (*this).cases_rbegin ();
    }

    [[nodiscard]]
    cases_criter
    cases_crbegin (void) const noexcept
    {
      return cases_rbegin ();
    }

    [[nodiscard]]
    cases_riter
    cases_rend (void) noexcept
    {
      return cases_riter { cases_begin () };
    }

    [[nodiscard]]
    cases_criter
    cases_rend (void) const noexcept
    {
      return as_mutable (*this).cases_rend ();
    }

    [[nodiscard]]
    cases_criter
    cases_crend (void) const noexcept
    {
      return cases_rend ();
    }

    [[nodiscard]]
    cases_ref
    cases_front (void)
    {
      return *cases_begin ();
    }

    [[nodiscard]]
    cases_cref
    cases_front (void) const
    {
      return *cases_begin ();
    }

    [[nodiscard]]
    cases_ref
    cases_back (void)
    {
      return *cases_rbegin ();
    }

    [[nodiscard]]
    cases_cref
    cases_back (void) const
    {
      return *cases_rbegin ();
    }

    [[nodiscard]]
    cases_size_ty
    cases_size (void) const noexcept
    {
     return m_cases.size ();
    }

    [[nodiscard]]
    bool
    cases_empty (void) const noexcept
    {
      return m_cases.empty ();
    }

    [[nodiscard]]
    cases_iter
    find_case (ir_component& c) const;

    [[nodiscard]]
    cases_citer
    find_case (const ir_component& c) const
    {
      return find_case (as_mutable (c));
    }

  private:
    ir_condition_block m_condition;
    cases_container    m_cases;
  };

}

#endif // OCTAVE_IR_IR_COMPONENT_FORK_HPP

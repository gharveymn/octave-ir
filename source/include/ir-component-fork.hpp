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

  class ir_component_fork;

  template <>
  struct ir_subcomponent_type_t<ir_component_fork>
  {
    explicit ir_subcomponent_type_t (void) = default;
  };

  class ir_component_fork
    : public ir_substructure
  {
  public:
    using component_container = std::vector<ir_component_storage>;
    using container_iter      = typename component_container::iterator;
    using container_citer     = typename component_container::const_iterator;
    using container_riter     = typename component_container::reverse_iterator;
    using container_criter    = typename component_container::const_reverse_iterator;
    using container_ref       = typename component_container::reference;
    using container_cref      = typename component_container::const_reference;
    using container_val_t     = typename component_container::value_type;
    using container_alloc_t   = typename component_container::allocator_type;
    using container_size_t    = typename component_container::size_type;
    using container_diff_t    = typename component_container::difference_type;

  private:
    class find_cache
    {
    public:

      find_cache            (void)                  = default;
      find_cache            (const find_cache&)     = default;
      find_cache            (find_cache&&) noexcept = default;
      find_cache& operator= (const find_cache&)     = default;
      find_cache& operator= (find_cache&&) noexcept = default;
      ~find_cache           (void)                  = default;

      explicit
      find_cache (ir_component_handle it) noexcept
        : m_handle (it)
      { }

      void
      emplace (ir_component_handle it) noexcept
      {
        m_handle = it;
      }

      [[nodiscard]] constexpr
      bool
      contains (const ir_component& c) const noexcept
      {
        return &c == m_handle;
      }

      [[nodiscard]]
      ir_component_handle
      get (void) const noexcept
      {
        return m_handle;
      }

    private:
      ir_component_handle m_handle;
    };

  public:
    ir_component_fork (void)                                    = delete;
    ir_component_fork (const ir_component_fork&)                = delete;
    ir_component_fork (ir_component_fork&&) noexcept            = delete;
    ir_component_fork& operator= (const ir_component_fork&)     = delete;
    ir_component_fork& operator= (ir_component_fork&&) noexcept = delete;
    ~ir_component_fork (void)       noexcept override;

    explicit
    ir_component_fork (ir_structure& parent);

    [[nodiscard]] constexpr
    ir_component_ptr
    get_condition (void) noexcept
    {
      return ir_component_ptr { &m_condition };
    }

    [[nodiscard]] constexpr
    ir_component_cptr
    get_condition (void) const noexcept
    {
      return ir_component_cptr { &m_condition };
    }

    [[nodiscard]] constexpr
    bool
    is_condition (ir_component_cptr comp) const noexcept
    {
      return comp == get_condition ();
    }

    [[nodiscard]] constexpr
    bool
    is_condition (const ir_component& c) const noexcept
    {
      return &c == get_condition ();
    }

    [[nodiscard]]
    ptr
    cases_begin (void) noexcept
    {
      return make_ptr (m_cases.begin ());
    }

    [[nodiscard]]
    cptr
    cases_begin (void) const noexcept
    {
      return make_ptr (m_cases.begin ());
    }

    [[nodiscard]]
    cptr
    cases_cbegin (void) const noexcept
    {
      return as_mutable (*this).cases_begin ();
    }

    [[nodiscard]]
    ptr
    cases_end (void) noexcept
    {
      return make_ptr (m_cases.end ());
    }

    [[nodiscard]]
    cptr
    cases_end (void) const noexcept
    {
      return make_ptr (m_cases.end ());
    }

    [[nodiscard]]
    cptr
    cases_cend (void) const noexcept
    {
      return as_mutable (*this).cases_end ();
    }

    [[nodiscard]]
    rptr
    cases_rbegin (void) noexcept
    {
      return std::make_reverse_iterator (cases_end ());
    }

    [[nodiscard]]
    crptr
    cases_rbegin (void) const noexcept
    {
      return std::make_reverse_iterator (cases_end ());
    }

    [[nodiscard]]
    crptr
    cases_crbegin (void) const noexcept
    {
      return as_mutable (*this).cases_rbegin ();
    }

    [[nodiscard]]
    rptr
    cases_rend (void) noexcept
    {
      return std::make_reverse_iterator (cases_begin ());
    }

    [[nodiscard]]
    crptr
    cases_rend (void) const noexcept
    {
      return std::make_reverse_iterator (cases_begin ());
    }

    [[nodiscard]]
    crptr
    cases_crend (void) const noexcept
    {
      return as_mutable (*this).cases_rend ();
    }

    [[nodiscard]]
    size_ty
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
    ptr
    find_case (ir_component& c) const;

    [[nodiscard]]
    cptr
    find_case (const ir_component& c) const
    {
      return find_case (as_mutable (c));
    }

    [[nodiscard]]
    ptr
    find (ir_component& c) const;

    [[nodiscard]]
    cptr
    find (const ir_component& c) const
    {
      return find (as_mutable (c));
    }

    //
    // virtual from ir_component
    //

    bool
    reassociate_timelines (const std::vector<nonnull_ptr<ir_def_timeline>>& old_dts,
                           ir_def_timeline& new_dt,
                           std::vector<nonnull_ptr<ir_block>>& until) override;

    void
    reset (void) noexcept override;

    //
    // virtual from ir_structure
    //

    [[nodiscard]]
    ir_component_ptr
    get_ptr (ir_component& c) const override;

    [[nodiscard]]
    ir_component_ptr
    get_entry_ptr (void) override;

    [[nodiscard]]
    ir_link_set
    get_predecessors (ir_component_cptr comp) override;

    [[nodiscard]]
    ir_link_set
    get_successors (ir_component_cptr comp) override;

    [[nodiscard]]
    bool
    is_leaf (ir_component_cptr comp) noexcept override;

    void
    generate_leaf_cache (void) override;

    ir_use_timeline&
    join_incoming_at (ir_component_ptr pos, ir_def_timeline& dt) override;

    void
    recursive_flatten (void) override;

  private:
    ir_component_storage m_condition;
    component_container  m_cases;
    mutable find_cache   m_find_cache;
  };

}

#endif // OCTAVE_IR_IR_COMPONENT_FORK_HPP

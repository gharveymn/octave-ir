/** ir-def-resolution.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_DEF_RESOLUTION_HPP
#define OCTAVE_IR_IR_DEF_RESOLUTION_HPP

#include "ir-link-set.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>
#include <gch/small_vector.hpp>

#include <stack>
#include <vector>

namespace gch
{

  class ir_block;
  class ir_def_timeline;
  class ir_use_timeline;
  class ir_def_resolution_stack;
  class ir_def_resolution_frame;

  class ir_def_resolution_stack
  {
  public:
    ir_def_resolution_stack            (void)                               = delete;
    ir_def_resolution_stack            (const ir_def_resolution_stack&)     = default;
    ir_def_resolution_stack            (ir_def_resolution_stack&&) noexcept = default;
    ir_def_resolution_stack& operator= (const ir_def_resolution_stack&)     = default;
    ir_def_resolution_stack& operator= (ir_def_resolution_stack&&) noexcept = default;
    ~ir_def_resolution_stack           (void)                               = default;

    explicit
    ir_def_resolution_stack (ir_block& leaf_block);

    [[nodiscard]]
    bool
    is_resolvable (void) const noexcept;

    const ir_link_set<ir_def_timeline>&
    resolve (void);

    const ir_link_set<ir_def_timeline>&
    resolve_with (ir_link_set<ir_def_timeline> c);

    [[nodiscard]]
    bool
    is_resolved (void) const noexcept;

    [[nodiscard]]
    bool
    is_resolved_nonempty (void) const noexcept;

    [[nodiscard]]
    const std::optional<ir_link_set<ir_def_timeline>>&
    maybe_get_resolution (void) const noexcept;

    [[nodiscard]]
    const ir_link_set<ir_def_timeline>&
    get_resolution (void) const noexcept;

    [[nodiscard]]
    bool
    check_all_same_nonnull_outgoing (void) const;

    [[nodiscard]]
    ir_def&
    get_resolved_def (void) const;

    [[nodiscard]]
    optional_ref<ir_def>
    maybe_get_resolved_def (void) const;

    [[nodiscard]]
    ir_block&
    get_leaf_block (void) const noexcept;

    ir_def_resolution_frame&
    push (ir_block& join_block);

    ir_def_resolution_frame&
    top (void);

    const ir_def_resolution_frame&
    top (void) const;

  private:
    ir_component_ptr                            m_component;
    std::vector<ir_def_resolution_stack>        m_leaves;
    nonnull_ptr<ir_block>                       m_leaf_block;
    std::stack<ir_def_resolution_frame>         m_stack;
    std::optional<ir_link_set<ir_def_timeline>> m_resolution;
  };


  class ir_def_resolution_frame
  {
    using stack = ir_def_resolution_stack;

  public:
    using container_type          = small_vector<ir_def_resolution_stack, 1>;
    using value_type              = typename container_type::value_type;
    using allocator_type          = typename container_type::allocator_type;
    using size_type               = typename container_type::size_type;
    using difference_type         = typename container_type::difference_type;
    using reference               = typename container_type::reference;
    using const_reference         = typename container_type::const_reference;
    using pointer                 = typename container_type::pointer;
    using const_pointer           = typename container_type::const_pointer;

    using iterator                = typename container_type::iterator;
    using const_iterator          = typename container_type::const_iterator;
    using reverse_iterator        = typename container_type::reverse_iterator;
    using const_reverse_iterator  = typename container_type::const_reverse_iterator;

    using val_t   = value_type;
    using alloc_t = allocator_type;
    using size_ty = size_type;
    using diff_ty = difference_type;
    using ref     = reference;
    using cref    = const_reference;
    using ptr     = pointer;
    using cptr    = const_pointer;

    using iter    = iterator;
    using citer   = const_iterator;
    using riter   = reverse_iterator;
    using criter  = const_reverse_iterator;

    ir_def_resolution_frame            (void)                               = delete;
    ir_def_resolution_frame            (const ir_def_resolution_frame&)     = default;
    ir_def_resolution_frame            (ir_def_resolution_frame&&) noexcept = default;
    ir_def_resolution_frame& operator= (const ir_def_resolution_frame&)     = default;
    ir_def_resolution_frame& operator= (ir_def_resolution_frame&&) noexcept = default;
    ~ir_def_resolution_frame           (void)                               = default;

    ir_def_resolution_frame (ir_block& join_block);

    [[nodiscard]]
    const_iterator
    begin (void) const noexcept
    {
     return m_incoming.begin ();
    }

    [[nodiscard]]
    const_iterator
    end (void) const noexcept
    {
     return m_incoming.end ();
    }

    [[nodiscard]]
    const_reverse_iterator
    rbegin (void) const noexcept
    {
     return m_incoming.rbegin ();
    }

    [[nodiscard]]
    const_reverse_iterator
    rend (void) const noexcept
    {
     return m_incoming.rend ();
    }

    [[nodiscard]]
    const_reference
    front (void) const
    {
      return *begin ();
    }

    [[nodiscard]]
    const_reference
    back (void) const
    {
      return *rbegin ();
    }

    [[nodiscard]]
    size_type
    size (void) const noexcept
    {
     return m_incoming.size ();
    }

    [[nodiscard]]
    bool
    empty (void) const noexcept
    {
      return m_incoming.empty ();
    }

    [[nodiscard]]
    bool
    is_joinable (void) const noexcept;

    [[nodiscard]]
    ir_link_set<ir_def_timeline>
    join (void) const;

    [[nodiscard]]
    ir_link_set<ir_def_timeline>
    join_with (ir_link_set<ir_def_timeline>&& c) const;

    ir_def_resolution_stack&
    add_substack (ir_block& leaf_block);

  private:
    nonnull_ptr<ir_block> m_join_block;
    container_type        m_incoming;
  };

}

#endif // OCTAVE_IR_IR_DEF_RESOLUTION_HPP

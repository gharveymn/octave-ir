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

  class ir_def_resolution
  {
  public:
    ir_def_resolution (ir_block& b, const ir_link_set<ir_def_timeline>& s)
      : m_leaf_block (b),
        m_timelines (s)
    { }

    [[nodiscard]]
    ir_block&
    get_leaf_block (void) const noexcept
    {
      return *m_leaf_block;
    }

    [[nodiscard]]
    const ir_link_set<ir_def_timeline>&
    get_timelines (void) const noexcept
    {
      return m_timelines;
    }

    [[nodiscard]]
    bool
    is_nonempty (void) const noexcept
    {
      return ! m_timelines.empty ();
    }

    [[nodiscard]]
    ir_def&
    get_common_def (void) const;

    [[nodiscard]]
    optional_ref<ir_def>
    maybe_get_common_def (void) const;

  private:
    nonnull_ptr<ir_block>        m_leaf_block;
    ir_link_set<ir_def_timeline> m_timelines;
  };

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
    ir_def_resolution_stack (ir_component_ptr comp);

    [[nodiscard]]
    bool
    is_resolvable (void) const noexcept;

    small_vector<ir_def_resolution>
    resolve (void);

    small_vector<ir_def_resolution>
    resolve_with (ir_link_set<ir_def_timeline>&& s);

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

    ir_component_ptr
    get_component (void) const noexcept;

  private:
    ir_component_ptr                     m_component;
    std::stack<ir_def_resolution_frame>  m_stack;
    std::vector<ir_def_resolution_stack> m_leaves;
  };

  class ir_def_resolution_frame
  {
  public:
    ir_def_resolution_frame (ir_block& join_block, ir_component_ptr comp);

    [[nodiscard]]
    bool
    is_joinable (void) const noexcept;

    [[nodiscard]]
    ir_link_set<ir_def_timeline>
    join (void);

    [[nodiscard]]
    ir_link_set<ir_def_timeline>
    join_with (small_vector<ir_def_resolution>&& c);

    ir_def_resolution_stack&
    add_substack (ir_block& leaf_block);

    ir_component_ptr
    get_component (void) const noexcept;

  private:
    nonnull_ptr<ir_block>   m_join_block;
    ir_def_resolution_stack m_node;
  };

}

#endif // OCTAVE_IR_IR_DEF_RESOLUTION_HPP

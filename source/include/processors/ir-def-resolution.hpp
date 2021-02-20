/** ir-def-resolution.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_DEF_RESOLUTION_HPP
#define OCTAVE_IR_IR_DEF_RESOLUTION_HPP

#include "utilities/ir-link-set.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>
#include <gch/small_vector.hpp>

#include <stack>
#include <vector>

namespace gch
{

  class ir_block;
  class ir_def;
  class ir_def_timeline;
  class ir_use_timeline;
  class ir_def_resolution_stack;
  class ir_def_resolution_frame;
  class ir_variable;

  class ir_def_resolution
  {
  public:
    ir_def_resolution (ir_block& b, const ir_link_set<ir_def_timeline>& s)
      : m_leaf_block (b),
        m_timelines (s)
    { }

    ir_def_resolution (ir_block& b, ir_link_set<ir_def_timeline>&& s)
      : m_leaf_block (b),
        m_timelines (std::move (s))
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
    class leading_block_resolution
    {
    public:
      explicit
      leading_block_resolution (ir_block& block);

      leading_block_resolution (ir_block& block, ir_link_set<ir_def_timeline>&& resolution);

      ir_block&
      get_block (void) const noexcept;

      bool
      has_resolution (void) const noexcept;

      const ir_link_set<ir_def_timeline>&
      get_resolution (void) const noexcept;

      const std::optional<ir_link_set<ir_def_timeline>>&
      maybe_get_resolution (void) const noexcept;

    private:
      nonnull_ptr<ir_block>                       m_block;
      std::optional<ir_link_set<ir_def_timeline>> m_resolution;
    };

    using stack_backing_type = std::vector<ir_def_resolution_frame>;

    ir_def_resolution_stack            (void)                               = delete;
    ir_def_resolution_stack            (const ir_def_resolution_stack&)     = default;
    ir_def_resolution_stack            (ir_def_resolution_stack&&) noexcept = default;
    ir_def_resolution_stack& operator= (const ir_def_resolution_stack&)     = default;
    ir_def_resolution_stack& operator= (ir_def_resolution_stack&&) noexcept = default;
    ~ir_def_resolution_stack           (void)                               = default;

    explicit
    ir_def_resolution_stack (ir_variable& var);

    ir_def_resolution_stack (ir_variable& var, ir_block& block);
    ir_def_resolution_stack (ir_variable& var, ir_block& block, ir_link_set<ir_def_timeline>&& s);

    [[nodiscard]]
    bool
    is_resolvable (void) const noexcept;

    ir_def_resolution_frame&
    push_frame (ir_block& join_block, ir_def_resolution_stack&& substack);

    ir_def_resolution_frame&
    push_frame (ir_block& join_block, ir_variable& var);

    [[nodiscard]]
    ir_def_resolution_frame&
    top (void);

    [[nodiscard]]
    const ir_def_resolution_frame&
    top (void) const;

    [[nodiscard]]
    bool
    has_frames (void) const noexcept;

    ir_def_resolution_stack&
    add_leaf (ir_def_resolution_stack&& leaf_stack);

    ir_def_resolution_stack&
    add_leaf (void);

    ir_def_resolution_stack&
    add_leaf (ir_block& block);

    ir_def_resolution_stack&
    add_leaf (ir_block& block, ir_def_timeline& dt);

    [[nodiscard]]
    bool
    has_leaves (void) const noexcept;

    [[nodiscard]]
    ir_component&
    get_component (void) const noexcept;

    [[nodiscard]]
    ir_variable&
    get_variable (void) const noexcept;

    [[nodiscard]]
    optional_ref<ir_block>
    maybe_cast_block (void) const noexcept;

    small_vector<ir_def_resolution>
    resolve (void);

    small_vector<ir_def_resolution>
    resolve_with (ir_link_set<ir_def_timeline> s);

    void
    set_block_resolution (ir_block& block, std::nullopt_t);

    void
    set_block_resolution (ir_block& block, ir_link_set<ir_def_timeline>&& s);

    void
    set_block_resolution (leading_block_resolution&& res);

    [[nodiscard]]
    bool
    holds_block (void) const noexcept;

    [[nodiscard]]
    leading_block_resolution
    get_block_resolution (void) const noexcept;

    [[nodiscard]]
    std::optional<leading_block_resolution>
    maybe_get_block_resolution (void) const noexcept;

  private:
    nonnull_ptr<ir_variable>                                m_variable;
    std::optional<leading_block_resolution>                 m_block_resolution;
    std::stack<ir_def_resolution_frame, stack_backing_type> m_stack;
    std::vector<ir_def_resolution_stack>                    m_leaves;
  };

  class ir_def_resolution_frame
  {
  public:
    ir_def_resolution_frame (ir_block& join_block, ir_def_resolution_stack&& substack);
    ir_def_resolution_frame (ir_block& join_block, ir_variable& var);

    [[nodiscard]]
    bool
    is_joinable (void) const noexcept;

    [[nodiscard]]
    ir_link_set<ir_def_timeline>
    join (void);

    [[nodiscard]]
    ir_link_set<ir_def_timeline>
    join_with (ir_link_set<ir_def_timeline>&& s);

    [[nodiscard]]
    ir_block&
    get_join_block (void) const noexcept;

    [[nodiscard]]
    ir_variable&
    get_variable (void) const noexcept;

  private:
    nonnull_ptr<ir_block>   m_join_block;
    ir_def_resolution_stack m_substack;
  };

}

#endif // OCTAVE_IR_IR_DEF_RESOLUTION_HPP

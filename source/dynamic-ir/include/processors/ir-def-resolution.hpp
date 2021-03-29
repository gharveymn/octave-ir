/** ir-def-resolution.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_DEF_RESOLUTION_HPP
#define OCTAVE_IR_IR_DEF_RESOLUTION_HPP

#include "gch/octave-ir-utilities/ir-link-set.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>
#include <gch/small_vector.hpp>

#include <stack>
#include <optional>
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
    ir_def_resolution            (void)                         = delete;
    ir_def_resolution            (const ir_def_resolution&)     = default;
    ir_def_resolution            (ir_def_resolution&&) noexcept = default;
    ir_def_resolution& operator= (const ir_def_resolution&)     = default;
    ir_def_resolution& operator= (ir_def_resolution&&) noexcept = default;
    ~ir_def_resolution           (void)                         = default;

    ir_def_resolution (ir_block& b, optional_ref<ir_def_timeline> dt);

    [[nodiscard]]
    ir_block&
    get_leaf_block (void) const noexcept;

    [[nodiscard]]
    ir_def_timeline&
    get_timeline (void) const noexcept;

    [[nodiscard]]
    bool
    has_timeline (void) const noexcept;

    [[nodiscard]]
    optional_ref<ir_def_timeline>
    maybe_get_timeline (void) const;

  private:
    nonnull_ptr<ir_block>         m_leaf_block;
    optional_ref<ir_def_timeline> m_timeline;
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

      [[nodiscard]]
      ir_block&
      get_block (void) const noexcept;

      [[nodiscard]]
      bool
      has_resolution (void) const noexcept;

      [[nodiscard]]
      optional_ref<ir_def_timeline>
      get_resolution (void) const noexcept;

      [[nodiscard]]
      const std::optional<optional_ref<ir_def_timeline>>&
      maybe_get_resolution (void) const noexcept;

    private:
      nonnull_ptr<ir_block>                        m_block;
      std::optional<optional_ref<ir_def_timeline>> m_resolution;
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
    maybe_cast_block (void) const noexcept; // FIXME: terrible name

    small_vector<ir_def_resolution>
    resolve (void);

    small_vector<ir_def_resolution>
    resolve_with (optional_ref<ir_def_timeline> dt);

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
    optional_ref<ir_def_timeline>
    join (void);

    [[nodiscard]]
    optional_ref<ir_def_timeline>
    join_with (optional_ref<ir_def_timeline> dt);

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

  class ir_def_resolution_build_result
  {
  public:
    ir_def_resolution_build_result            (void)                                      = delete;
    ir_def_resolution_build_result            (const ir_def_resolution_build_result&)     = delete;
    ir_def_resolution_build_result            (ir_def_resolution_build_result&&) noexcept = default;
    ir_def_resolution_build_result& operator= (const ir_def_resolution_build_result&)     = delete;
    ir_def_resolution_build_result& operator= (ir_def_resolution_build_result&&) noexcept = default;
    ~ir_def_resolution_build_result           (void)                                      = default;

    enum class join : bool
    {
      yes = true,
      no  = false,
    };

    enum class resolvable : bool
    {
      yes = true,
      no  = false,
    };

    ir_def_resolution_build_result (ir_variable& var, join j, resolvable r);

    ir_def_resolution_build_result (ir_def_resolution_stack&& s, join j, resolvable r);

    [[nodiscard]]
    join
    get_join_state (void) const noexcept;

    [[nodiscard]]
    resolvable
    get_resolvable_state (void) const noexcept;

    // returning rvalue reference just for semantic reasons
    [[nodiscard]]
    ir_def_resolution_stack&&
    release_stack (void) noexcept;

    [[nodiscard]]
    bool
    needs_join (void) const noexcept;

    [[nodiscard]]
    bool
    is_resolvable (void) const noexcept;

  private:
    ir_def_resolution_stack m_stack;
    join                    m_join;
    resolvable              m_resolvable;
  };

  [[nodiscard]]
  ir_def_resolution_stack
  build_def_resolution_stack (ir_block& block, ir_variable& var);

  ir_use_timeline&
  join_at (ir_block& block, ir_variable& var);

  ir_use_timeline&
  join_at (ir_def_timeline& dt);

}

#endif // OCTAVE_IR_IR_DEF_RESOLUTION_HPP

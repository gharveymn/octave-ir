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
    using stack_backing_type = std::vector<ir_def_resolution_frame>;

    ir_def_resolution_stack            (void)                               = delete;
    ir_def_resolution_stack            (const ir_def_resolution_stack&)     = default;
    ir_def_resolution_stack            (ir_def_resolution_stack&&) noexcept = default;
    ir_def_resolution_stack& operator= (const ir_def_resolution_stack&)     = default;
    ir_def_resolution_stack& operator= (ir_def_resolution_stack&&) noexcept = default;
    ~ir_def_resolution_stack           (void)                               = default;

    explicit
    ir_def_resolution_stack (ir_component& c);

    [[nodiscard]]
    bool
    is_resolvable (void) const noexcept;

    ir_def_resolution_frame&
    push (ir_block& join_block, ir_component& c);

    [[nodiscard]]
    ir_def_resolution_frame&
    top (void);

    [[nodiscard]]
    const ir_def_resolution_frame&
    top (void) const;

    template <typename ...Args>
    ir_def_resolution_stack&
    add_leaf (Args&&... args)
    {
      m_leaves.emplace_back (std::forward<Args> (args)...);
    }

    [[nodiscard]]
    ir_component&
    get_component (void) const noexcept;

    [[nodiscard]]
    bool
    holds_singleton_block (void) const noexcept;

    small_vector<ir_def_resolution>
    resolve (void);

    small_vector<ir_def_resolution>
    resolve_with (ir_link_set<ir_def_timeline> s);

  private:
    nonnull_ptr<ir_component>                               m_component;
    std::stack<ir_def_resolution_frame, stack_backing_type> m_stack;
    std::vector<ir_def_resolution_stack>                    m_leaves;
  };

  class ir_def_resolution_frame
  {
  public:
    ir_def_resolution_frame (ir_block& join_block, ir_component& c);

    [[nodiscard]]
    bool
    is_joinable (void) const noexcept;

    [[nodiscard]]
    ir_link_set<ir_def_timeline>
    join (void);

    [[nodiscard]]
    ir_link_set<ir_def_timeline>
    join_with (ir_link_set<ir_def_timeline>&& s);

    ir_component&
    get_subcomponent (void) const noexcept;

    ir_block&
    get_block (void) const noexcept;

  private:
    nonnull_ptr<ir_block>   m_join_block;
    ir_def_resolution_stack m_substack;
  };

  // if returns nullopt then no resolution in descent
  // else if not resolvable we need to continue
  // else resolvable we collapse
  std::optional<ir_def_resolution_stack>
  build_resolution (ir_component& c);

  class ir_def_resolution_build_descender
  {

  };

  class ir_def_resolution_build_ascender
  {
  public:
    bool
    dispatch_child (ir_component& join_component, ir_component& c)
    {
      if (std::optional<ir_def_resolution_stack> child { build_resolution (c) })
      {
        if (child->is_resolvable ())

      }
      return false;
    }

    void
    ascend (void);

  private:
    ir_def_resolution_stack m_stack;
  };

  class ir_def_resolution_builder
  {
  public:
    bool
    dispatch_child (ir_component& join_component, ir_component& c)
    {
      if (std::optional<ir_def_resolution_stack> child { build_resolution (c) })
      {
        if (child->is_resolvable ())

      }
      return false;
    }

    void
    ascend (void);

  private:
    ir_def_resolution_stack m_stack;
  };

}

#endif // OCTAVE_IR_IR_DEF_RESOLUTION_HPP

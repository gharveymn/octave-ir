/** ir-def-resolution.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_DEF_RESOLUTION_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_DEF_RESOLUTION_HPP

#include "ir-link-set.hpp"
#include "ir-utility.hpp"

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

  extern template class ir_link_set<ir_def_timeline>;
  extern template class ir_link_set<const ir_def_timeline>;

  class ir_def_resolution
  {
  public:
    ir_def_resolution            (void)                         = delete;
    ir_def_resolution            (const ir_def_resolution&)     = default;
    ir_def_resolution            (ir_def_resolution&&) noexcept = default;
    ir_def_resolution& operator= (const ir_def_resolution&)     = default;
    ir_def_resolution& operator= (ir_def_resolution&&) noexcept = default;
    ~ir_def_resolution           (void)                         = default;

    ir_def_resolution (const ir_block& b, optional_cref<ir_def_timeline> dt);

    [[nodiscard]]
    const ir_block&
    get_leaf_block (void) const noexcept;

    [[nodiscard]]
    const ir_def_timeline&
    get_timeline (void) const;

    [[nodiscard]]
    bool
    has_timeline (void) const noexcept;

    [[nodiscard]]
    optional_cref<ir_def_timeline>
    maybe_get_timeline (void) const noexcept;

  private:
    nonnull_cptr<ir_block>         m_leaf_block;
    optional_cref<ir_def_timeline> m_timeline;
  };

  class ir_def_resolution_stack
  {
  public:
    class leading_block_resolution
    {
    public:
      explicit
      leading_block_resolution (const ir_block& block);

      leading_block_resolution (const ir_block& block,
                                ir_link_set<const ir_def_timeline>&& resolution);

      [[nodiscard]]
      const ir_block&
      get_block (void) const noexcept;

      [[nodiscard]]
      bool
      has_resolution (void) const noexcept;

      [[nodiscard]]
      optional_cref<ir_def_timeline>
      get_resolution (void) const noexcept;

      [[nodiscard]]
      const std::optional<optional_cref<ir_def_timeline>>&
      maybe_get_resolution (void) const noexcept;

    private:
      nonnull_cptr<ir_block>                        m_block;
      std::optional<optional_cref<ir_def_timeline>> m_resolution;
    };

    using stack_backing_type = std::vector<ir_def_resolution_frame>;

    using leaves_container_type          = std::vector<ir_def_resolution_stack>;
    using leaves_value_type              = leaves_container_type::value_type;
    using leaves_allocator_type          = leaves_container_type::allocator_type;
    using leaves_size_type               = leaves_container_type::size_type;
    using leaves_difference_type         = leaves_container_type::difference_type;
    using leaves_reference               = leaves_container_type::reference;
    using leaves_const_reference         = leaves_container_type::const_reference;
    using leaves_pointer                 = leaves_container_type::pointer;
    using leaves_const_pointer           = leaves_container_type::const_pointer;

    using leaves_iterator                = leaves_container_type::iterator;
    using leaves_const_iterator          = leaves_container_type::const_iterator;
    using leaves_reverse_iterator        = leaves_container_type::reverse_iterator;
    using leaves_const_reverse_iterator  = leaves_container_type::const_reverse_iterator;

    using leaves_val_t   = leaves_value_type;
    using leaves_alloc_t = leaves_allocator_type;
    using leaves_size_ty = leaves_size_type;
    using leaves_diff_ty = leaves_difference_type;
    using leaves_ref     = leaves_reference;
    using leaves_cref    = leaves_const_reference;
    using leaves_ptr     = leaves_pointer;
    using leaves_cptr    = leaves_const_pointer;

    using leaves_iter    = leaves_iterator;
    using leaves_citer   = leaves_const_iterator;
    using leaves_riter   = leaves_reverse_iterator;
    using leaves_criter  = leaves_const_reverse_iterator;

    ir_def_resolution_stack            (void)                               = delete;
    ir_def_resolution_stack            (const ir_def_resolution_stack&)     = default;
    ir_def_resolution_stack            (ir_def_resolution_stack&&) noexcept = default;
    ir_def_resolution_stack& operator= (const ir_def_resolution_stack&)     = default;
    ir_def_resolution_stack& operator= (ir_def_resolution_stack&&) noexcept = default;
    ~ir_def_resolution_stack           (void)                               = default;

    explicit
    ir_def_resolution_stack (const ir_variable& var);

    ir_def_resolution_stack (const ir_variable& var, const ir_block& block);

    ir_def_resolution_stack (const ir_variable& var, const ir_block& block,
                             ir_link_set<const ir_def_timeline>&& s);

    [[nodiscard]]
    bool
    is_resolvable (void) const noexcept;

    ir_def_resolution_frame&
    push_frame (const ir_block& join_block, ir_def_resolution_stack&& substack);

    ir_def_resolution_frame&
    push_frame (const ir_block& join_block, const ir_variable& var);

    [[nodiscard]]
    ir_def_resolution_frame&
    top (void);

    [[nodiscard]]
    const ir_def_resolution_frame&
    top (void) const;

    void
    pop (void);

    [[nodiscard]]
    std::size_t
    num_frames (void) const noexcept;

    [[nodiscard]]
    bool
    has_frames (void) const noexcept;

    ir_def_resolution_stack&
    add_leaf (ir_def_resolution_stack&& leaf_stack);

    ir_def_resolution_stack&
    add_leaf (void);

    ir_def_resolution_stack&
    add_leaf (const ir_block& block);

    ir_def_resolution_stack&
    add_leaf (const ir_block& block, const ir_def_timeline& dt);

    [[nodiscard]]
    const ir_variable&
    get_variable (void) const noexcept;

    void
    set_block_resolution (const ir_block& block, std::nullopt_t);

    void
    set_block_resolution (const ir_block& block, ir_link_set<const ir_def_timeline>&& s);

    void
    set_block_resolution (leading_block_resolution&& res);

    [[nodiscard]]
    bool
    holds_block (void) const noexcept;

    [[nodiscard]]
    leading_block_resolution
    get_block_resolution (void) const noexcept;

    [[nodiscard]]
    const std::optional<leading_block_resolution>&
    maybe_get_block_resolution (void) const noexcept;

    [[nodiscard]]
    leaves_iter
    leaves_begin (void) noexcept;

    [[nodiscard]]
    leaves_citer
    leaves_begin (void) const noexcept;

    [[nodiscard]]
    leaves_citer
    leaves_cbegin (void) const noexcept;

    [[nodiscard]]
    leaves_iter
    leaves_end (void) noexcept;

    [[nodiscard]]
    leaves_citer
    leaves_end (void) const noexcept;

    [[nodiscard]]
    leaves_citer
    leaves_cend (void) const noexcept;

    [[nodiscard]]
    leaves_riter
    leaves_rbegin (void) noexcept;

    [[nodiscard]]
    leaves_criter
    leaves_rbegin (void) const noexcept;

    [[nodiscard]]
    leaves_criter
    leaves_crbegin (void) const noexcept;

    [[nodiscard]]
    leaves_riter
    leaves_rend (void) noexcept;

    [[nodiscard]]
    leaves_criter
    leaves_rend (void) const noexcept;

    [[nodiscard]]
    leaves_criter
    leaves_crend (void) const noexcept;

    [[nodiscard]]
    leaves_ref
    leaves_front (void);

    [[nodiscard]]
    leaves_cref
    leaves_front (void) const;

    [[nodiscard]]
    leaves_ref
    leaves_back (void);

    [[nodiscard]]
    leaves_cref
    leaves_back (void) const;

    [[nodiscard]]
    leaves_size_ty
    num_leaves (void) const noexcept;

    [[nodiscard]]
    bool
    has_leaves (void) const noexcept;

    void
    dominate_with (const ir_block& join_block, ir_def_resolution_stack&& dominator);

  private:
    nonnull_cptr<ir_variable>                               m_variable;
    std::optional<leading_block_resolution>                 m_block_resolution;
    std::stack<ir_def_resolution_frame, stack_backing_type> m_stack;
    std::vector<ir_def_resolution_stack>                    m_leaves;
  };

  class ir_def_resolution_frame
  {
  public:
    ir_def_resolution_frame (const ir_block& join_block, ir_def_resolution_stack&& substack);

    ir_def_resolution_frame (const ir_block& join_block, const ir_variable& var);

    [[nodiscard]]
    bool
    is_joinable (void) const noexcept;

    [[nodiscard]]
    const ir_block&
    get_join_block (void) const noexcept;

    [[nodiscard]]
    ir_def_resolution_stack&
    get_substack (void) noexcept;

    [[nodiscard]]
    const ir_def_resolution_stack&
    get_substack (void) const noexcept;

    [[nodiscard]]
    const ir_variable&
    get_variable (void) const noexcept;

  private:
    nonnull_cptr<ir_block>  m_join_block;
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

    struct join_type
      : transparent_named_type<join_type, bool>
    {
      using transparent_named_type<join_type, bool>::transparent_named_type;
    };

    template <bool B>
    static constexpr join_type join { B };

    struct resolvable_type
      : transparent_named_type<resolvable_type, bool>
    {
      using transparent_named_type<resolvable_type, bool>::transparent_named_type;
    };

    template <bool B>
    static constexpr resolvable_type resolvable { B };

    ir_def_resolution_build_result (const ir_variable& var, join_type j, resolvable_type r);

    ir_def_resolution_build_result (ir_def_resolution_stack&& s, join_type j, resolvable_type r);

    [[nodiscard]]
    const ir_variable&
    get_variable (void) const noexcept;

    [[nodiscard]]
    const ir_def_resolution_stack&
    get_stack (void) const noexcept;

    [[nodiscard]]
    ir_def_resolution_stack&
    get_stack (void) noexcept;

    // returning rvalue reference just for semantic reasons
    [[nodiscard]]
    ir_def_resolution_stack&&
    release_stack (void) noexcept;

    [[nodiscard]]
    join_type
    needs_join (void) const noexcept;

    [[nodiscard]]
    resolvable_type
    is_resolvable (void) const noexcept;

    void
    set_join (join_type j) noexcept;

    void
    set_resolvable (resolvable_type r) noexcept;

    ir_def_resolution_build_result&
    operator&= (ir_def_resolution_build_result::join_type j)
    {
      m_join &= j;
      return *this;
    }

    ir_def_resolution_build_result&
    operator|= (ir_def_resolution_build_result::join_type j)
    {
      m_join |= j;
      return *this;
    }

    ir_def_resolution_build_result&
    operator&= (ir_def_resolution_build_result::resolvable_type r)
    {
      m_resolvable &= r;
      return *this;
    }

    ir_def_resolution_build_result&
    operator|= (ir_def_resolution_build_result::resolvable_type r)
    {
      m_resolvable |= r;
      return *this;
    }

  private:
    ir_def_resolution_stack m_stack;
    join_type               m_join;
    resolvable_type         m_resolvable;
  };

  [[nodiscard]]
  ir_def_resolution_stack
  build_def_resolution_stack (const ir_block& block, const ir_variable& var);

  small_vector<ir_def_resolution>
  resolve (ir_def_resolution_stack& stack);

  small_vector<ir_def_resolution>
  resolve_with (ir_def_resolution_stack& stack, optional_ref<ir_def_timeline> dt);

  optional_ref<ir_def_timeline>
  join (ir_def_resolution_frame& frame);

  optional_ref<ir_def_timeline>
  join_with (ir_def_resolution_frame& frame, optional_ref<ir_def_timeline> dt);

  optional_ref<ir_def_timeline>
  join_at (ir_block& block, ir_variable& var, const small_vector<ir_def_resolution>& incoming);

  ir_use_timeline&
  join_at (ir_def_timeline& dt);

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_DEF_RESOLUTION_HPP

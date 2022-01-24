/** ir-def-resolution.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-block.hpp"
#include "ir-component.hpp"
#include "ir-structure.hpp"
#include "ir-function.hpp"
#include "ir-component-fork.hpp"
#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-def-resolution.hpp"
#include "ir-optional-util.hpp"
#include "structure/inspectors/ir-ascending-def-resolution-builder.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

namespace gch
{

  [[nodiscard]]
  static
  bool
  check_matching_incoming_blocks (const ir_block& join_block,
                                  const small_vector<ir_def_resolution>& c)
  {
    ir_link_set<ir_block> lhs = get_predecessors (join_block);
    ir_link_set<const ir_block> rhs;

    std::for_each (c.begin (), c.end (), [&rhs](const ir_def_resolution& r) {
      rhs.emplace (r.get_leaf_block ());
    });

    bool res = std::equal (lhs.begin (), lhs.end (), rhs.begin (), rhs.end ());
    assert (res);
    return res;
  }

  static
  void
  assert_matching_incoming_blocks (ir_block& join_block, const small_vector<ir_def_resolution>& c)
  {
    assert (check_matching_incoming_blocks (join_block, c));
  }

  //
  // ir_def_resolution
  //

  ir_def_resolution::
  ir_def_resolution (const ir_block& b, optional_cref<ir_def_timeline> dt)
    : m_leaf_block (b),
      m_timeline (dt)
  { }

  const ir_block&
  ir_def_resolution::
  get_leaf_block (void) const noexcept
  {
    return *m_leaf_block;
  }

  const ir_def_timeline&
  ir_def_resolution::
  get_timeline (void) const
  {
    return *m_timeline;
  }

  bool
  ir_def_resolution::
  has_timeline (void) const noexcept
  {
    return m_timeline.has_value ();
  }

  optional_cref<ir_def_timeline>
  ir_def_resolution::
  maybe_get_timeline (void) const noexcept
  {
    return m_timeline;
  }

  //
  // ir_def_resolution_stack
  //

  ir_def_resolution_stack::leading_block_resolution::
  leading_block_resolution (const ir_block& block)
    : m_block (block)
  { }

  ir_def_resolution_stack::leading_block_resolution::
  leading_block_resolution (const ir_block& block, ir_link_set<const ir_def_timeline>&& resolution)
    : m_block (block),
      m_resolution (resolution.empty () ? optional_ref<ir_def_timeline> { }
                                        : optional_ref { *resolution.front () })
  { }

  const ir_block&
  ir_def_resolution_stack::leading_block_resolution::
  get_block (void) const noexcept
  {
    return *m_block;
  }

  bool
  ir_def_resolution_stack::leading_block_resolution::
  has_resolution (void) const noexcept
  {
    return m_resolution.has_value ();
  }

  optional_cref<ir_def_timeline>
  ir_def_resolution_stack::leading_block_resolution::
  get_resolution (void) const noexcept
  {
    return *m_resolution;
  }

  const std::optional<optional_cref<ir_def_timeline>>&
  ir_def_resolution_stack::leading_block_resolution::
  maybe_get_resolution (void) const noexcept
  {
    return m_resolution;
  }

  ir_def_resolution_stack::
  ir_def_resolution_stack (const ir_variable& var)
    : m_variable (var)
  { }

  ir_def_resolution_stack::
  ir_def_resolution_stack (const ir_variable& var, const ir_block& block)
    : m_variable         (var),
      m_block_resolution (std::in_place, block)
  { }

  ir_def_resolution_stack::
  ir_def_resolution_stack (const ir_variable& var, const ir_block& block,
                           ir_link_set<const ir_def_timeline>&& s)
    : m_variable         (var),
      m_block_resolution (std::in_place, block, std::move (s))
  { }

  bool
  ir_def_resolution_stack::
  is_resolvable (void) const noexcept
  {
    return (holds_block () && m_block_resolution->has_resolution ())
       ||  (! m_stack.empty () && m_stack.top ().is_joinable ())
       ||  std::all_of (m_leaves.begin (), m_leaves.end (),
                        std::mem_fn (&ir_def_resolution_stack::is_resolvable));
  }

  ir_def_resolution_frame&
  ir_def_resolution_stack::
  push_frame (const ir_block& join_block, const ir_variable& var)
  {
    return m_stack.emplace (join_block, var);
  }

  ir_def_resolution_frame&
  ir_def_resolution_stack::
  push_frame (const ir_block& join_block, ir_def_resolution_stack&& substack)
  {
    return m_stack.emplace (join_block, std::move (substack));
  }

  ir_def_resolution_frame&
  ir_def_resolution_stack::
  top (void)
  {
    return m_stack.top ();
  }

  const ir_def_resolution_frame&
  ir_def_resolution_stack::
  top (void) const
  {
    return m_stack.top ();
  }

  void
  ir_def_resolution_stack::
  pop (void)
  {
    m_stack.pop ();
  }

  bool
  ir_def_resolution_stack::
  has_frames (void) const noexcept
  {
    return ! m_stack.empty ();
  }

  ir_def_resolution_stack&
  ir_def_resolution_stack::
  add_leaf (ir_def_resolution_stack&& leaf_stack)
  {
    return m_leaves.emplace_back (std::move (leaf_stack));
  }

  ir_def_resolution_stack&
  ir_def_resolution_stack::
  add_leaf (void)
  {
    return m_leaves.emplace_back (get_variable ());
  }

  ir_def_resolution_stack&
  ir_def_resolution_stack::
  add_leaf (const ir_block& block)
  {
    return m_leaves.emplace_back (get_variable (), block);
  }

  ir_def_resolution_stack&
  ir_def_resolution_stack::
  add_leaf (const ir_block& block, const ir_def_timeline& dt)
  {
    return m_leaves.emplace_back (get_variable (), block, ir_link_set { dt });
  }

  const ir_variable&
  ir_def_resolution_stack::
  get_variable (void) const noexcept
  {
    return *m_variable;
  }

  void
  ir_def_resolution_stack::
  set_block_resolution (const ir_block& block, std::nullopt_t)
  {
    m_block_resolution.emplace (block);
  }

  void
  ir_def_resolution_stack::
  set_block_resolution (const ir_block& block, ir_link_set<const ir_def_timeline>&& s)
  {
    m_block_resolution.emplace (block, std::move (s));
  }

  void
  ir_def_resolution_stack::
  set_block_resolution (leading_block_resolution&& res)
  {
    m_block_resolution = std::move (res);
  }

  bool
  ir_def_resolution_stack::
  holds_block (void) const noexcept
  {
    return m_block_resolution.has_value ();
  }

  auto
  ir_def_resolution_stack::
  get_block_resolution (void) const noexcept
    -> leading_block_resolution
  {
    return *m_block_resolution;
  }

  auto
  ir_def_resolution_stack::
  maybe_get_block_resolution (void) const noexcept
    -> const std::optional<leading_block_resolution>&
  {
    return m_block_resolution;
  }

  auto
  ir_def_resolution_stack::
  leaves_begin (void) noexcept
    -> leaves_iter
  {
    return m_leaves.begin ();
  }

  auto
  ir_def_resolution_stack::
  leaves_begin (void) const noexcept
    -> leaves_citer
  {
    return as_mutable (*this).leaves_begin ();
  }

  auto
  ir_def_resolution_stack::
  leaves_cbegin (void) const noexcept
    -> leaves_citer
  {
    return leaves_begin ();
  }

  auto
  ir_def_resolution_stack::
  leaves_end (void) noexcept
    -> leaves_iter
  {
    return m_leaves.end ();
  }

  auto
  ir_def_resolution_stack::
  leaves_end (void) const noexcept
    -> leaves_citer
  {
    return as_mutable (*this).leaves_end ();
  }

  auto
  ir_def_resolution_stack::
  leaves_cend (void) const noexcept
    -> leaves_citer
  {
    return leaves_end ();
  }

  auto
  ir_def_resolution_stack::
  leaves_rbegin (void) noexcept
    -> leaves_riter
  {
    return m_leaves.rbegin ();
  }

  auto
  ir_def_resolution_stack::
  leaves_rbegin (void) const noexcept
    -> leaves_criter
  {
    return as_mutable (*this).leaves_rbegin ();
  }

  auto
  ir_def_resolution_stack::
  leaves_crbegin (void) const noexcept
    -> leaves_criter
  {
    return leaves_rbegin ();
  }

  auto
  ir_def_resolution_stack::
  leaves_rend (void) noexcept
    -> leaves_riter
  {
    return m_leaves.rend ();
  }

  auto
  ir_def_resolution_stack::
  leaves_rend (void) const noexcept
    -> leaves_criter
  {
    return as_mutable (*this).leaves_rend ();
  }

  auto
  ir_def_resolution_stack::
  leaves_crend (void) const noexcept
    -> leaves_criter
  {
    return leaves_rend ();
  }

  auto
  ir_def_resolution_stack::
  leaves_front (void)
    -> leaves_ref
  {
    return *leaves_begin ();
  }

  auto
  ir_def_resolution_stack::
  leaves_front (void) const
    -> leaves_cref
  {
    return as_mutable (*this).leaves_front ();
  }

  auto
  ir_def_resolution_stack::
  leaves_back (void)
    -> leaves_ref
  {
    return *leaves_rbegin ();
  }

  auto
  ir_def_resolution_stack::
  leaves_back (void) const
    -> leaves_cref
  {
    return as_mutable (*this).leaves_back ();
  }

  auto
  ir_def_resolution_stack::
  num_leaves (void) const noexcept
    -> leaves_size_ty
  {
    return m_leaves.size ();
  }

  bool
  ir_def_resolution_stack::
  has_leaves (void) const noexcept
  {
    return ! m_leaves.empty ();
  }

  //
  // ir_def_resolution_frame
  //

  ir_def_resolution_frame::
  ir_def_resolution_frame (const ir_block& join_block, ir_def_resolution_stack&& substack)
    : m_join_block (join_block),
      m_substack   (std::move (substack))
  { }

  ir_def_resolution_frame::
  ir_def_resolution_frame (const ir_block& join_block, const ir_variable& var)
    : m_join_block (join_block),
      m_substack   (var)
  { }

  bool
  ir_def_resolution_frame::
  is_joinable (void) const noexcept
  {
    return m_substack.is_resolvable ();
  }

  const ir_block&
  ir_def_resolution_frame::
  get_join_block (void) const noexcept
  {
    return *m_join_block;
  }

  ir_def_resolution_stack&
  ir_def_resolution_frame::
  get_substack (void) noexcept
  {
    return m_substack;
  }

  const ir_def_resolution_stack&
  ir_def_resolution_frame::
  get_substack (void) const noexcept
  {
    return m_substack;
  }

  const ir_variable&
  ir_def_resolution_frame::
  get_variable (void) const noexcept
  {
    return get_substack ().get_variable ();
  }

  //
  // ir_def_resolution_build_result
  //

  ir_def_resolution_build_result::
  ir_def_resolution_build_result (const ir_variable& var, join j, resolvable r)
    : m_stack      (var),
      m_join       (j),
      m_resolvable (r)
  { }

  ir_def_resolution_build_result::
  ir_def_resolution_build_result (ir_def_resolution_stack&& s, join j, resolvable r)
    : m_stack      (std::move (s)),
      m_join       (j),
      m_resolvable (r)
  { }

  auto
  ir_def_resolution_build_result::
  get_join_state (void) const noexcept
  -> join
  {
    return m_join;
  }

  auto
  ir_def_resolution_build_result::
  get_resolvable_state (void) const noexcept
  -> resolvable
  {
    return m_resolvable;
  }

  ir_def_resolution_stack&&
  ir_def_resolution_build_result::
  release_stack (void) noexcept
  {
    return std::move (m_stack);
  }

  bool
  ir_def_resolution_build_result::
  needs_join (void) const noexcept
  {
    return get_join_state () == join::yes;
  }

  bool
  ir_def_resolution_build_result::
  is_resolvable (void) const noexcept
  {
    return get_resolvable_state () == resolvable::yes;
  }

  ir_def_resolution_stack
  build_def_resolution_stack (const ir_block& block, const ir_variable& var)
  {
    auto stack { ir_ascending_def_resolution_builder { block, var } ().release_stack () };

    // assert (! stack.has_leaves ());
    stack.add_leaf (block);

    return stack;
  }

  [[nodiscard]]
  optional_ref<ir_def_timeline>
  join_at (ir_block& join_block, ir_variable& var, const small_vector<ir_def_resolution>& incoming)
  {
    // assert_matching_incoming_blocks (join_block, incoming);
    if (incoming.empty ())
      return { };

    // Create a def-timeline at the join-block.
    ir_def_timeline& dt = join_block.get_def_timeline (var);

    // Note: In the case of loops we may be appending to a def-timeline which already exists.

    std::for_each (incoming.begin (), incoming.end (), [&dt](const ir_def_resolution& r) {
      dt.append_incoming (
        as_mutable (r.get_leaf_block ()),
        as_mutable (r.maybe_get_timeline ()));
    });

    // Q: Do we need to re-point references to the found predecessor timelines
    //    in subsequent blocks?
    //
    // Theorem.
    //   If a def is created here, then there are no references to the
    //   found predecessor def-timelines in succeeding incoming-nodes.
    // Proof.
    //   Suppose the opposite of the consequent. A def is created here if and
    //   only if the set of def-timelines found here is heterogeneous. But,
    //   incoming-nodes only track a homogeneous set of def-timelines which
    //   implies that a def is not created here. ////
    //
    // A: We don't.

    return optional_ref { dt };
  }

  small_vector<ir_def_resolution>
  resolve (ir_def_resolution_stack& stack)
  {
    // I think this is just a recursive version of the (resolve|join)_with algorithm.
    // Can't remember.

    assert (stack.is_resolvable () && "stack should be resolvable");

    if (const auto& block_res = stack.maybe_get_block_resolution ())
    {
      // collapse the stack with the seed
      if (const auto& res = block_res->maybe_get_resolution ())
        return { { block_res->get_block (), *res } };
      return { { block_res->get_block (), { } } };
    }
    else if (stack.has_frames ()) // collapse the stack
    {
      optional_ref curr_result { join (stack.top ()) };
      stack.pop ();
      return resolve_with (stack, curr_result);
    }
    else // the resolution is the aggregate of the leaf components
    {
      small_vector<ir_def_resolution> ret;
      std::for_each (stack.leaves_begin (), stack.leaves_end (), [&](auto& leaf_stack) {
        ret.append (resolve (leaf_stack));
      });

      return ret;
    }
  }

  small_vector<ir_def_resolution>
  resolve_with (ir_def_resolution_stack& stack, optional_ref<ir_def_timeline> dt)
  {
    if (const auto& block_res = stack.maybe_get_block_resolution ())
    {
      if (const auto& res = block_res->maybe_get_resolution ())
        return { { block_res->get_block (), *res } };
      return { { block_res->get_block (), dt } };
    }

    while (stack.has_frames ())
    {
      dt = join_with (stack.top (), dt);
      stack.pop ();
    }

    small_vector<ir_def_resolution> ret;
    std::for_each (stack.leaves_begin (), stack.leaves_end (), [&](auto& leaf_stack) {
      ret.append (resolve_with (leaf_stack, dt));
    });

    return ret;
  }

  optional_ref<ir_def_timeline>
  join (ir_def_resolution_frame& frame)
  {
    assert (frame.is_joinable () && "Frame should be joinable.");
    return join_at (
      as_mutable (frame.get_join_block ()),
      as_mutable (frame.get_variable ()),
      resolve (frame.get_substack ()));
  }

  optional_ref<ir_def_timeline>
  join_with (ir_def_resolution_frame& frame, optional_ref<ir_def_timeline> dt)
  {
    return join_at (
      as_mutable (frame.get_join_block ()),
      as_mutable (frame.get_variable ()),
      resolve_with (frame.get_substack (), dt));
  }

  ir_use_timeline&
  join_at (ir_def_timeline& dt)
  {
    assert (! dt.has_incoming_timeline ());

    ir_def_resolution_stack stack {
      build_def_resolution_stack (dt.get_block (), dt.get_variable ())
    };

    small_vector<ir_def_resolution> res = resolve_with (stack, { });
    
    // FIXME: Results in self reference.
    join_at (dt.get_block (), dt.get_variable (), res);

    // If no incoming timeline was found, we will generate an orphaned use-timeline.
    if (! dt.has_incoming_timeline ())
      return dt.create_orphaned_incoming_timeline ();
    return dt.get_incoming_timeline ();
  }

}

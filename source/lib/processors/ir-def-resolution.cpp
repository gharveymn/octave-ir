/** ir-def-resolution.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "components/ir-block.hpp"
#include "components/ir-component.hpp"
#include "components/ir-structure.hpp"
#include "components/ir-function.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "processors/ir-def-resolution.hpp"
#include "utilities/ir-optional-util.hpp"
#include "visitors/structure/mutators/ir-ascending-def-resolution-builder.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

namespace gch
{

  [[nodiscard]]
  bool
  check_all_same_nonnull_outgoing (const ir_link_set<ir_def_timeline>& v)
  {
    if (v.empty ())
      return true;

    if (optional_ref cmp { v.front ()->maybe_get_outgoing_timeline () })
      return std::all_of (std::next (v.begin ()), v.end (),
                          [cmp](nonnull_ptr<ir_def_timeline> dt)
                          {
                            return has_same_def (*cmp, dt->maybe_get_outgoing_timeline ());
                          });

    return false;
  }

  void
  assert_all_same_nonnull_outgoing (const ir_link_set<ir_def_timeline>& v)
  {
    assert (check_all_same_nonnull_outgoing (v)
        &&  "all timelines should have an outgoing timeline, "
            "and all such timelines should point to the same def");
  }

  [[nodiscard]]
  bool
  check_matching_incoming_blocks (ir_block& join_block, const small_vector<ir_def_resolution>& c)
  {
    ir_link_set<ir_block> lhs = get_predecessors (join_block);
    ir_link_set<ir_block> rhs;
    std::for_each (c.begin (), c.end (), [&rhs](const ir_def_resolution& r)
                                         { rhs.emplace (r.get_leaf_block ()); });
    return lhs == rhs;
  }

  void
  assert_matching_incoming_blocks (ir_block& join_block, const small_vector<ir_def_resolution>& c)
  {
    assert (check_matching_incoming_blocks (join_block, c));
  }

  [[nodiscard]]
  optional_ref<ir_def_timeline>
  join_at (ir_block& join_block, ir_variable& var, const small_vector<ir_def_resolution>& incoming)
  {
    assert_matching_incoming_blocks (join_block, incoming);
    if (incoming.empty ())
      return { };

    // create an incoming-timeline at the join-block
    ir_def_timeline& dt = join_block.get_def_timeline (var);

    // Note: In the case of loops we may be appending to an def-timeline which already exists.

    std::for_each (incoming.begin (), incoming.end (),
                   [&dt](const ir_def_resolution& r)
                   {
                     dt.append_incoming (r.get_leaf_block (), r.maybe_get_timeline ());
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

  //
  // ir_def_resolution_build_result
  //

  ir_def_resolution_build_result::
  ir_def_resolution_build_result (ir_variable& var, join j, resolvable r)
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

  //
  // ir_def_resolution
  //

  //
  // ir_def_resolution_stack
  //

  ir_def_resolution_stack::
  ir_def_resolution_stack (ir_variable& var)
    : m_variable (var)
  { }

  ir_def_resolution_stack::
  ir_def_resolution_stack (ir_variable& var, ir_block& block)
    : m_variable         (var),
      m_block_resolution (std::in_place, block)
  { }

  ir_def_resolution_stack::
  ir_def_resolution_stack (ir_variable& var, ir_block& block, ir_link_set<ir_def_timeline>&& s)
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
  push_frame (ir_block& join_block, ir_variable& var)
  {
    return m_stack.emplace (join_block, var);
  }

  ir_def_resolution_frame&
  ir_def_resolution_stack::
  push_frame (ir_block& join_block, ir_def_resolution_stack&& substack)
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

  ir_variable&
  ir_def_resolution_stack::
  get_variable (void) const noexcept
  {
    return *m_variable;
  }

  optional_ref<ir_block>
  ir_def_resolution_stack::
  maybe_cast_block (void) const noexcept
  {
    // FIXME: The condition that both the stack and leaves are empty
    //        is equivalent to a non-null dynamic cast, but less safe
    //        from a static analysis perspective. Switch to that
    //        condition when verified.
    optional_ref ret { maybe_cast<ir_block> (get_component ()) };
    assert ((! ret || (m_stack.empty () && m_leaves.empty ()))
        &&  "Invalid state. If the component is a block then the "
            "stack and leaves should be empty.");
    return ret;
  }

  void
  ir_def_resolution_stack::
  set_block_resolution (ir_block& block, std::nullopt_t)
  {
    m_block_resolution.emplace (block);
  }

  void
  ir_def_resolution_stack::
  set_block_resolution (ir_block& block, ir_link_set<ir_def_timeline>&& s)
  {
    m_block_resolution.emplace (block, std::move (s));
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
    -> std::optional<leading_block_resolution>
  {
    return m_block_resolution;
  }

  small_vector<ir_def_resolution>
  ir_def_resolution_stack::
  resolve (void)
  {
    assert (is_resolvable () && "stack should be resolvable");

    if (std::optional res { maybe_get_block_resolution () }) // collapse the stack with the seed
      return resolve_with (res->get_resolution ());
    else if (! m_stack.empty ()) // collapse the stack
    {
      optional_ref<ir_def_timeline> curr_result = m_stack.top ().join ();
      m_stack.pop ();
      return resolve_with (curr_result);
    }
    else // the resolution is the aggregate of the leaf components
    {
      small_vector<ir_def_resolution> ret;
      std::for_each (m_leaves.begin (), m_leaves.end (),
                     [&](auto& leaf_stack) { ret.append (leaf_stack.resolve ()); });
      return ret;
    }
  }

  small_vector<ir_def_resolution>
  ir_def_resolution_stack::
  resolve_with (optional_ref<ir_def_timeline> dt)
  {
    if (optional_ref block { maybe_cast_block () })
      return { { *block, dt } };

    while (! m_stack.empty ())
    {
      dt = top ().join_with (dt);
      m_stack.pop ();
    }

    small_vector<ir_def_resolution> ret;
    std::for_each (m_leaves.begin (), m_leaves.end (),
                   [&](auto& leaf_stack) { ret.append (leaf_stack.resolve_with (dt)); });

    return ret;
  }

  //
  // ir_def_resolution_frame
  //

  ir_def_resolution_frame::
  ir_def_resolution_frame (ir_block& join_block, ir_def_resolution_stack&& substack)
    : m_join_block (join_block),
      m_substack   (std::move (substack))
  { }

  ir_def_resolution_frame::
  ir_def_resolution_frame (ir_block& join_block, ir_variable& var)
    : m_join_block (join_block),
      m_substack   (var)
  { }

  bool
  ir_def_resolution_frame::
  is_joinable (void) const noexcept
  {
    return m_substack.is_resolvable ();
  }

  optional_ref<ir_def_timeline>
  ir_def_resolution_frame::
  join (void)
  {
    assert (is_joinable () && "Frame should be joinable.");
    return join_at (get_join_block (), get_variable (), m_substack.resolve ());
  }

  optional_ref<ir_def_timeline>
  ir_def_resolution_frame::
  join_with (optional_ref<ir_def_timeline> dt)
  {
    return join_at (get_join_block (), get_variable (), m_substack.resolve_with (dt));
  }

  ir_block&
  ir_def_resolution_frame::
  get_join_block (void) const noexcept
  {
    return *m_join_block;
  }

  ir_variable&
  ir_def_resolution_frame::
  get_variable (void) const noexcept
  {
    return m_substack.get_variable ();
  }

  ir_def_resolution_stack
  build_def_resolution_stack (ir_block& block, ir_variable& var)
  {
    auto stack { ir_ascending_def_resolution_builder { block, var } ().release_stack () };

    assert (! stack.has_leaves ());
    stack.add_leaf (block);

    return stack;
  }

  ir_use_timeline&
  join_at (ir_def_timeline& dt)
  {
    assert (! dt.has_incoming_timeline ());

    auto res { build_def_resolution_stack (dt.get_block (), dt.get_variable ()) };
    res.resolve_with ({ });

    assert (dt.has_incoming_timeline ());
    return dt.get_incoming_timeline ();
  }

}

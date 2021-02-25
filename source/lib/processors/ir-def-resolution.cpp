/** ir-def-resolution.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "components/ir-block.hpp"
#include "processors/ir-def-resolution.hpp"
#include "utilities/ir-optional-util.hpp"
#include "components/ir-component.hpp"
#include "components/ir-structure.hpp"
#include "components/ir-function.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"

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
  bool
  check_homogeneous (const ir_link_set<ir_def_timeline>& c)
  {
    if (c.empty ())
      return true;

    const ir_def& cmp = c.front ()->get_outgoing_def ();
    return std::all_of (c.begin (), c.end (), [&cmp](nonnull_ptr<ir_def_timeline> dt)
                                              {
                                                return &cmp == &dt->get_outgoing_def ();
                                              });
  }

  void
  assert_homogeneous (const ir_link_set<ir_def_timeline>& c)
  {
    assert (check_homogeneous (c)
        &&  "The def-timeline link-set should be homogeneous in the origin defs of elements.");
  }

  [[nodiscard]]
  ir_link_set<ir_def_timeline>
  join_at (ir_block& join_block, ir_variable& var, small_vector<ir_def_resolution>&& incoming)
  {
    assert_matching_incoming_blocks (join_block, incoming);

    if (incoming.empty ())
      return { };

    optional_ref<ir_def> first_def = incoming.front ().maybe_get_common_def ();
    auto found_hetero = std::find_if_not (std::next (incoming.begin ()), incoming.end (),
                                          [first_def](const ir_def_resolution& r)
                                          {
                                            return first_def == r.maybe_get_common_def ();
                                          });

    // check if all the incoming timelines have the same parent def
    // if they do we can skip creation of an incoming-timeline in the current block
    if (found_hetero == incoming.end ())
    {
      // if homogeneous and we can skip creation of the incoming-timeline
      // and forward the timelines
      return std::accumulate (std::next (incoming.begin ()), incoming.end (),
                              incoming.front ().get_timelines (),
                              [](auto&& ret, const ir_def_resolution& r) -> decltype (auto)
                              {
                                return std::move (ret |= r.get_timelines ());
                              });
    }

    optional_ref<ir_def> def = first_def;
    if (! first_def)
      def.emplace (found_hetero->get_common_def ());
    assert (def);

    // otherwise we need to create an incoming-timeline
    ir_def_timeline& dt = join_block.get_def_timeline (var);

    // note: in the case of loops we can be appending to an already existing def-timeline

    std::for_each (incoming.begin (), incoming.end (),
                   [&dt](const ir_def_resolution& r)
                   {
                     dt.append_incoming (r.get_leaf_block (),
                                         r.get_timelines ().begin (),
                                         r.get_timelines ().end ());
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

    return { nonnull_ptr { dt } };
  }

  //
  // ir_def_resolution
  //

  ir_def&
  ir_def_resolution::
  get_common_def (void) const
  {
    assert_all_same_nonnull_outgoing (get_timelines ());
    return get_timelines ().front ()->get_outgoing_def ();
  }

  optional_ref<ir_def>
  ir_def_resolution::
  maybe_get_common_def (void) const
  {
    if (is_nonempty ())
      return optional_ref { get_common_def () };
    return nullopt;
  }

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

    if (auto res { maybe_get_block_resolution () }) // collapse the stack with the seed
      return resolve_with (res->get_resolution ());
    else if (! m_stack.empty ()) // collapse the stack
    {
      ir_link_set<ir_def_timeline> curr_result = m_stack.top ().join ();
      m_stack.pop ();
      return resolve_with (std::move (curr_result));
    }
    else // the resolution is the aggregate of the leaf components
    {
      small_vector<ir_def_resolution> ret;
      std::for_each (m_leaves.begin (), m_leaves.end (),
                     [&](ir_def_resolution_stack& leaf_stack)
                     { ret.append (leaf_stack.resolve ()); });
      return ret;
    }
  }

  small_vector<ir_def_resolution>
  ir_def_resolution_stack::
  resolve_with (ir_link_set<ir_def_timeline> s)
  {
    if (optional_ref block { maybe_cast_block () })
      return { { *block, std::move (s) } };

    while (! m_stack.empty ())
    {
      s = std::move (top ().join_with (std::move (s)));
      m_stack.pop ();
    }

    small_vector<ir_def_resolution> ret;
    std::for_each (m_leaves.begin (), m_leaves.end (),
                   [&](ir_def_resolution_stack& leaf_stack)
                   {
                     small_vector<ir_def_resolution> res = leaf_stack.resolve_with (s);
                     ret.insert (ret.end (),
                                 std::move_iterator { res.begin () },
                                 std::move_iterator { res.end () });
                   });
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

  ir_link_set<ir_def_timeline>
  ir_def_resolution_frame::
  join (void)
  {
    assert (is_joinable () && "frame should be joinable");
    auto ret = join_at (get_join_block (), get_variable (), m_substack.resolve ());
    assert_homogeneous (ret);
    return ret;
  }

  ir_link_set<ir_def_timeline>
  ir_def_resolution_frame::
  join_with (ir_link_set<ir_def_timeline>&& s)
  {
    auto ret = join_at (get_join_block (), get_variable (),
                        m_substack.resolve_with (std::move (s)));
    assert_homogeneous (ret);
    return ret;
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

}

/** ir-def-resolution.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-block.hpp"
#include "ir-def-resolution.hpp"
#include "ir-optional-util.hpp"
#include "ir-stack-builder.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

namespace gch
{

  bool
  check_all_same_nonnull_outgoing (const ir_link_set<ir_def_timeline>& v)
  {
    if (v.empty ())
      return true;

    if (optional_ref cmp { v.front ()->maybe_get_outgoing_timeline () })
      return std::equal (std::next (v.begin ()), v.end (),
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
  // ir_def_resolution_frame
  //

  ir_def_resolution_frame::
  ir_def_resolution_frame (ir_block& join_block, ir_component_ptr comp)
    : m_join_block (join_block),
      m_node       (comp)
  { }

  bool
  ir_def_resolution_frame::
  is_joinable (void) const noexcept
  {
    // ie. all the substacks are resolved
    return m_node.is_resolvable ();
  }

  ir_link_set<ir_def_timeline>
  ir_def_resolution_frame::
  join (void)
  {
    assert (is_joinable () && "frame should be joinable");
    return join_with (m_node.resolve ());
  }

  ir_link_set<ir_def_timeline>
  ir_def_resolution_frame::
  join_with (small_vector<ir_def_resolution>&& c)
  {
    if (c.empty ())
      return { };

    optional_ref<ir_def> first_def = c.front ().maybe_get_common_def ();
    auto found_hetero = std::find_if_not (std::next (c.begin ()), c.end (),
                                          [first_def](const ir_def_resolution& r)
                                          {
                                            return first_def == r.maybe_get_common_def ();
                                          });

    // check if all the incoming timelines have the same parent def
    // if they do we can skip creation of an incoming-timeline in the current block
    if (found_hetero == c.end ())
    {
      // if homogeneous and we can skip creation of the incoming-timeline
      // and forward the timelines
      return std::accumulate (std::next (c.begin ()), c.end (),
                              c.front ().get_timelines (),
                              [](auto&& ret, const ir_def_resolution& r) -> decltype (auto)
                              {
                                ret.merge (r);
                                return std::move (ret);
                              });
    }

    optional_ref<ir_def> def = first_def;
    if (! first_def)
      def.emplace (found_hetero->get_common_def ());
    assert (def);

    // otherwise we need to create an incoming-timeline
    const ir_variable& var = def->get_variable ();
    ir_def_timeline&   dt  = m_join_block->get_def_timeline (var);

    assert (! dt.has_incoming ()          && "the block already has incoming blocks");
    assert (! dt.has_incoming_timeline () && "the block already has an incoming timeline");

    std::for_each (c.begin (), c.end (),
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

  ir_def_resolution_stack&
  ir_def_resolution_frame::
  add_substack (ir_block& leaf_block)
  {
    m_incoming.emplace_back (leaf_block);
  }

  //
  // ir_def_resolution_stack
  //

  ir_def_resolution_stack::
  ir_def_resolution_stack (ir_component_ptr comp)
    : m_component (comp)
  { }

  bool
  ir_def_resolution_stack::
  is_resolvable (void) const noexcept
  {
    assert (! m_stack.empty () && "stack should not be empty");
    return m_stack.top ().is_joinable ();
  }

  small_vector<ir_def_resolution>
  ir_def_resolution_stack::
  resolve (void)
  {
    assert (is_resolvable () && "stack should be resolvable");

    ir_link_set<ir_def_timeline> curr_result = m_stack.top ().join ();
    m_stack.pop ();
    return resolve_with (std::move (curr_result));
  }

  small_vector<ir_def_resolution>
  ir_def_resolution_stack::
  resolve_with (ir_link_set<ir_def_timeline>&& s)
  {
    assert (is_a<ir_structure> (*get_component ()));
    ir_structure& this_structure = get_as<ir_structure> (get_component ());

    while (! m_stack.empty ())
    {
      ir_link_set<ir_block> curr_preds = this_structure.get_predecessors (top ().get_component ());
      small_vector<ir_def_resolution> r;
      std::transform (curr_preds.begin (), curr_preds.end (), std::back_inserter (r),
                      [&s](nonnull_ptr<ir_block> b) -> ir_def_resolution { return { *b, s }; });
      s = std::move (top ().join_with (std::move (r)));
      m_stack.pop ();
    }

    if (m_leaves.empty ())
    {
      ir_link_set<ir_block> leaves = this_structure.get_leaves ();
    }

    return m_resolution.emplace (c);
  }

  bool
  ir_def_resolution_stack::
  is_resolved (void) const noexcept
  {
    return m_resolution.has_value ();
  }

  bool
  ir_def_resolution_stack::
  is_resolved_nonempty (void) const noexcept
  {
    return is_resolved () && ! m_resolution->empty ();
  }

  const std::optional<ir_link_set<ir_def_timeline>>&
  ir_def_resolution_stack::
  maybe_get_resolution (void) const noexcept
  {
    return m_resolution;
  }

  const ir_link_set<ir_def_timeline>&
  ir_def_resolution_stack::
  get_resolution (void) const noexcept
  {
    return *m_resolution;
  }

  bool
  ir_def_resolution_stack::
  check_all_same_nonnull_outgoing (void) const
  {
    assert (is_resolved () && "the stack should be resolved before using this function");
    return gch::check_all_same_nonnull_outgoing (get_resolution ());
  }

  ir_def&
  ir_def_resolution_stack::
  get_resolved_def (void) const
  {
    assert (is_resolved ()
        &&  "the stack should be resolved before using this function");

    assert (! get_resolution ().empty ()
        &&  "the stack should have at least one resolved timeline");

    assert_all_same_nonnull_outgoing (get_resolution ());

    return get_resolution ().front ()->get_outgoing_def ();
  }

  optional_ref<ir_def>
  ir_def_resolution_stack::
  maybe_get_resolved_def (void) const
  {
    if (is_resolved ())
    {
      assert (! get_resolution ().empty ()
                &&  "the stack should have at least one resolved timeline");

      assert_all_same_nonnull_outgoing (get_resolution ());

      return optional_ref { get_resolution ().front ()->get_outgoing_def () };
    }
    return nullopt;
  }

  ir_block&
  ir_def_resolution_stack::
  get_leaf_block (void) const noexcept
  {
    return *m_leaf_block;
  }

  ir_def_resolution_frame&
  ir_def_resolution_stack::
  push (ir_block& join_block)
  {
    return m_stack.emplace (join_block);
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

}

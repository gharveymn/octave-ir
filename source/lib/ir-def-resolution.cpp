/** ir-def-resolution.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-block.hpp"
#include "ir-def-resolution.hpp"
#include "ir-optional-util.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

#include <list>


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

  bool
  ir_def_resolution_frame::
  is_joinable (void) const noexcept
  {
    // ie. all the substacks are resolved
    return std::all_of (begin (), end (), [](const substack_ptr& u) { u->is_resolved (); });
  }

  ir_link_set<ir_def_timeline>
  ir_def_resolution_frame::
  join (void)
  {
    assert (is_joinable () && "frame should be joinable");

    // find the first non-empty association
    auto found_nonempty = std::find_if (begin (), end (),
                                        [](const substack_ptr& u)
                                        {
                                          return u->is_resolved_nonempty ();
                                        });
    if (found_nonempty == end ())
      return { };

    // check if all the incoming timelines have the same parent def
    // if they do we can skip creation of an incoming-timeline in the current block
    const ir_def& cmp = (*found_nonempty)->get_resolved_def ();
    auto found_hetero = std::find_if (std::next (found_nonempty), end (),
                                      [&cmp](const substack_ptr& u)
                                      {
                                        return ! u->is_resolved ()
                                             ||  &cmp == &u->get_resolved_def ();
                                      });

    if (found_hetero == end ())
    {
      // if homogeneous and we can skip creation of the incoming-timeline
      // and forward the timelines
      return std::accumulate (std::next (found_nonempty), end (),
                              (*found_nonempty)->get_resolution (),
                              [](auto&& ret, const substack_ptr& u) -> decltype (auto)
                              {
                                ret.insert (u->get_resolution ().begin (),
                                            u->get_resolution ().end ());
                                return std::move (ret);
                              });
    }

    // otherwise we need to create an incoming-timeline
    const ir_variable& var = cmp.get_variable ();
    ir_def_timeline&   dt  = m_join_block->get_def_timeline (var);

    assert (! dt.has_incoming ()          && "the block already has incoming blocks");
    assert (! dt.has_incoming_timeline () && "the block already has an incoming timeline");

    std::for_each (begin (), found_nonempty,
                   [&dt](const substack_ptr& u)
                   {
                     dt.append_incoming (u->get_leaf_block (),
                                         u->get_resolution ().begin (),
                                         u->get_resolution ().end ());
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

  ir_link_set<ir_def_timeline>
  ir_def_resolution_frame::
  join_with (ir_link_set<ir_def_timeline>&& c)
  {
    assert_all_same_nonnull_outgoing (c);

    // find the first non-empty association
    auto found_nonempty = std::find_if (begin (), end (),
                                        [](const substack_ptr& u)
                                        {
                                          return u->is_resolved_nonempty ();
                                        });
    if (found_nonempty == end ())
      return c;

    // check if all the incoming timelines have the same parent def
    // if they do we can skip creation of an incoming-timeline in the current block

    // if we found a non-empty association, then it can only be heterogeneous if
    // the c is non-empty.

    if (! c.empty ())
    {
      const ir_def& cmp = c.front ()->get_outgoing_def ();
      auto found_hetero = std::find_if_not (std::next (found_nonempty), end (),
                                            [&cmp](const substack_ptr& u)
                                            {
                                              return ! u->is_resolved ()
                                                   ||  &cmp == &u->get_resolved_def ();
                                            });

      if (found_hetero == end ())
      {
        // if homogeneous and we can skip creation of the incoming-timeline
        // and forward the timelines
        return std::accumulate (std::next (found_nonempty), end (), std::move (c),
                                [](auto&& ret, const substack_ptr& u) -> decltype (auto)
                                {
                                  if (const auto& r = u->maybe_get_resolution ())
                                    ret.insert (r->begin (), r->end ());
                                  return std::move (ret);
                                });
      }
    }

    const ir_def& cmp = (*found_nonempty)->get_resolved_def ();

    // otherwise we need to create an incoming-timeline
    const ir_variable& var = cmp.get_variable ();
    ir_def_timeline&   dt  = m_join_block->get_def_timeline (var);

    assert (! dt.has_incoming ()          && "the block already has incoming blocks");
    assert (! dt.has_incoming_timeline () && "the block already has an incoming timeline");

    std::for_each (begin (), found_nonempty,
                   [&](const substack_ptr& u)
                   {
                     if (const auto& r = u->maybe_get_resolution ())
                       dt.append_incoming (u->get_leaf_block (), r->begin (), r->end ());
                     else
                       dt.append_incoming (u->get_leaf_block (), c.begin (), c.end ());
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

  bool
  ir_def_resolution_stack::
  is_resolvable (void) const noexcept
  {
    assert (! m_stack.empty () && "stack should not be empty");
    return m_stack.top ().is_joinable ();
  }

  const ir_link_set<ir_def_timeline>&
  ir_def_resolution_stack::
  resolve (void)
  {
    assert (is_resolvable () && "stack should be resolvable");
    ir_link_set<ir_def_timeline> curr_result = m_stack.top ().join ();
    m_stack.pop ();
    while (! m_stack.empty ())
    {
      // replace the unresolved timelines in the next frame
      ir_def_resolution_frame& curr_frame = m_stack.top ();
      std::for_each (curr_frame.begin (), curr_frame.end (),
                     [&curr_result](ir_associated_incoming& inc)
                     {
                       if (! inc.is_resolved ())
                         inc.set_timelines (curr_result);
                     });
      curr_result = curr_frame.join ();
      m_stack.pop ();
    }
    return m_resolution.emplace (curr_result);
  }

  const ir_link_set<ir_def_timeline>&
  ir_def_resolution_stack::
  resolve_with (ir_link_set<ir_def_timeline> c)
  {
    while (! m_stack.empty ())
    {
      c = m_stack.top ().join_with (c);
      m_stack.pop ();
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

  ir_block&
  ir_def_resolution_stack::
  get_leaf_block (void) const noexcept
  {
    return *m_leaf_block;
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

}

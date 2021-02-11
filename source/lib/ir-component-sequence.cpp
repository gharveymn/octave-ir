/** ir-sequence.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include "ir-component-sequence.hpp"
#include "ir-block.hpp"
#include "ir-optional-util.hpp"

#include <cassert>
#include <numeric>

namespace gch
{

  auto
  ir_component_sequence::
  find (ir_component& c) const
    -> ptr
  {
    if (m_find_cache.contains (c))
      return m_find_cache.get ();

    ptr found = std::find_if (as_mutable (*this).begin (),
                              as_mutable (*this).end (),
                              [&](const ir_component& cmp) { return &cmp == &c; });

    if (found != end ())
      m_find_cache.emplace (make_handle (found));

    return found;
  }

  auto
  ir_component_sequence::
  flatten_element (ptr pos)
    -> ptr
  {
    if (optional_ref seq { maybe_get_as<ir_component_sequence> (pos) })
    {
      assert (! m_body.empty () && "sequence body should not be empty");

      const auto size_change = seq->size () - 1;
      const auto pos_offset  = std::distance (begin (), pos);

      // move everything after the first element
      const auto resolved_it
        = std::prev (m_body.insert (
                       std::next (get_iter (pos)),
                       std::move_iterator { std::next (seq->container_begin ()) },
                       std::move_iterator { seq->container_end () }));

      // then move the first element into *iter
      *resolved_it = std::move (seq->container_front ());
      const auto after = make_ptr (std::next (resolved_it, size_change + 1));

      std::for_each (make_ptr (resolved_it), after,
                     [this](ir_component& c) { c.set_parent (*this); });

      return after;
    }
    return std::next (pos);
  }

  auto
  ir_component_sequence::
  flatten_range (ptr first, const cptr last)
    -> ptr
  {
    // find the new size as a result of the flattening operation
    std::size_t new_size = std::accumulate (cptr (first), last, size (),
      [](std::size_t curr, ir_component& c)
      {
        return curr + (maybe_cast<ir_component_sequence> (c)
                         >>= [](ir_component_sequence& seq) { return seq.size () - 1; });
      });

    assert (size () <= new_size);
    if (size () < new_size)
    {
      const auto change    = new_size - size ();
      const auto first_off = std::distance (begin (), first);

      // shift over elements after the range
      // if we fail here the operation has no effect
      const auto resolved_last_it  = m_body.insert (get_iter (last), new_size - size (), { });
      const auto resolved_first_it = std::next (m_body.begin (), first_off);
      const auto new_last_it       = std::next (resolved_last_it, change);

      // the reset is noexcept
      // iterate backwards over the range to place new elements
      std::reverse_iterator rcurr_it { new_last_it };
      std::for_each (std::reverse_iterator { resolved_last_it  },
                     std::reverse_iterator { resolved_first_it },
                     [this, &rcurr_it](ir_component_storage& u)
                     {
                       if (optional_ref seq { maybe_cast<ir_component_sequence> (*u) })
                       {
                         std::for_each (seq->begin (), seq->end (),
                                        [this](ir_component& c) { c.set_parent (*this); });

                         rcurr_it = std::move (seq->container_rbegin (),
                                               seq->container_rend (),
                                               rcurr_it);
                       }
                       else
                         *rcurr_it++ = std::move (u);
                     });
      return make_ptr (new_last_it);
    }
    return make_mutable (last);
  }

  void
  ir_component_sequence::
  flatten (void)
  {
    flatten_range (begin (), end ());
  }

  //
  // virtual from ir_component
  //

  bool
  ir_component_sequence::
  reassociate_timelines (const std::vector<nonnull_ptr<ir_def_timeline>>& old_dts,
                         ir_def_timeline& new_dt, std::vector<nonnull_ptr<ir_block>>& until)
  {
    return std::any_of (begin (), end (),
                        [&](ir_component& c)
                        {
                          return c.reassociate_timelines (old_dts, new_dt, until);
                        });
  }

  void
  ir_component_sequence::
  reset (void) noexcept
  {
    m_body.erase (std::next (m_body.begin ()), m_body.end ());
    front ().reset ();
    m_find_cache.emplace (make_handle (begin ()));
    invalidate_leaf_cache ();
  }

  //
  // virtual from ir_structure
  //

  ir_component_ptr
  ir_component_sequence::
  get_ptr (ir_component& c) const
  {
    return find (c);
  }

  ir_component_ptr
  ir_component_sequence::
  get_entry_ptr (void)
  {
    return begin ();
  }

  ir_link_set
  ir_component_sequence::
  get_predecessors (ir_component_cptr comp)
  {
    if (is_entry (comp))
      return get_parent ().get_predecessors (*this);
    return copy_leaves (std::prev (make_mutable (comp)));
  }

  ir_link_set
  ir_component_sequence::
  get_successors (ir_component_cptr comp)
  {
    if (is_leaf (comp))
      return get_parent ().get_successors (*this);
    return { nonnull_ptr { get_entry_block (std::next (make_mutable (comp))) } };
  }

  bool
  ir_component_sequence::
  is_leaf (ir_component_cptr comp) noexcept
  {
    return comp == last ();
  }

  void
  ir_component_sequence::
  generate_leaf_cache (void)
  {
    assert (leaf_cache_empty () && "generating for non-empty leaf cache");
    assert (! m_body.empty ()   && "sequence body should not be empty");
    leaves_append (last ());
  }

  void
  ir_component_sequence::
  recursive_flatten (void)
  {
    std::vector<ir_component_mover> collected = recursive_collect_components ();

    assert (size () <= collected.size ());
    if (size () < collected.size ())
    {
      const auto pivot = std::next (collected.begin (), size ());
      const auto old_end = m_body.insert (m_body.end (), pivot, collected.end ());
      std::move_backward (collected.begin (), collected.end (), old_end);
    }

    // now flatten all subcomponents
    std::for_each (begin (), end (),
      [](ir_component& c) { maybe_cast<ir_structure> (c) >>= &ir_structure::recursive_flatten; });
  }

  void
  ir_component_sequence::
  reassociate_timelines_after (ir_component_ptr pos, ir_def_timeline& dt,
                               const std::vector<nonnull_ptr<ir_block>>& until)
  {
    if (is_leaf (pos))
      get_parent ().reassociate_timelines_after (pos, dt, until);

  }

  std::vector<ir_component_mover>&
  ir_component_sequence::
  recursive_collect_components (std::vector<ir_component_mover>& collector)
  {
    std::for_each (container_begin (), container_end (),
                   [&](ir_component_storage& u)
                   {
                     if (optional_ref seq { maybe_cast<ir_component_sequence> (*u) })
                       seq->recursive_collect_components (collector);
                     else
                       collector.emplace_back (u);
                   });
    return collector;
  }

}

/** ir-sequence.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include "components/ir-component-sequence.hpp"
#include "components/ir-block.hpp"
#include "utilities/ir-optional-util.hpp"
#include "utilities/ir-error.hpp"

#include <cassert>
#include <numeric>
#include <vector>

namespace gch
{

  auto
  ir_component_sequence::
  find (ir_subcomponent& sub) const
    -> iter
  {
    if (m_find_cache.contains (sub))
      return m_find_cache.get ();

    iter found = std::find_if (as_mutable (*this).begin (),
                              as_mutable (*this).end (),
                              [&](const ir_component& cmp) { return &cmp == &sub; });

    if (found != end ())
      m_find_cache.emplace (make_handle (found));

    return found;
  }

  auto
  ir_component_sequence::
  flatten_element (iter pos)
    -> iter
  {
    if (optional_ref seq { maybe_as_type<ir_component_sequence> (pos) })
    {
      assert (! m_body.empty () && "sequence body should not be empty");

      const auto size_change = seq->size () - 1;
      const auto pos_offset  = std::distance (begin (), pos);

      // move everything after the first element
      const auto resolved_it
        = std::prev (m_body.insert (
                       std::next (get_container_iter (pos)),
                       std::move_iterator { std::next (seq->begin ()) },
                       std::move_iterator { seq->end () }));

      // then move the first element into *iter
      *resolved_it = std::move (seq->m_body.front ());
      const auto after = std::next (resolved_it, size_change + 1);

      std::for_each (resolved_it, after,
                     [this](auto& u) { u->set_parent (*this); });

      return make_ptr (after);
    }
    return std::next (pos);
  }

  auto
  ir_component_sequence::
  flatten_range (iter first, const citer last)
    -> iter
  {
    // find the new size as a result of the flattening operation
    std::size_t new_size = std::accumulate (citer (first), last, size (),
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
      const auto resolved_last_it  = m_body.insert (get_container_iter (last),
                                                    new_size - size (),
                                                    { });

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
                                        [this](ir_subcomponent& sub) { sub.set_parent (*this); });

                         rcurr_it = std::move (seq->rbegin (),
                                               seq->rend (),
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

  void
  ir_component_sequence::
  recursive_flatten (void)
  {
    std::vector<ir_component_mover> collected (recursive_collect_components ());

    assert (size () <= collected.size ());
    if (size () < collected.size ())
    {
      const auto pivot = std::next (collected.begin (), size ());
      const auto old_end = m_body.insert (m_body.end (), pivot, collected.end ());
      std::move_backward (collected.begin (), collected.end (), old_end);
    }

    // now flatten all subcomponents
    std::for_each (begin (), end (),
                   [](ir_subcomponent& sub) { maybe_cast<ir_structure> (sub) >>= &gch::flatten; });
  }

  //
  // virtual from ir_component
  //

  bool
  ir_component_sequence::
  reassociate_timelines (const ir_link_set<ir_def_timeline>& old_dts, ir_def_timeline& new_dt,
                         std::vector<nonnull_ptr<ir_block>>& until)
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

  auto
  ir_component_sequence::
  must_find (const ir_subcomponent& c)
    -> citer
  {
    if (citer cit = find (c); cit != end ())
      return cit;
    abort::ir_logic_error ("Component not found in the structure.");
  }

  //
  // virtual from ir_structure
  //

  std::vector<ir_component_mover>&
  ir_component_sequence::
  recursive_collect_components (std::vector<ir_component_mover>& collector)
  {
    std::for_each (begin (), end (),
                   [&](ir_component_storage& u)
                   {
                     if (optional_ref seq { maybe_cast<ir_component_sequence> (*u) })
                       seq->recursive_collect_components (collector);
                     else
                       collector.emplace_back (u);
                   });
    return collector;
  }

  std::vector<ir_component_mover>
  ir_component_sequence::
  recursive_collect_components (void)
  {
    std::vector<ir_component_mover> ret { };
    recursive_collect_components (ret);
    return ret;
  }



}

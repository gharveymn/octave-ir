/** ir-sequence.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include "ir-component-sequence.hpp"
#include "ir-block.hpp"
#include "ir-error.hpp"
#include "ir-all-substructure-visitors.hpp"

#include <cassert>
#include <numeric>
#include <vector>

namespace gch
{

  ir_component_sequence::
  ~ir_component_sequence (void) = default;

  ir_component_sequence::
  ir_component_sequence (ir_structure& parent, std::initializer_list<ir_component_mover> init)
    : ir_substructure { parent },
      m_body          (init.begin (), init.end ()),
      m_find_cache    { make_handle (begin ()) }
  { }

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
    if (optional_ref seq { maybe_as_component<ir_component_sequence> (pos) })
    {
      assert (! m_body.empty () && "sequence body should not be empty");

      const diff_ty size_change = static_cast<diff_ty> (seq->size () - 1);

      // move everything after the first element
      const auto resolved_it
        = std::prev (m_body.insert (
                       std::next (get_container_iter (pos)),
                       std::make_move_iterator (std::next (seq->m_body.begin ())),
                       std::make_move_iterator (seq->m_body.end ())));

      // then move the first element into *iter
      *resolved_it = std::move (seq->m_body.front ());
      const auto after = std::next (resolved_it, size_change + 1);

      std::for_each (resolved_it, after, [this](ir_component_storage& u) {
        u->set_parent (*this);
      });

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
    std::size_t new_size = std::accumulate (first, make_mutable (last), size (),
      [](std::size_t curr, ir_subcomponent& sub)
      {

        return curr + (maybe_cast<ir_component_sequence> (sub)
                         >>= [](auto& seq) noexcept { return seq.size () - 1; });
      });

    assert (size () <= new_size);
    if (size () < new_size)
    {
      assert (new_size <= m_body.max_size ());
      const auto change    = static_cast<diff_ty> (new_size - size ());
      const auto first_off = std::distance (begin (), first);
      const auto last_off  = std::distance (cbegin (), last);
      const auto old_size  = size ();

      // shift over elements after the range
      // if we fail here the operation has no effect
      m_body.resize (new_size);
      const auto old_end_it        = std::next (m_body.begin (), static_cast<diff_ty> (old_size));
      const auto resolved_first_it = std::next (m_body.begin (), first_off);
      const auto resolved_last_it  = std::next (m_body.begin (), last_off);

      const auto new_last_it = std::move_backward (resolved_last_it, old_end_it, m_body.end ());
      assert (new_last_it == std::next (resolved_last_it, change));

      // the reset is noexcept
      // iterate backwards over the range to place new elements
      std::reverse_iterator<container_type::iterator> rcurr_it { new_last_it };
      std::for_each (std::make_reverse_iterator (resolved_last_it),
                     std::make_reverse_iterator (resolved_first_it),
                     [&](ir_component_storage& u)
                     {
                       if (optional_ref seq { maybe_cast<ir_component_sequence> (*u) })
                       {
                         std::for_each (seq->begin (), seq->end (),
                                        [this](ir_subcomponent& sub) { sub.set_parent (*this); });

                         rcurr_it = std::move (seq->m_body.rbegin (), seq->m_body.rend (),
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
  flatten_level (void)
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
      const auto pivot = std::next (collected.begin (), static_cast<diff_ty> (size ()));
      const auto old_end = m_body.insert (m_body.end (), pivot, collected.end ());
      std::move_backward (collected.begin (), collected.end (), old_end);
    }

    // now flatten all subcomponents
    std::for_each (begin (), end (), [](ir_subcomponent& sub)
                                     { maybe_cast<ir_structure> (sub) >>= &gch::flatten; });
  }

  auto
  ir_component_sequence::
  must_find (const ir_subcomponent& c)
    -> citer
  {
    if (citer cit = find (c); cit != end ())
      return cit;
    abort<reason::logic_error> ("Component not found in the structure.");
  }

  //
  // virtual from ir_structure
  //

  std::vector<ir_component_mover>&
  ir_component_sequence::
  recursive_collect_components (std::vector<ir_component_mover>& collector)
  {
    std::for_each (m_body.begin (), m_body.end (),
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

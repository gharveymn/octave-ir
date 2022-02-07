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

  ir_component_sequence::find_cache::
  find_cache (ir_component_handle it) noexcept
    : m_handle (it)
  { }

  void
  ir_component_sequence::find_cache::
  emplace (ir_component_handle it) noexcept
  {
    m_handle = it;
  }

  bool
  ir_component_sequence::find_cache::
  contains (const ir_subcomponent& sub) const noexcept
  {
    return &sub == m_handle;
  }

  ir_component_handle
  ir_component_sequence::find_cache::
  get (void) const noexcept
  {
    return m_handle;
  }

  ir_component_sequence::
  ~ir_component_sequence (void) = default;

  ir_component_sequence::
  ir_component_sequence (ir_structure& parent)
    : ir_component_sequence (parent, ir_subcomponent_type<ir_block>)
  { }

  ir_component_sequence::
  ir_component_sequence (ir_structure& parent, std::initializer_list<ir_component_mover> init)
    : ir_substructure { parent },
      m_body          (init.begin (), init.end ()),
      m_find_cache    { make_handle (begin ()) }
  { }

  ir_component_sequence::
  ir_component_sequence (ir_structure& parent, ir_component_mover init)
    : ir_component_sequence (parent, { init })
  { }

  auto
  ir_component_sequence::
  make_mutable (const citer cit)
    -> iter
  {
    return std::next (begin (), std::distance (cbegin (), cit));
  }

  auto
  ir_component_sequence::
  get_container_iter (iter it) const
    -> typename container_type::iterator
  {
    return std::next (as_mutable (*this).m_body.begin (),
                      std::distance (as_mutable (*this).begin (), it));
  }

  auto
  ir_component_sequence::
  get_container_iter (citer cit) const
    -> typename container_type::const_iterator
  {
    return std::next (m_body.cbegin (), std::distance (cbegin (), cit));
  }

  auto
  ir_component_sequence::
  begin (void) noexcept
    -> iter
  {
    return make_ptr (m_body.begin ());
  }

  auto
  ir_component_sequence::
  begin (void) const noexcept
    -> citer
  {
    return as_mutable (*this).begin ();
  }

  auto
  ir_component_sequence::
  cbegin (void) const noexcept
    -> citer
  {
    return begin ();
  }

  auto
  ir_component_sequence::
  end (void) noexcept
    -> iter
  {
    return make_ptr (m_body.end ());
  }

  auto
  ir_component_sequence::
  end (void) const noexcept
    -> citer
  {
    return as_mutable (*this).end ();
  }

  auto
  ir_component_sequence::
  cend (void) const noexcept
    -> citer
  {
    return end ();
  }

  auto
  ir_component_sequence::
  rbegin (void) noexcept
    -> riter
  {
    return riter { end () };
  }

  auto
  ir_component_sequence::
  rbegin (void) const noexcept
    -> criter
  {
    return as_mutable (*this).rbegin ();
  }

  auto
  ir_component_sequence::
  crbegin (void) const noexcept
    -> criter
  {
    return rbegin ();
  }

  auto
  ir_component_sequence::
  rend (void) noexcept
    -> riter
  {
    return riter { begin () };
  }

  auto
  ir_component_sequence::
  rend (void) const noexcept
    -> criter
  {
    return as_mutable (*this).rend ();
  }

  auto
  ir_component_sequence::
  crend (void) const noexcept
    -> criter
  {
    return rend ();
  }

  auto
  ir_component_sequence::
  front (void)
    -> ref
  {
    return *begin ();
  }

  auto
  ir_component_sequence::
  front (void) const
    -> cref
  {
    return as_mutable (*this).front ();
  }

  auto
  ir_component_sequence::
  back (void)
    -> ref
  {
    return *rbegin ();
  }

  auto
  ir_component_sequence::
  back (void) const
    -> cref
  {
    return as_mutable (*this).back ();
  }

  auto
  ir_component_sequence::
  size (void) const noexcept
    -> size_ty
  {
    return m_body.size ();
  }

  [[nodiscard]]
  auto
  ir_component_sequence::
  last (void) noexcept
    -> iter
  {
    return std::prev (end ());
  }

  [[nodiscard]]
  auto
  ir_component_sequence::
  last (void) const noexcept
    -> citer
  {
    return as_mutable (*this).last ();
  }

  [[nodiscard]]
  auto
  ir_component_sequence::
  clast (void) const noexcept
    -> citer
  {
    return last ();
  }

  auto
  ir_component_sequence::
  find (ir_subcomponent& sub) const
    -> iter
  {
    if (m_find_cache.contains (sub))
      return m_find_cache.get ();

    iter found = std::find_if (as_mutable (*this).begin (), as_mutable (*this).end (),
                               [&](const ir_component& cmp) {
                                 return &cmp == &sub;
                               });

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
      assert (new_last_it == std::next (resolved_last_it,
                                        static_cast<diff_ty> (new_size - old_size)));

      // the reset is noexcept
      // iterate backwards over the range to place new elements
      std::reverse_iterator<container_type::iterator> rcurr_it { new_last_it };
      std::for_each (std::make_reverse_iterator (resolved_last_it),
                     std::make_reverse_iterator (resolved_first_it),
                     [&](ir_component_storage& u) {
                       if (optional_ref seq { maybe_cast<ir_component_sequence> (*u) })
                       {
                         std::for_each (seq->begin (), seq->end (), [this](ir_subcomponent& sub) {
                           sub.set_parent (*this);
                         });

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
    std::for_each (begin (), end (), [](ir_subcomponent& sub) {
      maybe_cast<ir_structure> (sub) >>= &gch::flatten;
    });
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
    std::for_each (m_body.begin (), m_body.end (), [&](ir_component_storage& u) {
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

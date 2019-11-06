/*

Copyright (C) 2019 Gene Harvey

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "ir-structure.h"
#include "ir-component.h"

#include <algorithm>

namespace octave
{

  //
  // ir_component_sequence
  //

  template <typename T, typename ...Args>
  enable_if_t<std::is_base_of<ir_component, T>::value, T>&
  ir_component_sequence::emplace_back (Args&&... args)
  {

    static_assert (std::is_same<T, ir_component_sequence>::value,
                   "Cannot nest component sequences.");
    std::unique_ptr<T> u = make_unique<T> (get_module (), *this,
                                              std::forward<Args> (args)...);
    T *ret = u.get ();
    m_body.emplace_back (std::move (u));
    if (auto s = dynamic_cast<ir_fork_component *> (get_parent ()))
      s->invalidate_leaf_cache ();
    return *ret;
  }

  ir_component_sequence::comp_citer
  ir_component_sequence::find (ir_component *blk) const
  {
    return std::find_if (m_body.begin (), m_body.end (),
                         [blk](comp_cref u) { return u.get () == blk; });
  }

  ir_component::link_iter
  ir_component_sequence::pred_begin (ir_component *c)
  {
    comp_citer cit = find (c);
    if (cit == m_body.begin ())
      {
        if (ir_structure *p = get_parent ())
          return p->pred_begin (this);
        // condition fails if this is the super-sequence
        return link_iter (nullptr);
      }
    --cit;
    // cit now points to one-before c
    return (*cit)->leaf_begin ();
  }

  ir_component::link_iter
  ir_component_sequence::pred_end (ir_component *c)
  {
    comp_citer cit = find (c);
    if (cit == m_body.begin ())
      {
        if (ir_structure *p = get_parent ())
          return p->pred_end (this);
        // condition fails if this is the super-sequence
        return link_iter (nullptr);
      }
    --cit;
    // cit now points to one-before c
    return (*cit)->leaf_end ();
  }

  ir_component::link_iter
  ir_component_sequence::leaf_begin (void) noexcept
  {
    if (empty ())
      return link_iter (nullptr);
    return back ()->leaf_begin ();
  }

  ir_component::link_citer
  ir_component_sequence::leaf_begin (void) const noexcept
  {
    if (empty ())
      return link_iter (nullptr);
    return back ()->leaf_begin ();
  }

  ir_component::link_iter
  ir_component_sequence::leaf_end (void) noexcept
  {
    if (empty ())
      return link_iter (nullptr);
    return back ()->leaf_end ();
  }

  ir_component::link_citer
  ir_component_sequence::leaf_end (void) const noexcept
  {
    if (empty ())
      return link_iter (nullptr);
    return back ()->leaf_end ();
  }

  //
  // ir_fork_component
  //

  ir_component::link_iter
  ir_fork_component::pred_begin (ir_component *c)
  {
    if (c == &m_condition)
      return get_parent ()->pred_begin (this);
    return m_condition.leaf_begin ();
  }

  ir_component::link_iter
  ir_fork_component::pred_end (ir_component *c)
  {
    if (c == &m_condition)
      return get_parent ()->pred_end (this);
    return m_condition.leaf_end ();
  }

  void
  ir_fork_component::invalidate_leaf_cache (void) noexcept
  {
    m_leaf_cache.clear ();
    if (auto s = dynamic_cast<ir_fork_component *> (get_parent ()))
      s->invalidate_leaf_cache ();
  }

  void
  ir_fork_component::generate_leaf_cache (void)
  {
    m_leaf_cache.push_back (&m_condition);
    for (comp_ref u : m_subcomponents)
      m_leaf_cache.insert (m_leaf_cache.end (), u->leaf_begin (),
                           u->leaf_end ());
  }

  ir_component::link_iter
  ir_fork_component::leaf_begin (void)
  {
    if (m_leaf_cache.empty ())
      generate_leaf_cache ();
    return m_leaf_cache.begin ();
  }

  ir_component::link_citer
  ir_fork_component::leaf_begin (void) const noexcept
  {
    return m_leaf_cache.begin ();
  }

  ir_component::link_iter
  ir_fork_component::leaf_end (void)
  {
    if (m_leaf_cache.empty ())
      generate_leaf_cache ();
    return m_leaf_cache.end ();
  }

  ir_component::link_citer
  ir_fork_component::leaf_end (void) const noexcept
  {
    return m_leaf_cache.end ();
  }

  //
  // ir_loop_component
  //

  ir_component::link_iter
  ir_loop_component::pred_begin (ir_component *c)
  {
    if (c == &m_entry)
      return get_parent ()->pred_begin (this);
    else if (c == &m_condition)
      return m_cond_preds.begin ();
    else if (c == &m_body)
      return m_condition.leaf_begin ();
    else if (c == &m_update)
      return m_body.leaf_begin ();
    throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::pred_end (ir_component *c)
  {
    if (c == &m_entry)
      return get_parent ()->pred_end (this);
    else if (c == &m_condition)
      return m_cond_preds.end ();
    else if (c == &m_body)
      return m_condition.leaf_end ();
    else if (c == &m_update)
      return m_body.leaf_end ();
    throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::leaf_begin (void) noexcept
  {
    return m_leaf.begin ();
  }

  ir_component::link_citer
  ir_loop_component::leaf_begin (void) const noexcept
  {
    return m_leaf.begin ();
  }

  ir_component::link_iter
  ir_loop_component::leaf_end (void) noexcept
  {
    return m_leaf.end ();
  }

  ir_component::link_citer
  ir_loop_component::leaf_end (void) const noexcept
  {
    return m_leaf.end ();
  }
}

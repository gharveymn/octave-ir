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
#include "ir-block.h"

#include <algorithm>

namespace octave
{
  //
  // ir_structure
  //

  ir_structure::~ir_structure (void) noexcept = default;

  void
  ir_structure::leaf_push_back (ir_basic_block *blk)
  {
    m_leaf_cache.push_back (blk);
  }

  template <typename It>
  void
  ir_structure::leaf_push_back (It first, It last)
  {
    std::copy (first, last, std::back_inserter (m_leaf_cache));
  }

  ir_structure::link_iter
  ir_structure::leaf_begin (void)
  {
    if (leaf_cache_empty ())
      generate_leaf_cache ();
    return link_iter (m_leaf_cache.begin ());
  }

  ir_structure::link_iter
  ir_structure::leaf_end (void)
  {
    if (leaf_cache_empty ())
      generate_leaf_cache ();
    return link_iter (m_leaf_cache.end ());
  }

  //
  // ir_sequence
  //

  ir_sequence::~ir_sequence (void) noexcept = default;

  ir_sequence::comp_citer
  ir_sequence::last (void) const
  {
    if (empty ())
      throw ir_exception ("Component sequence was empty.");
    return --end ();
  }

  ir_sequence::comp_citer
  ir_sequence::find (ir_component *c)
  {
    if (c != m_find_cache.first)
      m_find_cache = {c,
                      std::find_if (m_body.begin (), m_body.end (),
                                  [c](comp_cref u) {return u.get () == c; })};
    return m_find_cache.second;
  }

  ir_basic_block *
  ir_sequence::get_entry_block (void)
  {
    if (empty ())
      throw ir_exception ("sequence was empty");
    return front ()->get_entry_block ();
  }

  ir_component::link_iter
  ir_sequence::pred_begin (ir_component *c)
  {
    comp_citer cit = must_find (c);
    if (cit == begin ())
      return link_iter (nullptr);
    return link_iter ((*--cit)->leaf_begin ());
  }

  ir_component::link_iter
  ir_sequence::pred_end (ir_component *c)
  {
    comp_citer cit = must_find (c);
    if (cit == begin ())
      return link_iter (nullptr);
    return ++link_iter ((*--cit)->leaf_end ());
  }

  ir_component::link_iter
  ir_sequence::succ_begin (ir_component *c)
  {
    comp_citer cit = must_find (c);
    if (cit == last ())
        return link_iter (nullptr);
    return link_iter ((*++cit)->get_entry_block ());
  }

  ir_component::link_iter
  ir_sequence::succ_end (ir_component *c)
  {
    comp_citer cit = must_find (c);
    if (cit == last ())
      return link_iter (nullptr);
    return ++link_iter ((*++cit)->get_entry_block ());
  }

  void
  ir_sequence::invalidate_leaf_cache (void) noexcept
  {
    clear_leaf_cache ();
  }

  bool
  ir_sequence::is_leaf_component (ir_component *c) noexcept
  {
    if (empty ())
      return false;
    return c == back ().get ();
  }

  void
  ir_sequence::generate_leaf_cache (void)
  {
    if (empty ())
      return invalidate_leaf_cache ();
    leaf_push_back (back ()->leaf_begin (), back ()->leaf_end ());
  }

  ir_sequence::comp_citer
  ir_sequence::must_find (ir_component *c)
  {
    comp_citer cit = find (c);
    if (cit == end ())
      throw ir_exception ("component not found in the structure");
    return cit;
  }

  //
  // ir_subsequence
  //

  template <typename T>
  ir_subsequence
  ir_subsequence::create (ir_module& mod, ir_structure& parent)
  {
    return { mod, parent, init_wrapper<T>{} };
  }

  ir_subsequence::~ir_subsequence (void) noexcept = default;

  ir_component::link_iter
  ir_subsequence::pred_begin (ir_component *c)
  {
    comp_citer cit = must_find (c);
    if (cit == begin ())
      return m_parent.pred_begin (this);
    return (*--cit)->leaf_begin ();
  }

  ir_component::link_iter
  ir_subsequence::pred_end (ir_component *c)
  {
    comp_citer cit = must_find (c);
    if (cit == begin ())
      return m_parent.pred_end (this);
    return (*--cit)->leaf_end ();
  }

  ir_component::link_iter
  ir_subsequence::succ_begin (ir_component *c)
  {
    comp_citer cit = must_find (c);
    if (cit == last ())
      return m_parent.succ_begin (this);
    return link_iter ((*++cit)->get_entry_block ());
  }

  ir_component::link_iter
  ir_subsequence::succ_end (ir_component *c)
  {
    comp_citer cit = must_find (c);
    if (cit == last ())
      return m_parent.succ_end (this);
    return ++link_iter ((*++cit)->get_entry_block ());
  }

  void
  ir_subsequence::invalidate_leaf_cache (void) noexcept
  {
    if (! leaf_cache_empty ())
      {
        clear_leaf_cache ();
        if (is_leaf_component (this))
          m_parent.invalidate_leaf_cache ();
      }
  }

  //
  // ir_fork_component
  //

  ir_fork_component::~ir_fork_component (void) noexcept = default;

  ir_component::link_iter
  ir_fork_component::pred_begin (ir_component *c)
  {
    if (c == &m_condition)
      return m_parent.pred_begin (this);
    return link_iter (&m_condition);
  }

  ir_component::link_iter
  ir_fork_component::pred_end (ir_component *c)
  {
    if (c == &m_condition)
      return m_parent.pred_end (this);
    return ++link_iter (&m_condition);
  }

  ir_component::link_iter
  ir_fork_component::succ_begin (ir_component *c)
  {
    if (c == &m_condition)
      return sub_entry_begin ();
    return m_parent.succ_begin (this);
  }

  ir_component::link_iter
  ir_fork_component::succ_end (ir_component *c)
  {
    if (c == &m_condition)
      return sub_entry_end ();
    return m_parent.succ_end (this);
  }

  ir_basic_block *
  ir_fork_component::get_entry_block (void)
  {
    return &m_condition;
  }

  bool
  ir_fork_component::is_leaf_component (ir_component *c) noexcept
  {
    // assumes that c is in the component
    return c != &m_condition;
  }

  void
  ir_fork_component::generate_leaf_cache (void)
  {
    for (comp_ref c_uptr : m_subcomponents)
      leaf_push_back (c_uptr->leaf_begin (), c_uptr->leaf_end ());
  }

  ir_component::link_iter
  ir_fork_component::sub_entry_begin (void)
  {
    if (m_sub_entry_cache.empty ())
      generate_sub_entry_cache ();
    return link_iter (m_sub_entry_cache.begin ());
  }

  ir_component::link_iter
  ir_fork_component::sub_entry_end (void)
  {
    if (m_sub_entry_cache.empty ())
      generate_sub_entry_cache ();
    return link_iter (m_sub_entry_cache.end ());
  }

  void
  ir_fork_component::generate_sub_entry_cache (void)
  {
    for (comp_ref c_uptr : m_subcomponents)
      leaf_push_back (c_uptr->get_entry_block ());
  }

  //
  // ir_loop_component
  //

  ir_loop_component::~ir_loop_component (void) noexcept = default;

  ir_component::link_iter
  ir_loop_component::pred_begin (ir_component *c)
  {
    if (c == &m_entry)
      return m_parent.pred_begin (this);
    else if (c == &m_condition)
      return link_iter (m_cond_preds.begin ());
    else if (c == &m_body)
      return link_iter (&m_condition);
    throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::pred_end (ir_component *c)
  {
    if (c == &m_entry)
      return m_parent.pred_end (this);
    else if (c == &m_condition)
      return link_iter (m_cond_preds.end ());
    else if (c == &m_body)
      return ++link_iter (&m_condition);
    throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::succ_begin (ir_component *c)
  {
    if (c == &m_entry)
      return link_iter (&m_condition);
    else if (c == &m_condition)
      return cond_succ_begin ();
    else if (c == &m_body)
      return link_iter (&m_condition);
    throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::succ_end (ir_component *c)
  {
    if (c == &m_entry)
      return m_parent.pred_end (this);
    else if (c == &m_condition)
      return cond_succ_end ();
    else if (c == &m_body)
      return ++link_iter (&m_condition);
    throw ir_exception ("Component was not in the loop component.");
  }
  
  ir_basic_block *
  ir_loop_component::get_update_block (void) const noexcept
  {
    return static_cast<ir_basic_block *> (m_body.back ().get ());
  }

  ir_component::link_iter
  ir_loop_component::cond_succ_begin (void)
  {
    m_succ_cache.front () = m_body.get_entry_block ();
    return link_iter (m_succ_cache.begin ());
  }

  ir_component::link_iter
  ir_loop_component::cond_succ_end (void)
  {
    return link_iter (m_succ_cache.end ());
  }

}

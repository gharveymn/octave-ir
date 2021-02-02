/** ir-sequence.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include "ir-component-sequence.hpp"

#include <cassert>

namespace gch
{

  auto
  ir_component_sequence::
  find (const ir_component& c)
    -> iter
  {
    if (m_find_cache.contains (c))
      return m_find_cache.get ();

    iter found = std::find (m_body.begin (), m_body.end (), c);

    if (found != end ())
      m_find_cache.emplace (found);

    return found;
  }

  auto
  ir_component_sequence::
  get_pos (ir_component_handle comp)
    -> iter
  {
    return std::next (begin (), std::distance (&*cbegin (), &comp));
  }

  ir_block&
  ir_component_sequence::
  split (ir_block& blk, ir_instruction_iter pivot) //override
  {
    auto  ret_it = emplace_before<ir_block> (must_find (blk));
    auto& ret    = static_cast<ir_block&> (**ret_it);

    // transfer instructions which occur after the pivot
    blk.split (pivot, ret);
  }

  [[nodiscard]]
  std::list<nonnull_ptr<ir_def>>
  ir_component_sequence::
  get_latest_defs (ir_variable& var) noexcept
  {
    for (auto rit = rbegin (); rit != rend (); ++rit)
    {
      if (auto ret = (*rit)->get_latest_defs (var); ! ret.empty ())
        return ret;
    }
    return { };
  }

  //
  // virtual from ir_component
  //

  void
  ir_component_sequence::
  reset (void) noexcept
  {
    m_body.erase (++m_body.begin (), m_body.end ());
    front ()->reset ();
    m_find_cache.emplace (begin ());
    invalidate_leaf_cache ();
  }

  //
  // virtual from ir_structure
  //

  ir_component_handle
  ir_component_sequence::
  get_entry_component (void)
  {
    return front ();
  }

  ir_component_handle
  ir_component_sequence::
  get_handle (const ir_component& c) const
  {
    return *find (c);
  }

  auto
  ir_component_sequence::
  get_preds (ir_component_handle comp)
    -> link_vector
  {
    if (is_entry_component (comp))
      return get_parent ().get_preds (*this);
    return copy_leaves (*std::prev (get_pos (comp)));
  }

  void
  ir_component_sequence::
  generate_leaf_cache (void)
  {
    if (! empty ())
      leaves_append (back ());
  }

  bool
  ir_component_sequence::
  is_leaf_component (ir_component_handle comp) noexcept
  {
    return comp == back ();
  }

}

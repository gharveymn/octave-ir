/** ir-loop-component.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"

namespace gch
{

  ir_component_loop::
  ir_component_loop (ir_structure& parent)
    : ir_structure (parent),
      m_entry     (create_component<ir_block> ()),
      m_condition (create_component<ir_condition_block> ()),
      m_body      (create_component<ir_component_sequence> (std::in_place_type<ir_block>)),
      m_cond_preds { m_entry, get_update_block () },
      m_succ_cache { m_body.get_entry_block (), m_exit }
  { }

  ir_component_loop::
  ~ir_component_loop (void) noexcept = default;

  ir_component::link_iter
  ir_component_loop::
  preds_begin (ir_component& c)
  {
    if      (is_entry (c))     return m_parent.preds_begin (*this);
    else if (is_condition (c)) return m_cond_preds.begin ();
    else if (is_body (c))      return m_condition;
    else                       throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_component_loop::
  preds_end (ir_component& c)
  {
    if      (is_entry (c))     return m_parent.preds_end (*this);
    else if (is_condition (c)) return m_cond_preds.end ();
    else if (is_body (c))      return ++link_iter (m_condition);
    else                       throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_component_loop::
  succs_begin (ir_component& c)
  {
    if      (is_entry (c))     return m_condition;
    else if (is_condition (c)) return cond_succ_begin ();
    else if (is_body (c))      return m_condition;
    else                       throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_component_loop::
  succs_end (ir_component& c)
  {
    if      (is_entry (c))     return ++link_iter (m_condition);
    else if (is_condition (c)) return cond_succ_end ();
    else if (is_body (c))      return ++link_iter (m_condition);
    else                       throw ir_exception ("component was not in the loop component");
  }

  ir_component::link_iter
  ir_component_loop::
  cond_succ_begin (void)
  {
    m_succ_cache.front () = m_body.get_entry_block ();
    return m_succ_cache.begin ();
  }

  ir_component::link_iter
  ir_component_loop::
  cond_succ_end (void)
  {
    return m_succ_cache.end ();
  }

  //
  // virtual from ir_component
  //

  //
  // virtual from ir_structure
  //

  auto
  ir_component_loop::
  get_preds (ir_component_handle comp)
    -> link_vector
  {
    if (is_entry_component (comp))
      return get_parent ().get_preds (*this);

    if (is_condition_component (comp))
      return copy_leaves (get_start_component (), get_update_component ());
    if (is_body_component (comp))
      return copy_leaves (get_condition_component ());
    if (is_update_component (comp))
      return copy_leaves (get_body_component ());

    throw ir_exception ("component was not in the loop component");
  }

  ir_component_handle
  ir_component_loop::
  get_entry_component (void)
  {
    return get_start_component ();
  }

  void
  ir_component_loop::
  generate_leaf_cache (void)
  {
    throw ir_exception ("this should not run");
  }

  bool
  ir_component_loop::
  is_leaf_component (ir_component_handle comp) noexcept
  {
    return comp == m_condition;
  }

}

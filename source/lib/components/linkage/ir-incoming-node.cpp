/** ir-incoming-node.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "components/linkage/ir-incoming-node.hpp"


#include "components/linkage/ir-def-timeline.hpp"
#include "utilities/ir-common.hpp"

namespace gch
{

  ir_incoming_node::
  ir_incoming_node (ir_def_timeline& parent, ir_incoming_node&& other) noexcept
    : base             (std::move (other)),
      m_parent         (parent)
  { }

  ir_incoming_node::
  ir_incoming_node (ir_def_timeline& parent)
    : m_parent         (parent)
  { }

  ir_incoming_node::
  ir_incoming_node (ir_def_timeline& parent, ir_def_timeline& pred)
    : base             (tag::bind, pred),
      m_parent         (parent)
  { }

  ir_incoming_node&
  ir_incoming_node::
  operator= (ir_incoming_node&& other) noexcept
  {
    base::operator= (std::move (other));
    return *this;
  }

  void
  ir_incoming_node::
  set_parent (ir_def_timeline& dt) noexcept
  {
    m_parent.emplace (dt);
  }

  [[nodiscard]]
  ir_def_timeline&
  ir_incoming_node::
  get_parent (void) noexcept
  {
    return *m_parent;
  }

  [[nodiscard]]
  const ir_def_timeline&
  ir_incoming_node::
  get_parent (void) const noexcept
  {
    return as_mutable (*this).get_parent ();
  }

  ir_block&
  ir_incoming_node::
  get_parent_block (void) noexcept
  {
    return get_parent ().get_block ();
  }

  const ir_block&
  ir_incoming_node::
  get_parent_block (void) const noexcept
  {
    return get_parent ().get_block ();
  }

  auto
  ir_incoming_node::
  add_predecessor (ir_def_timeline& dt)
    -> iter
  {
    return base::bind (dt);
  }

  // return true if the removal caused a erasure
  auto
  ir_incoming_node::
  remove_predecessor (const ir_def_timeline& dt)
    -> iter
  {
    auto finder = [&dt](const ir_def_timeline& e) { return &e == &dt; };
    return erase (std::find_if (begin (), end (), finder));
  }

  void
  ir_incoming_node::
  swap (ir_incoming_node& other) noexcept
  {
    base::swap (other);
  }

}

/** ir-incoming-node.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "components/linkage/ir-incoming-node.hpp"

#include "components/linkage/ir-def-timeline.hpp"
#include "utilities/ir-utility.hpp"

namespace gch
{

  ir_incoming_node::
  ir_incoming_node (ir_def_timeline& parent, ir_incoming_node&& other) noexcept
    : base             (std::move (other)),
      m_parent_timeline (parent)
  { }

  ir_incoming_node::
  ir_incoming_node (ir_def_timeline& parent, nullopt_t)
    : m_parent_timeline (parent)
  { }

  ir_incoming_node::
  ir_incoming_node (ir_def_timeline& parent, ir_def_timeline& incoming)
    : base              (tag::bind, incoming),
      m_parent_timeline (parent)
  { }

  ir_incoming_node::
  ir_incoming_node (ir_def_timeline& parent, optional_ref<ir_def_timeline> opt_incoming)
    : m_parent_timeline (parent)
  {
    opt_incoming >>= [this](ir_def_timeline& dt) { rebind (dt); };
  }

  ir_incoming_node&
  ir_incoming_node::
  operator= (ir_incoming_node&& other) noexcept
  {
    base::operator= (std::move (other));
    return *this;
  }

  void
  ir_incoming_node::
  set_parent_timeline (ir_def_timeline& dt) noexcept
  {
    m_parent_timeline.emplace (dt);
  }

  [[nodiscard]]
  ir_def_timeline&
  ir_incoming_node::
  get_parent_timeline (void) noexcept
  {
    return *m_parent_timeline;
  }

  [[nodiscard]]
  const ir_def_timeline&
  ir_incoming_node::
  get_parent_timeline (void) const noexcept
  {
    return as_mutable (*this).get_parent_timeline ();
  }

  ir_block&
  ir_incoming_node::
  get_parent_block (void) noexcept
  {
    return get_parent_timeline ().get_block ();
  }

  const ir_block&
  ir_incoming_node::
  get_parent_block (void) const noexcept
  {
    return get_parent_timeline ().get_block ();
  }

  void
  ir_incoming_node::
  swap (ir_incoming_node& other) noexcept
  {
    base::swap (other);
  }

  void
  swap (ir_incoming_node& lhs, ir_incoming_node& rhs) noexcept
  {
    lhs.swap (rhs);
  }

}

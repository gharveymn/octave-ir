/** ir-parent-mutator.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/mutators/ir-parent-mutator.hpp"

#include "components/ir-component.hpp"

namespace gch
{

  ir_parent_mutator::
  ir_parent_mutator (ir_subcomponent& sub)
    : m_subcomponent (sub)
  { }

  ir_structure&
  ir_parent_mutator::
  get_parent (void) const noexcept
  {
    return get_subcomponent ().get_parent ();
  }

  ir_subcomponent&
  ir_parent_mutator::
  get_subcomponent (void) const noexcept
  {
    return m_subcomponent;
  }

}

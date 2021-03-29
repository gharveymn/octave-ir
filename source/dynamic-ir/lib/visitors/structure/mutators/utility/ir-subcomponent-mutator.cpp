/** ir-subcomponent-mutator.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/structure/mutators/utility/ir-subcomponent-mutator.hpp"

#include "components/ir-component.hpp"

namespace gch
{

  ir_subcomponent_mutator::
  ir_subcomponent_mutator (ir_subcomponent& sub)
    : m_subcomponent (sub)
  { }

  ir_structure&
  ir_subcomponent_mutator::
  get_parent (void) const noexcept
  {
    return get_subcomponent ().get_parent ();
  }

  ir_subcomponent&
  ir_subcomponent_mutator::
  get_subcomponent (void) const noexcept
  {
    return m_subcomponent;
  }

}

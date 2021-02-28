/** ir-parent-inspector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/structure/inspectors/utility/ir-subcomponent-inspector.hpp"

#include "components/ir-component.hpp"

namespace gch
{

  ir_subcomponent_inspector::
  ir_subcomponent_inspector (const ir_subcomponent& sub)
    : m_subcomponent (sub)
  { }

  const ir_structure&
  ir_subcomponent_inspector::
  get_parent (void) const noexcept
  {
    return get_subcomponent ().get_parent ();
  }

  const ir_subcomponent&
  ir_subcomponent_inspector::
  get_subcomponent (void) const noexcept
  {
    return m_subcomponent;
  }

}

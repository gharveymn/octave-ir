/** ir-parent-inspector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_PARENT_INSPECTOR_HPP
#define OCTAVE_IR_IR_PARENT_INSPECTOR_HPP

#include "components/ir-structure.hpp"

namespace gch
{

  class ir_structure;
  class ir_subcomponent;

  class ir_parent_inspector
  {
  public:
    ir_parent_inspector            (void)                           = delete;
    ir_parent_inspector            (const ir_parent_inspector&)     = delete;
    ir_parent_inspector            (ir_parent_inspector&&) noexcept = delete;
    ir_parent_inspector& operator= (const ir_parent_inspector&)     = delete;
    ir_parent_inspector& operator= (ir_parent_inspector&&) noexcept = delete;
    ~ir_parent_inspector           (void)                           = default;

    explicit
    ir_parent_inspector (ir_subcomponent& sub);

  protected:
    [[nodiscard]]
    ir_structure&
    get_parent (void) const noexcept;

    [[nodiscard]]
    ir_subcomponent&
    get_subcomponent (void) const noexcept;

  private:
    ir_subcomponent& m_subcomponent;
  };

}



#endif // OCTAVE_IR_IR_PARENT_INSPECTOR_HPP

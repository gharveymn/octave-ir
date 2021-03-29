/** ir-parent-inspector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_SUBCOMPONENT_INSPECTOR_HPP
#define OCTAVE_IR_IR_SUBCOMPONENT_INSPECTOR_HPP

namespace gch
{

  class ir_structure;
  class ir_subcomponent;

  class ir_subcomponent_inspector
  {
  public:
    ir_subcomponent_inspector            (void)                                 = delete;
    ir_subcomponent_inspector            (const ir_subcomponent_inspector&)     = delete;
    ir_subcomponent_inspector            (ir_subcomponent_inspector&&) noexcept = delete;
    ir_subcomponent_inspector& operator= (const ir_subcomponent_inspector&)     = delete;
    ir_subcomponent_inspector& operator= (ir_subcomponent_inspector&&) noexcept = delete;
    ~ir_subcomponent_inspector           (void)                                 = default;

    explicit
    ir_subcomponent_inspector (const ir_subcomponent& sub);

  protected:
    [[nodiscard]]
    const ir_structure&
    get_parent (void) const noexcept;

    [[nodiscard]]
    const ir_subcomponent&
    get_subcomponent (void) const noexcept;

  private:
    const ir_subcomponent& m_subcomponent;
  };

}



#endif // OCTAVE_IR_IR_SUBCOMPONENT_INSPECTOR_HPP

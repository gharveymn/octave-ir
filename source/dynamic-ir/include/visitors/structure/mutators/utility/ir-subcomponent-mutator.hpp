/** ir-subcomponent-mutator.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_SUBCOMPONENT_MUTATOR_HPP
#define OCTAVE_IR_IR_SUBCOMPONENT_MUTATOR_HPP

namespace gch
{

  class ir_structure;
  class ir_subcomponent;

  class ir_subcomponent_mutator
  {
  public:
    ir_subcomponent_mutator            (void)                               = delete;
    ir_subcomponent_mutator            (const ir_subcomponent_mutator&)     = delete;
    ir_subcomponent_mutator            (ir_subcomponent_mutator&&) noexcept = delete;
    ir_subcomponent_mutator& operator= (const ir_subcomponent_mutator&)     = delete;
    ir_subcomponent_mutator& operator= (ir_subcomponent_mutator&&) noexcept = delete;
    ~ir_subcomponent_mutator           (void)                               = default;

    explicit
    ir_subcomponent_mutator (ir_subcomponent& sub);

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

#endif //

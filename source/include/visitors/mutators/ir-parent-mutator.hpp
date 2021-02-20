/** ir-parent-mutator.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_PARENT_MUTATOR_HPP
#define OCTAVE_IR_IR_PARENT_MUTATOR_HPP

namespace gch
{

  class ir_structure;
  class ir_subcomponent;

  class ir_parent_mutator
  {
  public:
    ir_parent_mutator            (void)                         = delete;
    ir_parent_mutator            (const ir_parent_mutator&)     = delete;
    ir_parent_mutator            (ir_parent_mutator&&) noexcept = delete;
    ir_parent_mutator& operator= (const ir_parent_mutator&)     = delete;
    ir_parent_mutator& operator= (ir_parent_mutator&&) noexcept = delete;
    ~ir_parent_mutator           (void)                         = default;

    explicit
    ir_parent_mutator (ir_subcomponent& sub);

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

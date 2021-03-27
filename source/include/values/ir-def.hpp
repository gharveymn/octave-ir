/** ir-def.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_DEF_HPP
#define OCTAVE_IR_IR_DEF_HPP

#include <gch/nonnull_ptr.hpp>

#include <string_view>

namespace gch
{

  class ir_instruction;
  class ir_type;
  class ir_variable;

  //! An ssa variable def. It holds very little information about itself,
  //! it's more of just an indicating stub for the variable.
  class ir_def
  {
  public:
    using id_type = int;

    ir_def            (void)                = delete;
    ir_def            (const ir_def&)       = delete;
    ir_def            (ir_def&& d) noexcept = delete;
//  ir_def& operator= (const ir_def&)       = impl;
//  ir_def& operator= (ir_def&&)   noexcept = impl;
    ~ir_def           (void)                = default;

    ir_def (ir_instruction& instr, ir_variable& var) noexcept;

    ir_def (ir_instruction& new_instr, const ir_def& other) noexcept;

    ir_def (ir_instruction& new_instr, ir_def&& other) noexcept;

    ir_def&
    operator= (const ir_def& other) noexcept;

    ir_def&
    operator= (ir_def&& other) noexcept;

    [[nodiscard]]
    ir_variable&
    get_variable (void) noexcept;

    [[nodiscard]]
    const ir_variable&
    get_variable (void) const noexcept;

    [[nodiscard]]
    id_type
    get_id (void) const noexcept;

    [[nodiscard]]
    std::string_view
    get_variable_name (void) const noexcept;

    [[nodiscard]]
    ir_type
    get_type (void) const noexcept;

    [[nodiscard]]
    ir_instruction&
    get_instruction (void) noexcept;

    [[nodiscard]]
    const ir_instruction&
    get_instruction (void) const noexcept;

    void
    set_instruction (ir_instruction& instr) noexcept;

  private:
    nonnull_ptr<ir_instruction> m_instruction;
    nonnull_ptr<ir_variable>    m_variable;
    id_type                     m_id;
  };

}

#endif // OCTAVE_IR_IR_DEF_HPP

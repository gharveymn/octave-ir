/** ir-def.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "values/ir-def.hpp"

#include "gch/octave-static-ir/ir-type.hpp"
#include "values/ir-variable.hpp"

namespace gch
{

  static_assert (std::is_same_v<ir_def::id_type, ir_variable::id_type>, "id types should match");

  ir_def::
  ir_def (ir_instruction& instr, ir_variable& var) noexcept
    : m_instruction { instr },
      m_variable    { var },
      m_id          { var.create_id () }
  { }

  ir_def::
  ir_def (ir_instruction& new_instr, const ir_def& other) noexcept
    : m_instruction { new_instr   },
      m_variable    { other.m_variable },
      m_id          { other.m_id  }
  { }

  ir_def::
  ir_def (ir_instruction& new_instr, ir_def&& other) noexcept
    : ir_def (new_instr, other)
  { }

  ir_def&
  ir_def::
  operator= (const ir_def& other) noexcept
  {
    if (&other != this)
    {
      m_variable = other.m_variable;
      m_id       = other.m_id;
    }
    return *this;
  }

  ir_def&
  ir_def::
  operator= (ir_def&& other) noexcept
  {
    return *this = other;
  }

  ir_variable&
  ir_def::
  get_variable (void) noexcept
  {
    return *m_variable;
  }

  const ir_variable&
  ir_def::
  get_variable (void) const noexcept
  {
    return *m_variable;
  }

  auto
  ir_def::
  get_id (void) const noexcept
    -> id_type
  {
    return m_id;
  }

  std::string_view
  ir_def::
  get_variable_name (void) const noexcept
  {
    return get_variable ().get_name ();
  }

  ir_type
  ir_def::
  get_type (void) const noexcept
  {
    return get_variable ().get_type ();
  }

  ir_instruction&
  ir_def::
  get_instruction (void) noexcept
  {
    return *m_instruction;
  }

  const ir_instruction&
  ir_def::
  get_instruction (void) const noexcept
  {
    return *m_instruction;
  }

  void
  ir_def::
  set_instruction (ir_instruction& instr) noexcept
  {
    m_instruction.emplace (instr);
  }

}

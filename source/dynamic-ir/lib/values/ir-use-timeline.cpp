/** ir-use-timeline.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "values/ir-use-timeline.hpp"

#include "gch/octave-ir-utilities/ir-utility.hpp"
#include "values/ir-def.hpp"
#include "values/ir-instruction.hpp"

namespace gch
{

  ir_def&
  ir_use_timeline::
  get_def (void) noexcept
  {
    return get_def_instruction ().get_def ();
  }

  const ir_def&
  ir_use_timeline::
  get_def (void) const noexcept
  {
    return as_mutable (*this).get_def ();
  }

  ir_instruction_iter
  ir_use_timeline::
  get_def_pos (void) noexcept
  {
    return m_instruction_pos;
  }

  ir_instruction_citer
  ir_use_timeline::
  get_def_pos (void) const noexcept
  {
    return as_mutable (*this).get_def_pos ();
  }

  ir_instruction&
  ir_use_timeline::
  get_def_instruction (void) noexcept
  {
    return *get_def_pos ();
  }

  const ir_instruction&
  ir_use_timeline::
  get_def_instruction (void) const noexcept
  {
    return as_mutable (*this).get_def_instruction ();
  }

  ir_variable&
  ir_use_timeline::
  get_variable (void) noexcept
  {
    return get_def ().get_variable ();
  }

  const ir_variable&
  ir_use_timeline::
  get_variable (void) const noexcept
  {
    return as_mutable (*this).get_variable ();
  }

  void
  ir_use_timeline::
  set_instruction_pos (const ir_instruction_iter instr) noexcept
  {
    m_instruction_pos = instr;
  }

  bool
  has_same_def (const ir_use_timeline& l, const ir_use_timeline& r) noexcept
  {
    return l.get_def_pos () == r.get_def_pos ();
  }

  bool
  has_same_def (optional_cref<ir_use_timeline> lhs, const ir_use_timeline& r) noexcept
  {
    return lhs >>= [&r](const ir_use_timeline& l) noexcept { return has_same_def (l, r); };
  }

  bool
  has_same_def (const ir_use_timeline& l, optional_cref<ir_use_timeline> rhs) noexcept
  {
    return has_same_def (rhs, l);
  }

  bool
  has_same_def (optional_cref<ir_use_timeline> lhs, optional_cref<ir_use_timeline> rhs) noexcept
  {
    return lhs >>= [rhs](const ir_use_timeline& l) noexcept { return has_same_def (l, rhs); };
  }

}

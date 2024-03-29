/** ir-use.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-use.hpp"

#include "ir-def.hpp"
#include "ir-use-timeline.hpp"

#include "ir-type.hpp"
#include "ir-utility.hpp"

#include "ir-variable.hpp"

namespace gch
{

  //
  // ir_use
  //

  ir_use::
  ir_use (ir_instruction& instr, ir_use_timeline& ut, remote_iterator pos)
    : reporter_type (tag::bind, ut, pos),
      m_instr (instr)
  { }

  ir_use::
  ir_use (ir_instruction& instr, const ir_use_info& info)
    : ir_use (instr, info.get_timeline (), info.get_insertion_position ())
  { }

  ir_use_timeline&
  ir_use::
  get_timeline (void) noexcept
  {
    return get_remote ();
  }

  const ir_use_timeline&
  ir_use::
  get_timeline (void) const noexcept
  {
    return as_mutable (*this).get_timeline ();
  }

  ir_def&
  ir_use::
  get_def (void) noexcept
  {
    return get_timeline ().get_def ();
  }

  const ir_def&
  ir_use::
  get_def (void) const noexcept
  {
    return as_mutable (*this).get_def ();
  }

  ir_variable&
  ir_use::
  get_variable (void) noexcept
  {
    return get_timeline ().get_variable ();
  }

  const ir_variable&
  ir_use::
  get_variable (void) const noexcept
  {
    return as_mutable (*this).get_variable ();
  }

  ir_type
  ir_use::
  get_type (void) const noexcept
  {
    return get_def ().get_type ();
  }

  std::size_t
  ir_use::
  get_id (void) const noexcept
  {
    return get_position ();
  }

  std::string_view
  ir_use::
  get_variable_name (void) const
  {
    return get_variable ().get_name ();
  }

  [[nodiscard]]
  ir_instruction&
  ir_use::
  get_instruction (void) noexcept
  {
    return *m_instr;
  }

  [[nodiscard]]
  const ir_instruction&
  ir_use::
  get_instruction (void) const noexcept
  {
    return as_mutable (*this).get_instruction ();
  }

  void
  ir_use::
  set_instruction (ir_instruction& instr) noexcept
  {
    m_instr.emplace (instr);
  }

  //
  // ir_use_info
  //

  ir_use_info::
  ir_use_info (ir_use_timeline& ut, remote_iterator pos)
    : m_timeline           (ut),
      m_insertion_position (pos)
  { }

  ir_use_timeline&
  ir_use_info::
  get_timeline (void) const noexcept
  {
    return *m_timeline;
  }

  auto
  ir_use_info::
  get_insertion_position (void) const noexcept
    -> remote_iterator
  {
    return m_insertion_position;
  }

}

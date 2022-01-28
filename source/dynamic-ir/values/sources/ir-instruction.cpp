/** ir-instruction.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-block.hpp"
#include "ir-instruction.hpp"
#include "ir-optional-util.hpp"
#include "ir-variable.hpp"
#include <list>

namespace gch
{

  //
  // ir_instruction
  //

  ir_instruction::
  ir_instruction (ir_instruction&& other) noexcept
    : m_metadata (other.m_metadata),
      m_def      (std::move (other.m_def)),
      m_args     (std::move (other.m_args))
  {
    m_def >>= [&](ir_def& def) noexcept { def.set_instruction (*this); };
    for (ir_operand& arg : m_args)
      maybe_get<ir_use> (arg) >>= [&](ir_use& use) noexcept { use.set_instruction (*this); };
  }

  ir_instruction&
  ir_instruction::
  operator= (ir_instruction&& other) noexcept
  {
    m_metadata = other.m_metadata;

    if (other.m_def)
      m_def.emplace (*this, std::move (*other.m_def));
    else
      m_def.reset ();

    set_args (std::move (other.m_args));
    return *this;
  }

  auto
  ir_instruction::
  begin (void) noexcept
    -> iter
  {
    return m_args.begin ();
  }

  auto
  ir_instruction::
  begin (void) const noexcept
    -> citer
  {
    return as_mutable (*this).begin ();
  }

  auto
  ir_instruction::
  cbegin (void) const noexcept
    -> citer
  {
    return begin ();
  }

  auto
  ir_instruction::
  end (void) noexcept
    -> iter
  {
    return m_args.end ();
  }

  auto
  ir_instruction::
  end (void) const noexcept
    -> citer
  {
    return as_mutable (*this).end ();
  }

  auto
  ir_instruction::
  cend (void) const noexcept
    -> citer
  {
    return end ();
  }

  auto
  ir_instruction::
  rbegin (void) noexcept
    -> riter
  {
    return m_args.rbegin ();
  }

  auto
  ir_instruction::
  rbegin (void) const noexcept
    -> criter
  {
    return as_mutable (*this).rbegin ();
  }

  auto
  ir_instruction::
  crbegin (void) const noexcept
    -> criter
  {
    return rbegin ();
  }

  auto
  ir_instruction::
  rend (void) noexcept
    -> riter
  {
    return m_args.rend ();
  }

  auto
  ir_instruction::
  rend (void) const noexcept
    -> criter
  {
    return as_mutable (*this).rend ();
  }

  auto
  ir_instruction::
  crend (void) const noexcept
    -> criter
  {
    return rend ();
  }

  auto
  ir_instruction::
  front (void)
    -> ref
  {
    return *begin ();
  }

  auto
  ir_instruction::
  front (void) const
    -> cref
  {
    return as_mutable (*this).front ();
  }

  auto
  ir_instruction::
  back (void)
    -> ref
  {
    return *rbegin ();
  }

  auto
  ir_instruction::
  back (void) const
    -> cref
  {
    return as_mutable (*this).back ();
  }

  auto
  ir_instruction::
  size (void) const noexcept
    -> size_ty
  {
   return m_args.size ();
  }

  bool
  ir_instruction::
  empty (void) const noexcept
  {
    return m_args.empty ();
  }

  auto
  ir_instruction::
  erase (citer pos)
    -> iter
  {
    return m_args.erase (pos);
  }

  auto
  ir_instruction::
  erase (citer first, citer last)
    -> iter
  {
    return m_args.erase (first, last);
  }

  void
  ir_instruction::
  set_args (args_container_type&& args)
  {
    m_args = std::move (args);
    for (ir_operand& arg : m_args)
      maybe_get<ir_use> (arg) >>= [&](ir_use& use) noexcept { use.set_instruction (*this); };
  }

  ir_metadata
  ir_instruction::
  get_metadata (void) const noexcept
  {
    return m_metadata;
  }

  bool
  ir_instruction::
  has_def (void) const noexcept
  {
    return m_def.has_value ();
  }

  ir_def&
  ir_instruction::
  get_def (void) noexcept
  {
    return *m_def;
  }

  const ir_def&
  ir_instruction::
  get_def (void) const noexcept
  {
    return *m_def;
  }

  optional_ref<ir_def>
  ir_instruction::
  maybe_get_def (void) noexcept
  {
    return m_def >>= identity { };
  }

  optional_cref<ir_def>
  ir_instruction::
  maybe_get_def (void) const noexcept
  {
    return m_def >>= identity { };
  }

  bool
  has_def (const ir_instruction& instr) noexcept
  {
    return instr.has_def ();
  }

  ir_def&
  get_def (ir_instruction& instr) noexcept
  {
    return instr.get_def ();
  }

  const ir_def&
  get_def (const ir_instruction& instr) noexcept
  {
    return instr.get_def ();
  }

  optional_ref<ir_def>
  maybe_get_def (ir_instruction& instr) noexcept
  {
    return instr.maybe_get_def ();
  }

  optional_cref<ir_def>
  maybe_get_def (const ir_instruction& instr) noexcept
  {
    return instr.maybe_get_def ();
  }

}

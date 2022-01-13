/** ir-static-block.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-static-block.hpp"

#include "ir-static-instruction.hpp"

#include <numeric>
#include <ostream>

namespace gch
{

  auto
  ir_static_block::
  begin (void) const noexcept
    -> citer
  {
    return m_instructions.begin ();
  }

  auto
  ir_static_block::
  end (void) const noexcept
    -> citer
  {
    return m_instructions.end ();
  }

  auto
  ir_static_block::
  rbegin (void) const noexcept
    -> criter
  {
    return m_instructions.rbegin ();
  }

  auto
  ir_static_block::
  rend (void) const noexcept
    -> criter
  {
    return m_instructions.rend ();
  }

  auto
  ir_static_block::
  front (void) const
    -> cref
  {
    return m_instructions.front ();
  }

  auto
  ir_static_block::
  back (void) const
    -> cref
  {
    return m_instructions.back ();
  }

  auto
  ir_static_block::
  size (void) const noexcept
    -> size_ty
  {
   return m_instructions.size ();
  }

  bool
  ir_static_block::
  empty (void) const noexcept
  {
    return m_instructions.empty ();
  }

  auto
  ir_static_block::
  operator[] (size_type n) const
    -> const_reference
  {
    return m_instructions[n];
  }

  void
  ir_static_block::
  push_back (ir_static_instruction&& instr)
  {
    m_instructions.emplace_back (std::move (instr));
  }

  std::ostream&
  operator<< (std::ostream& out, const ir_static_block& block)
  {
    out << block[0];
    std::for_each (std::next (block.begin ()), block.end (),
                   [&](const ir_static_instruction& instr) { out << '\n' << instr; });
    return out;
  }

}

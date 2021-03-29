/** ir-static-block.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-static-ir/ir-static-block.hpp"

#include "gch/octave-static-ir/ir-static-instruction.hpp"

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

}

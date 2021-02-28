/*

Copyright (C) 2019 Gene Harvey

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#include "components/ir-block.hpp"
#include "values/ir-instruction.hpp"
#include "utilities/ir-optional-util.hpp"
#include "values/ir-variable.hpp"
#include <algorithm>
#include <list>

namespace gch
{

  //
  // ir_instruction
  //

  ir_instruction::
  ir_instruction (ir_instruction&& other) noexcept
    : m_metadata (other.m_metadata),
      m_def (other.m_def ? std::optional<ir_def> (std::in_place,
                                                  std::move (*other.m_def), *this)
                         : std::nullopt),
      m_args     (std::move (other.m_args))
  {
    std::for_each (m_args.begin (), m_args.end (),
                   [this](ir_operand& arg)
                   {
                     maybe_get<ir_use> (arg) >>= [this](ir_use& u) { u.set_instruction (*this); };
                   });
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
  set_def (std::optional<ir_def>&& def)
  {
    (m_def = std::move (def)) >>= [this] (ir_def& d) { d.set_instruction (*this); };
  }

  void
  ir_instruction::
  set_args (args_container_type&& args)
  {
    m_args = std::move (args);
    std::for_each (m_args.begin (), m_args.end (),
                   [this](ir_operand& arg)
                   {
                     maybe_get<ir_use> (arg) >>= [this](ir_use& u) { u.set_instruction (*this); };
                   });
  }

  auto
  ir_instruction::
  get_metadata (void) const noexcept
    -> metadata_t
  {
    return m_metadata;
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

}
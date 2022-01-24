/** ir-static-instruction.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-static-instruction.hpp"

#include "ir-optional-util.hpp"

#include <numeric>
#include <ostream>

namespace gch
{
  ir_static_instruction::
  ir_static_instruction (ir_metadata m, ir_static_def def, args_container_type&& args) noexcept
    : m_metadata (m),
      m_def (def),
      m_args (std::move (args))
  {
    assert (m.has_def ());
  }

  ir_static_instruction::
  ir_static_instruction (ir_metadata m, args_container_type&& args) noexcept
    : m_metadata (m),
      m_args (std::move (args))
  {
    // assert (! m.has_def ());
  }

  ir_static_instruction::
  ir_static_instruction (ir_metadata m) noexcept
    : ir_static_instruction (m, { })
  { }

  auto
  ir_static_instruction::
  begin (void) const noexcept
    -> citer
  {
    return m_args.begin ();
  }

  auto
  ir_static_instruction::
  end (void) const noexcept
    -> citer
  {
    return m_args.end ();
  }

  auto
  ir_static_instruction::
  rbegin (void) const noexcept
    -> criter
  {
    return m_args.rbegin ();
  }

  auto
  ir_static_instruction::
  rend (void) const noexcept
    -> criter
  {
    return m_args.rend ();
  }

  auto
  ir_static_instruction::
  front (void) const
    -> cref
  {
    return m_args.front ();
  }

  auto
  ir_static_instruction::
  back (void) const
    -> cref
  {
    return m_args.back ();
  }

  auto
  ir_static_instruction::
  size (void) const noexcept
    -> size_ty
  {
   return m_args.size ();
  }

  bool
  ir_static_instruction::
  empty (void) const noexcept
  {
    return m_args.empty ();
  }

  auto
  ir_static_instruction::
  num_args (void) const noexcept
    -> size_ty
  {
   return size ();
  }

  bool
  ir_static_instruction::
  has_args (void) const noexcept
  {
    return ! empty ();
  }

  auto
  ir_static_instruction::
  operator[] (size_type n) const
    -> const_reference
  {
    return m_args[n];
  }

  ir_metadata
  ir_static_instruction::
  get_metadata (void) const noexcept
  {
    return m_metadata;
  }

  bool
  ir_static_instruction::
  has_def (void) const noexcept
  {
    return m_def.has_value ();
  }

  const ir_static_def&
  ir_static_instruction::
  get_def (void) const
  {
    return *m_def;
  }

  const std::optional<ir_static_def>&
  ir_static_instruction::
  maybe_get_def (void) const noexcept
  {
    return m_def;
  }

}

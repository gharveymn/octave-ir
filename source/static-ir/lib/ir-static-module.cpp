/** ir-static-module.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-static-ir/ir-static-module.hpp"

#include "gch/octave-static-ir/ir-static-block.hpp"
#include "gch/octave-static-ir/ir-static-instruction.hpp"
#include "gch/octave-static-ir/ir-static-variable.hpp"

namespace gch
{

  ir_static_module::
  ir_static_module (std::string_view name, ir_processed_id id, container_type&& blocks,
                    std::vector<ir_static_variable>&& vars)
    : m_name   (name),
      m_id     (id),
      m_blocks (std::move (blocks))
  { }

  auto
  ir_static_module::
  begin (void) const noexcept
    -> citer
  {
    return m_blocks.begin ();
  }

  auto
  ir_static_module::
  end (void) const noexcept
    -> citer
  {
    return m_blocks.end ();
  }

  auto
  ir_static_module::
  rbegin (void) const noexcept
    -> criter
  {
    return m_blocks.rbegin ();
  }

  auto
  ir_static_module::
  rend (void) const noexcept
    -> criter
  {
    return m_blocks.rend ();
  }

  auto
  ir_static_module::
  front (void) const
    -> cref
  {
    return m_blocks.front ();
  }

  auto
  ir_static_module::
  back (void) const
    -> cref
  {
    return m_blocks.back ();
  }

  auto
  ir_static_module::
  size (void) const noexcept
    -> size_ty
  {
   return m_blocks.size ();
  }

  bool
  ir_static_module::
  empty (void) const noexcept
  {
    return m_blocks.empty ();
  }

}

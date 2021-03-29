/** ir-static-instruction.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-static-ir/ir-static-instruction.hpp"

#include "gch/octave-ir-utilities/ir-optional-util.hpp"
// #include "values/ir-instruction.hpp"
// #include "visitors/component/inspectors/ir-static-module-generator.hpp"

namespace gch
{
  // ir_static_instruction::
  // ir_static_instruction (const ir_instruction& instr, const ir_static_variable_map& var_map)
  //   : m_metadata (instr.get_metadata ()),
  //     m_def (maybe_get_def (instr) >>= [&](const ir_def& def) -> std::optional<ir_static_def>
  //                                           { return { var_map.create_static_def (def) }; })
  // {
  //   std::transform (instr.begin (), instr.end (), std::back_inserter (m_args),
  //                   [&](const ir_operand& op) -> ir_static_operand
  //                   {
  //                     if (optional_ref use { maybe_get<ir_use> (op) })
  //                       return var_map.create_static_use (*use);
  //                     return get<ir_constant> (op);
  //                   });
  // }

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
  get_metadata (void) const noexcept
  -> metadata_t
  {
    return m_metadata;
  }

  const ir_static_def&
  ir_static_instruction::
  get_def (void) const noexcept
  {
    return *m_def;
  }

}

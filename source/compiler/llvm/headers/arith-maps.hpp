/** arith-maps.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_ARITH_MAPS_HPP
#define OCTAVE_IR_COMPILER_LLVM_ARITH_MAPS_HPP

#include "arith-mappers.hpp"

#include "ir-type-util.hpp"

namespace gch
{

  template <ir_opcode Op>
  inline constexpr
  auto
  llvm_arithmetic_creator_map = generate_ir_type_map<llvm_arith_mapper<Op>::template mapper> ();

}

#endif // OCTAVE_IR_COMPILER_LLVM_ARITH_MAPS_HPP

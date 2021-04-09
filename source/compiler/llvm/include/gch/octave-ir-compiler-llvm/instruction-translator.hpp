/** instruction-translator.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_INSTRUCTION_TRANSLATOR_HPP
#define OCTAVE_IR_COMPILER_LLVM_INSTRUCTION_TRANSLATOR_HPP

#include "instruction-maps/llvm-instruction-maps.hpp"
#include "llvm-common.hpp"
#include "llvm-interface.hpp"
#include "llvm-value-map.hpp"

#include "gch/octave-ir-static-ir/ir-metadata.hpp"
#include "gch/octave-ir-static-ir/ir-static-def.hpp"
#include "gch/octave-ir-static-ir/ir-static-instruction.hpp"

#ifdef _MSC_VER
#  pragma warning (push, 0)
#endif

#include <llvm/IR/IRBuilder.h>

#ifdef _MSC_VER
#  pragma warning (pop)
#endif

namespace gch
{
  template <ir_opcode Op, typename Enable = void>
  struct instruction_translator
  {
    static
    llvm::Value *
    translate (const ir_static_instruction&, llvm::IRBuilder<>&, llvm_value_map&)
    {
      throw std::logic_error { std::string ("No mapping available for instruction of type ")
                               + ir_metadata_v<Op>.get_name () };
    }
  };

  template <>
  struct instruction_translator<ir_opcode::phi>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm::IRBuilder<>&           builder,
               llvm_value_map&              value_map)
    {
      const ir_static_def& def           = instr.get_def ();
      unsigned             num_incoming  = static_cast<unsigned> (instr.num_args ()) / 2;

      llvm::Type& llvm_def_ty   = value_map[def.get_type ()];
      llvm::Twine llvm_def_name = create_twine (def.get_variable_name ());

      llvm::PHINode& llvm_phi = *builder.CreatePHI (&llvm_def_ty, num_incoming, llvm_def_name);
      return &llvm_phi;
    }
  };

  template <ir_opcode Op>
  struct instruction_translator<Op, std::enable_if_t<is_comparison_op_v<Op>>>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm::IRBuilder<>&           builder,
               llvm_value_map&              value_map)
    {
      const ir_static_def& def          = instr.get_def ();
      const ir_type        ty           = def.get_type ();
      auto                 creator_func = llvm_comparison_creator_map<Op>[ty];
      return creator_func (builder,
                           &value_map[instr[0]],
                           &value_map[instr[1]],
                           create_twine (def.get_variable_name ()));
    }
  };

  template <ir_opcode Op>
  struct instruction_translator_mapper
  {
    constexpr
    auto
    operator() (void) const noexcept
    {
      return &instruction_translator<Op>::translate;
    }
  };

  static constexpr
  auto
  translator_map { ir_metadata::generate_map<instruction_translator_mapper> () };

}

#endif // OCTAVE_IR_COMPILER_LLVM_INSTRUCTION_TRANSLATOR_HPP

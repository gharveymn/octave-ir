/** function-translator.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "function-translator.hpp"

#include "instruction-translator.hpp"
#include "llvm-interface.hpp"
#include "llvm-value-map.hpp"
#include "llvm-version.hpp"

#include "gch/octave-ir-compiler-interface.hpp"
#include "ir-static-block.hpp"
#include "ir-static-instruction.hpp"
#include "ir-static-variable.hpp"
#include "ir-type-util.hpp"

#include <iostream>
#include <octave/oct.h>
#include <octave/octave.h>
#include <octave/parse.h>
#include <octave/interpreter.h>

#include <gch/nonnull_ptr.hpp>

#include <unordered_map>
#include <vector>

namespace gch
{

  static
  llvm::BasicBlock&
  translate_block (const ir_static_block& block, llvm_value_map& value_map)
  {
    llvm::BasicBlock& llvm_block = value_map[block];
    llvm::IRBuilder<> block_builder (&llvm_block);

    std::for_each (block.begin (), block.end (),
                   [&](const ir_static_instruction& instr)
                   {
                     translator_map[instr.get_metadata ()] (instr, block_builder, value_map);
                   });

    return llvm_block;
  }

  static
  llvm::Function&
  translate_function (const ir_static_function& func, llvm::orc::ThreadSafeModule& llvm_module)
  {
    llvm::FunctionType *llvm_function_ty = nullptr;
    llvm::Function& out_func = *llvm_module.withModuleDo (
      [&] (llvm::Module& module)
      {
        return llvm::Function::Create (llvm_function_ty,
                                       llvm::Function::ExternalLinkage,
                                       create_twine (func.get_name ()),
                                       module);
      });

    llvm_value_map value_map { llvm_module, out_func, func };
    std::for_each (func.begin (), func.end (),
                   [&](const ir_static_block& block) { translate_block (block, value_map); });

    // resolve the phi nodes
    std::for_each (func.begin (), func.end (),
      [&](const ir_static_block& block)
      {
        auto instr_it  = block.begin ();
        auto phi_range = value_map[block].phis ();
        std::for_each (phi_range.begin (), phi_range.end (),
          [&](llvm::PHINode& phi_node)
          {
            assert (is_a<ir_opcode::phi> (*instr_it));
            assert (instr_it->num_args () % 2 == 0);

            for (auto op_it = instr_it->begin (); op_it != instr_it->end ();)
            {
              const ir_static_operand& value_op = *op_it++;
              const ir_static_operand& block_op = *op_it++;

              assert (is_use (value_op));
              const ir_static_use& value = as_use (value_op);

              assert (is_constant (block_op));
              const ir_constant& block_id_c = as_constant (block_op);

              assert (is_a<ir_static_block_id::value_type> (block_id_c));
              ir_static_block_id block_id { as<ir_static_block_id::value_type> (block_id_c) };

              phi_node.addIncoming (&value_map[value], &value_map[block_id]);
            }
          });
      });
    return out_func;
  }

  std::filesystem::path
  compile (const ir_static_function& func)
  {

    std::vector<int> x;
    llvm::SmallVector<std::string, 2> v;
    v.emplace_back ("hi");
    return v[0];
  }

}

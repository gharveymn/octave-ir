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

#include "gch/octave-ir-compiler-interface.hpp"
#include "ir-static-block.hpp"
#include "ir-static-instruction.hpp"
#include "ir-static-variable.hpp"

#include <gch/nonnull_ptr.hpp>

#include <iostream>
#include <vector>

namespace gch
{

  static
  llvm::BasicBlock&
  translate_block (const ir_static_block& block, llvm_value_map& value_map)
  {
    llvm::BasicBlock& llvm_block = value_map[block];
    llvm_ir_builder_type block_builder (&llvm_block);

    std::for_each (block.begin (), block.end (), [&](const ir_static_instruction& instr) {
      try
      {
        llvm::Value *val = translator_map[instr.get_metadata ()] (instr, block_builder, value_map);
        if (instr.has_def ())
          value_map.register_def (instr.get_def (), *val);
      }
      catch (std::exception& e)
      {
        std::cerr << e.what () << std::endl;
        throw;
      }
    });

    return llvm_block;
  }

  static
  llvm::Function&
  translate_function (const ir_static_function& func, llvm::orc::ThreadSafeModule& llvm_tsm)
  {
    llvm_module_interface module_interface (llvm_tsm);

    llvm::SmallVector<llvm::Type *> arg_types;
    std::transform (func.args_begin (), func.args_end (), std::back_inserter (arg_types),
                    [&](const ir_static_variable_id& id) {
      return &module_interface.get_llvm_type (func.get_type (id));
    });

    llvm::Type& ret_type = module_interface.get_llvm_type (
      func.has_returns () ? func.get_type (*func.returns_begin ()) : ir_type_v<void>);

    llvm::FunctionType *llvm_function_ty = llvm::FunctionType::get (&ret_type, arg_types, false);

    llvm::Function& out_func = *module_interface.invoke_with_module ([&](llvm::Module& module) {
      return llvm::Function::Create (
        llvm_function_ty,
        llvm::Function::ExternalLinkage,
        create_twine (func.get_name ()),
        module);
    });

    llvm_value_map value_map { module_interface, out_func, func };

    std::for_each (func.begin (), func.end (), [&](const ir_static_block& block) {
      translate_block (block, value_map);
    });

    // resolve the phi nodes
    std::for_each (func.begin (), func.end (), [&](const ir_static_block& block) {
      auto instr_it  = block.begin ();
      auto phi_range = value_map[block].phis ();

      std::for_each (phi_range.begin (), phi_range.end (), [&](llvm::PHINode& phi_node) {
        assert (is_a<ir_opcode::phi> (*instr_it));
        assert (instr_it->num_args () % 2 == 0);

        for (auto op_it = instr_it->begin (); op_it != instr_it->end (); ++op_it)
        {
          const ir_static_operand& block_op = *op_it++;
          const ir_static_operand& value_op = *op_it;

          assert (is_constant (block_op));
          const ir_constant& block_id_c = as_constant (block_op);

          assert (is_use (value_op));
          const ir_static_use& value = as_use (value_op);

          assert (is_a<ir_static_block_id> (block_id_c));
          ir_static_block_id block_id { as<ir_static_block_id> (block_id_c) };

          phi_node.addIncoming (&value_map[value], &value_map[block_id]);
        }

        assert (block.end () != instr_it);

        ++instr_it;
      });

      assert (block.end () == instr_it || ! is_a<ir_opcode::phi> (*instr_it));

    });

    return out_func;
  }

  llvm::orc::ThreadSafeModule
  create_llvm_module (const llvm::DataLayout& data_layout, const ir_static_function& func)
  {
    auto llvm_context = std::make_unique<llvm::LLVMContext> ();
    auto llvm_module  = std::make_unique<llvm::Module> ("my jit", *llvm_context);
    llvm_module->setDataLayout (data_layout);
    llvm::orc::ThreadSafeModule llvm_tsm (std::move (llvm_module), std::move (llvm_context));
    translate_function (func, llvm_tsm);
    return llvm_tsm;
  }

}

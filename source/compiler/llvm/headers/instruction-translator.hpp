/** instruction-translator.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_INSTRUCTION_TRANSLATOR_HPP
#define OCTAVE_IR_COMPILER_LLVM_INSTRUCTION_TRANSLATOR_HPP

#include "llvm-common.hpp"
#include "llvm-interface.hpp"
#include "llvm-value-map.hpp"

#include "mappers/arithmetic-mappers.hpp"
#include "mappers/bitwise-mappers.hpp"
#include "mappers/relation-mappers.hpp"

#include "ir-metadata.hpp"
#include "ir-optional-util.hpp"
#include "ir-static-block.hpp"
#include "ir-static-def.hpp"
#include "ir-static-instruction.hpp"

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/IR/IRBuilder.h>

GCH_ENABLE_WARNINGS_MSVC

namespace gch
{
  template <ir_opcode Op, typename Enable = void>
  struct instruction_translator
  {
    static
    llvm::Value *
    translate (const ir_static_instruction&, llvm_ir_builder_type&, llvm_value_map&)
    {
      throw std::logic_error {
        std::string ("No mapping available for instruction of type ")
        + ir_metadata_v<Op>.get_name ()
      };
    }
  };

  template <ir_opcode Op>
  struct instruction_translator<Op, std::enable_if_t<ir_instruction_traits<Op>::is_abstract>>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction&, llvm_ir_builder_type&, llvm_value_map&)
    {
      throw std::logic_error {
        "Cannot translate abstract instruction (it should not have been instantiated)."
      };
    }
  };

  template <>
  struct instruction_translator<ir_opcode::phi>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      const ir_static_def& def           = instr.get_def ();
      unsigned             num_incoming  = static_cast<unsigned> (instr.num_args ()) / 2;

      llvm::Type& llvm_def_ty   = value_map.get_llvm_type (def);
      llvm::Twine llvm_def_name = value_map.get_variable_name (def);

      return builder.CreatePHI (&llvm_def_ty, num_incoming, llvm_def_name);
    }
  };

  template <>
  struct instruction_translator<ir_opcode::cbranch>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      return builder.CreateCondBr (
        &value_map[instr[0]],
        &value_map[as_constant<ir_static_block_id> (instr[1])],
        &value_map[as_constant<ir_static_block_id> (instr[2])]);
    }
  };

  template <>
  struct instruction_translator<ir_opcode::ucbranch>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      return builder.CreateBr (&value_map[as_constant<ir_static_block_id> (instr[0])]);
    }
  };

  template <>
  struct instruction_translator<ir_opcode::assign>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      const ir_static_def& def = instr.get_def ();
      llvm::Value& var = value_map[def];

      builder.CreateStore (&value_map[instr[0]], &var);

      return builder.CreateLoad (
        &value_map.get_llvm_type (def),
        &var,
        value_map.get_variable_name (def));
    }
  };

  template <>
  struct instruction_translator<ir_opcode::unreachable>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction&,
               llvm_ir_builder_type& builder,
               llvm_value_map&)
    {
      return builder.CreateUnreachable ();
    }
  };

  template <>
  struct instruction_translator<ir_opcode::terminate>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction&,
               llvm_ir_builder_type& builder,
               llvm_value_map&)
    {
      return builder.CreateRetVoid ();
    }
  };

  template <>
  struct instruction_translator<ir_opcode::ret>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      assert (instr.num_args () <= 1 && "Multiple returns not supported yet.");

      if (! instr.has_args ())
        return builder.CreateRetVoid ();
      return builder.CreateRet (&value_map[instr[0]]);
    }
  };

  template <ir_opcode Op>
  struct instruction_translator<Op, std::enable_if_t<is_comparison_op_v<Op>>>
  {
    static constexpr
    auto
    creator_map = generate_ir_type_map<llvm_rel_mapper<Op>::template mapper> ();

    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      const ir_static_def& def = instr.get_def ();
      const ir_type type = value_map.get_type (def);

      return creator_map[type] (
        builder,
        &value_map[instr[0]],
        &value_map[instr[1]],
        value_map.get_variable_name (def));
    }
  };

  template <ir_opcode Op>
  struct instruction_translator<
    Op,
    std::enable_if_t<ir_instruction_traits<Op>::is_arithmetic
                 &&  ir_instruction_traits<Op>::is_binary>>
  {
    static constexpr
    auto
    creator_map = generate_ir_type_map<llvm_barith_map<Op>::template mapper> ();

    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      const ir_static_def& def = instr.get_def ();
      const ir_type type = value_map.get_type (def);

      return creator_map[type] (
        builder,
        &value_map[instr[0]],
        &value_map[instr[1]],
        value_map.get_variable_name (def));
    }
  };

  template <ir_opcode Op>
  struct instruction_translator<
    Op,
    std::enable_if_t<ir_instruction_traits<Op>::is_arithmetic
                 &&  ir_instruction_traits<Op>::is_unary>>
  {
    static constexpr
    auto
    creator_map = generate_ir_type_map<llvm_uarith_map<Op>::template mapper> ();

    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      const ir_static_def& def = instr.get_def ();
      const ir_type type = value_map.get_type (def);

      return creator_map[type] (
        value_map,
        builder,
        &value_map[instr[0]],
        value_map.get_variable_name (def));
    }
  };

  template <>
  struct instruction_translator<ir_opcode::land>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      const ir_static_def& def = instr.get_def ();

      const ir_static_operand& lhs = instr[0];
      const ir_type lhs_type = value_map.get_type (lhs);
      llvm::Constant& lhs_zero = *llvm::ConstantInt::get (&value_map.get_llvm_type (lhs_type), 0);
      const auto& lhs_cond_creator = instruction_translator<ir_opcode::eq>::creator_map[lhs_type];

      const ir_static_operand& rhs = instr[1];
      const ir_type rhs_type = value_map.get_type (rhs);
      llvm::Constant& rhs_zero = *llvm::ConstantInt::get (&value_map.get_llvm_type (rhs_type), 0);
      const auto& rhs_cond_creator = instruction_translator<ir_opcode::ne>::creator_map[rhs_type];

      llvm::BasicBlock& curr_block = *builder.GetInsertBlock ();
      llvm::BasicBlock& rhs_block = value_map.create_block ("rhs");
      llvm::BasicBlock& merge_block = value_map.create_block ("merge");

      // Check if the lhs is equal to 0.
      llvm::Value& lhs_res = *lhs_cond_creator (builder, &value_map[lhs], &lhs_zero, "");

      // If it is, then jump to the merge block, otherwise check the rhs.
      builder.CreateCondBr (&lhs_res, &merge_block, &rhs_block);

      // Switch to the rhs block.
      builder.SetInsertPoint (&rhs_block);
      llvm::Value& rhs_res = *rhs_cond_creator (builder, &value_map[rhs], &rhs_zero, "");
      builder.CreateBr (&merge_block);

      // Merge the results.
      builder.SetInsertPoint (&merge_block);

      llvm::PHINode *phi = builder.CreatePHI (
        &value_map.get_llvm_type<bool> (),
        2,
        value_map.get_variable_name (def));

      // The result is always false if not coming from the rhs block.
      phi->addIncoming (&value_map.get_bool_constant<false> (), &curr_block);
      phi->addIncoming (&rhs_res, &rhs_block);

      return phi;
    }
  };

  template <>
  struct instruction_translator<ir_opcode::lor>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      const ir_static_def& def = instr.get_def ();

      const ir_static_operand& lhs = instr[0];
      const ir_type lhs_type = value_map.get_type (lhs);
      llvm::Constant& lhs_zero = *llvm::ConstantInt::get (&value_map.get_llvm_type (lhs_type), 0);
      const auto& lhs_cond_creator = instruction_translator<ir_opcode::eq>::creator_map[lhs_type];

      const ir_static_operand& rhs = instr[1];
      const ir_type rhs_type = value_map.get_type (rhs);
      llvm::Constant& rhs_zero = *llvm::ConstantInt::get (&value_map.get_llvm_type (rhs_type), 0);
      const auto& rhs_cond_creator = instruction_translator<ir_opcode::ne>::creator_map[rhs_type];

      llvm::BasicBlock& curr_block = *builder.GetInsertBlock ();
      llvm::BasicBlock& rhs_block = value_map.create_block ("rhs");
      llvm::BasicBlock& merge_block = value_map.create_block ("merge");

      // Check if the lhs is equal to 0.
      llvm::Value& lhs_res = *lhs_cond_creator (builder, &value_map[lhs], &lhs_zero, "");

      // If it is, then check the rhs, otherwise jump to the merge block.
      builder.CreateCondBr (&lhs_res, &rhs_block, &merge_block);

      // Merge the results.
      builder.SetInsertPoint (&rhs_block);
      llvm::Value& rhs_res = *rhs_cond_creator (builder, &value_map[rhs], &rhs_zero, "");
      builder.CreateBr (&merge_block);

      // Merge the results.
      builder.SetInsertPoint (&merge_block);

      llvm::PHINode *phi = builder.CreatePHI (
        &value_map.get_llvm_type<bool> (),
        2,
        value_map.get_variable_name (def));

      // The result is always true if not coming from the rhs block.
      phi->addIncoming (&value_map.get_bool_constant<true> (), &curr_block);
      phi->addIncoming (&rhs_res, &rhs_block);

      return phi;
    }
  };

  template <>
  struct instruction_translator<ir_opcode::lnot>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      const ir_static_def& def = instr.get_def ();
      const ir_static_operand& val = instr[0];
      const ir_type val_type = value_map.get_type (val);
      llvm::Constant& zero = *llvm::ConstantInt::get (&value_map.get_llvm_type (val_type), 0);

      const auto& bool_creator = instruction_translator<ir_opcode::eq>::creator_map[val_type];
      llvm::Value& bool_val = *bool_creator (builder, &value_map[val], &zero, "");

      return builder.CreateXor (
        &bool_val,
        false,
        value_map.get_variable_name (def));
    }
  };

  template <ir_opcode Op>
  struct instruction_translator<
    Op,
    std::enable_if_t<ir_instruction_traits<Op>::is_bitwise
                 &&  ir_instruction_traits<Op>::is_binary>>
  {
    static constexpr
    auto
    creator_map = generate_ir_type_map<llvm_binary_bitwise_mapper<Op>::template mapper> ();

    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      const ir_static_def& def = instr.get_def ();
      const ir_type type = value_map.get_type (def);

      return creator_map[type] (
        builder,
        &value_map[instr[0]],
        &value_map[instr[1]],
        value_map.get_variable_name (def));
    }
  };

  template <ir_opcode Op>
  struct instruction_translator<
    Op,
    std::enable_if_t<ir_instruction_traits<Op>::is_bitwise
                 &&  ir_instruction_traits<Op>::is_unary>>
  {
    static constexpr
    auto
    creator_map = generate_ir_type_map<llvm_unary_bitwise_mapper<Op>::template mapper> ();

    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      const ir_static_def& def = instr.get_def ();
      const ir_type type = value_map.get_type (def);

      return creator_map[type] (
        builder,
        &value_map[instr[0]],
        value_map.get_variable_name (def));
    }
  };

  template <>
  struct instruction_translator<ir_opcode::call>
  {
    static
    llvm::Value *
    translate (const ir_static_instruction& instr,
               llvm_ir_builder_type&        builder,
               llvm_value_map&              value_map)
    {
      if (instr.empty ())
        throw std::logic_error ("The `call` instruction must have at least one argument.");

      llvm::SmallVector<llvm::Type *> arg_types;
      std::transform (std::next (instr.begin ()), instr.end (), std::back_inserter (arg_types),
                      [&](const ir_static_operand& op) {
        return &value_map.get_llvm_type (value_map.get_type (op));
      });

      llvm::Type& ret_type = value_map.get_llvm_type (
        instr.has_def () ? value_map.get_type (instr.get_def ()) : ir_type_v<void>);

      llvm::FunctionType *llvm_function_ty = llvm::FunctionType::get (&ret_type, arg_types, false);

      optional_ref<llvm::Function> func {
        *value_map.invoke_with_module ([&](llvm::Module& module) {
          return llvm::Function::Create (
            llvm_function_ty,
            llvm::Function::ExternalLinkage,
            create_twine (as_constant<const char *> (instr[0])),
            module);
        })
      };

      llvm::SmallVector<llvm::Value *> args;
      std::transform (std::next (instr.begin ()), instr.end (), std::back_inserter (args),
                      [&](const ir_static_operand& op) { return &value_map[op]; });

      return builder.CreateCall (
        func.get_pointer (),
        args,
        instr.maybe_get_def () >>= [&](auto def) { return value_map.get_variable_name (def); });
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

  inline constexpr
  auto
  translator_map { ir_metadata::generate_map<instruction_translator_mapper> () };

}

#endif // OCTAVE_IR_COMPILER_LLVM_INSTRUCTION_TRANSLATOR_HPP

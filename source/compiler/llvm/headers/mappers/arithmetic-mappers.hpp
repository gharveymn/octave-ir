/** arithmetic-mappers.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_ARITHMETIC_MAPPERS_HPP
#define OCTAVE_IR_COMPILER_LLVM_ARITHMETIC_MAPPERS_HPP

#include "llvm-common.hpp"

#include "ir-metadata.hpp"

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/IR/IRBuilder.h>

GCH_ENABLE_WARNINGS_MSVC

namespace gch
{
  using llvm_iarith_creator =
    llvm::Value * (llvm::IRBuilderBase::*) (llvm::Value *,
                                            llvm::Value *,
                                            const llvm::Twine&,
                                            bool,
                                            bool);

  static_assert (std::is_same_v<decltype (&llvm::IRBuilderBase::CreateAdd), llvm_iarith_creator>);

  using llvm_farith_creator =
    llvm::Value * (llvm::IRBuilderBase::*) (llvm::Value *,
                                            llvm::Value *,
                                            const llvm::Twine&,
                                            llvm::MDNode *);

  static_assert (std::is_same_v<decltype (&llvm::IRBuilderBase::CreateFAdd), llvm_farith_creator>);

  template <typename            T,
            llvm_iarith_creator SignedCreator,
            llvm_iarith_creator UnsignedCreator,
            llvm_farith_creator FloatCreator>
  struct llvm_arith_mapper_base
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
                const llvm::Twine& name = "")
               -> llvm::Value *
        {

          if constexpr (std::is_integral_v<T>)
          {
            if (std::is_signed_v<T>)
              return std::invoke (SignedCreator, builder, lhs, rhs, name, false, false);
            else
              return std::invoke (UnsignedCreator, builder, lhs, rhs, name, false, false);
          }
          else if constexpr (std::is_floating_point_v<T>)
            return std::invoke (FloatCreator, builder, lhs, rhs, name, nullptr);
          else
            throw std::logic_error { "No llvm function maps to these types." };
        };
    }
  };

  template <ir_opcode Op>
  struct llvm_barith_map
  {
    template <typename T>
    struct mapper;
  };

  template <>
  template <typename T>
  struct llvm_barith_map<ir_opcode::add>::mapper
    : llvm_arith_mapper_base<T, &llvm::IRBuilderBase::CreateAdd,
                                &llvm::IRBuilderBase::CreateAdd,
                                &llvm::IRBuilderBase::CreateFAdd>
  { };

  template <>
  template <typename T>
  struct llvm_barith_map<ir_opcode::sub>::mapper
    : llvm_arith_mapper_base<T, &llvm::IRBuilderBase::CreateSub,
                                &llvm::IRBuilderBase::CreateSub,
                                &llvm::IRBuilderBase::CreateFSub>
  { };

  template <>
  template <typename T>
  struct llvm_barith_map<ir_opcode::mul>::mapper
    : llvm_arith_mapper_base<T, &llvm::IRBuilderBase::CreateMul,
                                &llvm::IRBuilderBase::CreateMul,
                                &llvm::IRBuilderBase::CreateFMul>
  { };

  template <>
  template <typename T>
  struct llvm_barith_map<ir_opcode::div>::mapper
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
                const llvm::Twine& name = "")
               -> llvm::Value *
       {

          if constexpr (std::is_integral_v<T>)
          {
            if (std::is_signed_v<T>)
              return builder.CreateSDiv (lhs, rhs, name, false);
            else
              return builder.CreateUDiv (lhs, rhs, name, false);
          }
          else if constexpr (std::is_floating_point_v<T>)
            return builder.CreateFDiv (lhs, rhs, name, nullptr);
          else
            throw std::logic_error { "No llvm function maps to these types." };
       };
    }
  };

  template <>
  template <typename T>
  struct llvm_barith_map<ir_opcode::mod>::mapper
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](llvm::IRBuilderBase&, llvm::Value *, llvm::Value *, const llvm::Twine&)
               -> llvm::Value *
        {
          throw std::logic_error { "No llvm function maps to these types." };
        };
    }
  };

  template <>
  template <typename T>
  struct llvm_barith_map<ir_opcode::rem>::mapper
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
                const llvm::Twine& name = "")
               -> llvm::Value *
       {

          if constexpr (std::is_integral_v<T>)
          {
            if (std::is_signed_v<T>)
              return builder.CreateSRem (lhs, rhs, name);
            else
              return builder.CreateURem (lhs, rhs, name);
          }
          else if constexpr (std::is_floating_point_v<T>)
            return builder.CreateFRem (lhs, rhs, name, nullptr);
          else
            throw std::logic_error { "No llvm function maps to these types." };
       };
    }
  };

  template <ir_opcode Op>
  struct llvm_uarith_map
  {
    template <typename T>
    struct mapper;
  };

  template <>
  template <typename T>
  struct llvm_uarith_map<ir_opcode::neg>::mapper
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](llvm_module_interface& module, llvm::IRBuilderBase& builder, llvm::Value *val,
                const llvm::Twine& name = "")
               -> llvm::Value *
       {
          if constexpr (std::is_integral_v<T>)
          {
            llvm::Constant *zero = llvm::ConstantInt::get (&module.get_llvm_type<T> (), T ());
            return builder.CreateSub (zero, val, name);
          }
          else if constexpr (std::is_floating_point_v<T>)
            return builder.CreateFNeg (val, name, nullptr);
          else
            throw std::logic_error { "No llvm function maps to these types." };
       };
    }
  };

}

#endif // OCTAVE_IR_COMPILER_LLVM_ARITHMETIC_MAPPERS_HPP

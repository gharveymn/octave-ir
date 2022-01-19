/** relation-mappers.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_RELATION_MAPPERS_HPP
#define OCTAVE_IR_COMPILER_LLVM_RELATION_MAPPERS_HPP

#include "llvm-common.hpp"

#include "ir-metadata.hpp"

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/IR/IRBuilder.h>

GCH_ENABLE_WARNINGS_MSVC

#include <functional>
#include <stdexcept>

namespace gch
{

  using llvm_irel_creator =
    llvm::Value * (llvm::IRBuilderBase::*) (llvm::Value *, llvm::Value *, const llvm::Twine&);

  static_assert (std::is_same_v<decltype (&llvm::IRBuilderBase::CreateICmpEQ), llvm_irel_creator>);

  using llvm_frel_creator =
    llvm::Value * (llvm::IRBuilderBase::*) (llvm::Value *,
                                            llvm::Value *,
                                            const llvm::Twine&,
                                            llvm::MDNode *);

  static_assert (std::is_same_v<decltype (&llvm::IRBuilderBase::CreateFCmpOEQ), llvm_frel_creator>);

  template <typename          T,
            llvm_irel_creator SignedCreator,
            llvm_irel_creator UnsignedCreator,
            llvm_frel_creator FloatCreator>
  struct llvm_rel_mapper_base
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
              return std::invoke (SignedCreator, builder, lhs, rhs, name);
            else
              return std::invoke (UnsignedCreator, builder, lhs, rhs, name);
          }
          else if constexpr (std::is_floating_point_v<T>)
            return std::invoke (FloatCreator, builder, lhs, rhs, name, nullptr);
          else
            throw std::logic_error { "No llvm function maps to these types." };
        };
    }
  };

  template <ir_opcode Op>
  struct llvm_rel_mapper
  {
    template <typename T>
    struct mapper;
  };

  template <>
  template <typename T>
  struct llvm_rel_mapper<ir_opcode::eq>::mapper
    : llvm_rel_mapper_base<T, &llvm::IRBuilderBase::CreateICmpEQ,
                              &llvm::IRBuilderBase::CreateICmpEQ,
                              &llvm::IRBuilderBase::CreateFCmpOEQ>
  { };

  template <>
  template <typename T>
  struct llvm_rel_mapper<ir_opcode::ne>::mapper
    : llvm_rel_mapper_base<T, &llvm::IRBuilderBase::CreateICmpNE,
                              &llvm::IRBuilderBase::CreateICmpNE,
                              &llvm::IRBuilderBase::CreateFCmpONE>
  { };

  template <>
  template <typename T>
  struct llvm_rel_mapper<ir_opcode::lt>::mapper
    : llvm_rel_mapper_base<T, &llvm::IRBuilderBase::CreateICmpSLT,
                              &llvm::IRBuilderBase::CreateICmpULT,
                              &llvm::IRBuilderBase::CreateFCmpOLT>
  { };

  template <>
  template <typename T>
  struct llvm_rel_mapper<ir_opcode::le>::mapper
    : llvm_rel_mapper_base<T, &llvm::IRBuilderBase::CreateICmpSLE,
                              &llvm::IRBuilderBase::CreateICmpULE,
                              &llvm::IRBuilderBase::CreateFCmpOLE>
  { };

  template <>
  template <typename T>
  struct llvm_rel_mapper<ir_opcode::gt>::mapper
    : llvm_rel_mapper_base<T, &llvm::IRBuilderBase::CreateICmpSGT,
                              &llvm::IRBuilderBase::CreateICmpUGT,
                              &llvm::IRBuilderBase::CreateFCmpOGT>
  { };

  template <>
  template <typename T>
  struct llvm_rel_mapper<ir_opcode::ge>::mapper
    : llvm_rel_mapper_base<T, &llvm::IRBuilderBase::CreateICmpSGE,
                              &llvm::IRBuilderBase::CreateICmpUGE,
                              &llvm::IRBuilderBase::CreateFCmpOGE>
  { };

}

#endif // OCTAVE_IR_COMPILER_LLVM_RELATION_MAPPERS_HPP

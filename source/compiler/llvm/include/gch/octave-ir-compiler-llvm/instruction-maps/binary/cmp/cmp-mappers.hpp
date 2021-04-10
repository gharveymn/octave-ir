/** cmp-mappers.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_CMP_MAPPERS_HPP
#define OCTAVE_IR_COMPILER_LLVM_CMP_MAPPERS_HPP

#include "gch/octave-ir-compiler-llvm/llvm-common.hpp"

#include "gch/octave-ir-static-ir/ir-metadata.hpp"

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/IR/IRBuilder.h>

GCH_ENABLE_WARNINGS_MSVC

#include <functional>
#include <stdexcept>

namespace gch
{

  using llvm_icmp_creator = llvm::Value * (llvm::IRBuilderBase::*) (llvm::Value *,
                                                                    llvm::Value *,
                                                                    const llvm::Twine&);

  using llvm_fcmp_creator = llvm::Value * (llvm::IRBuilderBase::*) (llvm::Value *,
                                                                    llvm::Value *,
                                                                    const llvm::Twine&,
                                                                    llvm::MDNode *);

  struct cmp_creator
  {
    llvm::Value *
    operator() (llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
                const llvm::Twine& name = "", llvm::MDNode *fp_math = nullptr) const
    {
      return std::invoke (m_creator_function, builder, lhs, rhs, name, fp_math);
    }

    llvm::Value * (*m_creator_function) (llvm::IRBuilderBase&,
                                         llvm::Value *,
                                         llvm::Value *,
                                         const llvm::Twine&,
                                         llvm::MDNode *);
  };

  template <typename          T,
            llvm_icmp_creator SignedCMP,
            llvm_icmp_creator UnsignedCMP,
            llvm_fcmp_creator FloatCMP,
            typename          Enable = void>
  struct llvm_cmp_mapper_base
  {
    constexpr
    cmp_creator
    operator() (void) const
    {
      return { [](llvm::IRBuilderBase&, llvm::Value *, llvm::Value *,
                  const llvm::Twine&, llvm::MDNode *)
                 -> llvm::Value *
               {
                 throw std::logic_error { "No llvm function maps to these types." };
               } };
    }
  };

  template <typename          T,
            llvm_icmp_creator SignedCMP,
            llvm_icmp_creator UnsignedCMP,
            llvm_fcmp_creator FloatCMP>
  struct llvm_cmp_mapper_base<T, SignedCMP, UnsignedCMP, FloatCMP,
                              std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>>>
  {
    constexpr
    cmp_creator
    operator() (void) const noexcept
    {
      return { [](llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
                  const llvm::Twine& name, llvm::MDNode *)
                  -> llvm::Value *
               {
                 return std::invoke (SignedCMP, builder, lhs, rhs, name);
               } };
    }
  };

  template <typename          T,
            llvm_icmp_creator SignedCMP,
            llvm_icmp_creator UnsignedCMP,
            llvm_fcmp_creator FloatCMP>
  struct llvm_cmp_mapper_base<T, SignedCMP, UnsignedCMP, FloatCMP,
                              std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>>>
  {
    constexpr
    cmp_creator
    operator() (void) const noexcept
    {
      return { [](llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
                  const llvm::Twine& name, llvm::MDNode *)
                  -> llvm::Value *
               {
                 return std::invoke (UnsignedCMP, builder, lhs, rhs, name);
               } };
    }
  };

  template <typename          T,
            llvm_icmp_creator SignedCMP,
            llvm_icmp_creator UnsignedCMP,
            llvm_fcmp_creator FloatCMP>
  struct llvm_cmp_mapper_base<T, SignedCMP, UnsignedCMP, FloatCMP,
                              std::enable_if_t<std::is_floating_point_v<T>>>
  {
    constexpr
    cmp_creator
    operator() (void) const noexcept
    {
      return { [](llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
                  const llvm::Twine& name, llvm::MDNode *fp_tag)
                  -> llvm::Value *
               {
                 return std::invoke (FloatCMP, builder, lhs, rhs, name, fp_tag);
               } };
    }
  };

  template <ir_opcode Op>
  struct llvm_cmp_mapper
  {
    template <typename T>
    struct mapper;
  };

  template <>
  template <typename T>
  struct llvm_cmp_mapper<ir_opcode::eq>::mapper
    : llvm_cmp_mapper_base<T, &llvm::IRBuilderBase::CreateICmpEQ,
                              &llvm::IRBuilderBase::CreateICmpEQ,
                              &llvm::IRBuilderBase::CreateFCmpOEQ>
  { };

  template <>
  template <typename T>
  struct llvm_cmp_mapper<ir_opcode::ne>::mapper
    : llvm_cmp_mapper_base<T, &llvm::IRBuilderBase::CreateICmpNE,
                              &llvm::IRBuilderBase::CreateICmpNE,
                              &llvm::IRBuilderBase::CreateFCmpONE>
  { };

  template <>
  template <typename T>
  struct llvm_cmp_mapper<ir_opcode::lt>::mapper
    : llvm_cmp_mapper_base<T, &llvm::IRBuilderBase::CreateICmpSLT,
                              &llvm::IRBuilderBase::CreateICmpULT,
                              &llvm::IRBuilderBase::CreateFCmpOLT>
  { };

  template <>
  template <typename T>
  struct llvm_cmp_mapper<ir_opcode::le>::mapper
    : llvm_cmp_mapper_base<T, &llvm::IRBuilderBase::CreateICmpSLE,
                              &llvm::IRBuilderBase::CreateICmpULE,
                              &llvm::IRBuilderBase::CreateFCmpOLE>
  { };

  template <>
  template <typename T>
  struct llvm_cmp_mapper<ir_opcode::gt>::mapper
    : llvm_cmp_mapper_base<T, &llvm::IRBuilderBase::CreateICmpSGT,
                              &llvm::IRBuilderBase::CreateICmpUGT,
                              &llvm::IRBuilderBase::CreateFCmpOGT>
  { };

  template <>
  template <typename T>
  struct llvm_cmp_mapper<ir_opcode::ge>::mapper
    : llvm_cmp_mapper_base<T, &llvm::IRBuilderBase::CreateICmpSGE,
                              &llvm::IRBuilderBase::CreateICmpUGE,
                              &llvm::IRBuilderBase::CreateFCmpOGE>
  { };

}

#endif // OCTAVE_IR_COMPILER_LLVM_CMP_MAPPERS_HPP

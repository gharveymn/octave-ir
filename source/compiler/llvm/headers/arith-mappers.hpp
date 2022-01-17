/** arith-mappers.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_ARITH_MAPPERS_HPP
#define OCTAVE_IR_COMPILER_LLVM_ARITH_MAPPERS_HPP

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

  using llvm_farith_creator =
    llvm::Value * (llvm::IRBuilderBase::*) (llvm::Value *,
                                            llvm::Value *,
                                            const llvm::Twine&,
                                            llvm::MDNode *);

  using unbound_arith_creator =
    llvm::Value * (llvm::IRBuilderBase&,
                   llvm::Value *,
                   llvm::Value *,
                   const llvm::Twine&,
                   llvm::MDNode *);

  struct arith_creator
    : unbound_function<unbound_arith_creator>
  {
    using base = unbound_function<unbound_arith_creator>;

    using base::unbound_function;

    constexpr
    arith_creator (unbound_function<unbound_arith_creator> uf)
      : base (uf)
    { }

    llvm::Value *
    operator() (llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
                const llvm::Twine& name = "") const
    {
      return this->call (builder, lhs, rhs, name, nullptr);
    }
  };

  template <typename            T,
            llvm_iarith_creator SignedArith = nullptr,
            llvm_iarith_creator UnsignedArith = nullptr,
            llvm_farith_creator FloatArith = nullptr,
            typename            Enable = void>
  struct llvm_arith_mapper_base
  {
    constexpr
    auto
    operator() (void) const
    {
      return arith_creator {
        [](llvm::IRBuilderBase&, llvm::Value *, llvm::Value *, const llvm::Twine&, llvm::MDNode *)
          -> llvm::Value *
        {
          throw std::logic_error { "No llvm function maps to these types." };
        }
      };
    }
  };

  template <typename            T,
            llvm_iarith_creator SignedArith,
            llvm_iarith_creator UnsignedArith,
            llvm_farith_creator FloatArith>
  struct llvm_arith_mapper_base<T, SignedArith, UnsignedArith, FloatArith,
                                std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>>>
  {
    constexpr
    auto
    operator() (void) const noexcept
    {
      return arith_creator {
        [](llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
           const llvm::Twine& name, llvm::MDNode *)
          -> llvm::Value *
        {
          return std::invoke (SignedArith, builder, lhs, rhs, name, false, false);
        }
      };
    }
  };

  template <typename            T,
            llvm_iarith_creator SignedArith,
            llvm_iarith_creator UnsignedArith,
            llvm_farith_creator FloatArith>
  struct llvm_arith_mapper_base<T, SignedArith, UnsignedArith, FloatArith,
                                std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>>>
  {
    constexpr
    auto
    operator() (void) const noexcept
    {
      return arith_creator {
        [](llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
           const llvm::Twine& name, llvm::MDNode *)
          -> llvm::Value *
        {
          return std::invoke (UnsignedArith, builder, lhs, rhs, name, false, false);
        }
      };
    }
  };

  template <typename            T,
            llvm_iarith_creator SignedArith,
            llvm_iarith_creator UnsignedArith,
            llvm_farith_creator FloatArith>
  struct llvm_arith_mapper_base<T, SignedArith, UnsignedArith, FloatArith,
                                std::enable_if_t<std::is_floating_point_v<T>>>
  {
    constexpr
    arith_creator
    operator() (void) const noexcept
    {
      return static_unbound_function_v<FloatArith>;
    }
  };

  template <ir_opcode Op>
  struct llvm_arith_mapper
  {
    template <typename T>
    struct mapper;
  };

  template <>
  template <typename T>
  struct llvm_arith_mapper<ir_opcode::add>::mapper
    : llvm_arith_mapper_base<T, &llvm::IRBuilderBase::CreateAdd,
                                &llvm::IRBuilderBase::CreateAdd,
                                &llvm::IRBuilderBase::CreateFAdd>
  { };

  template <>
  template <typename T>
  struct llvm_arith_mapper<ir_opcode::sub>::mapper
    : llvm_arith_mapper_base<T, &llvm::IRBuilderBase::CreateSub,
                                &llvm::IRBuilderBase::CreateSub,
                                &llvm::IRBuilderBase::CreateFSub>
  { };

  template <>
  template <typename T>
  struct llvm_arith_mapper<ir_opcode::mul>::mapper
    : llvm_arith_mapper_base<T, &llvm::IRBuilderBase::CreateMul,
                                &llvm::IRBuilderBase::CreateMul,
                                &llvm::IRBuilderBase::CreateFMul>
  { };

  template <>
  template <typename T>
  struct llvm_arith_mapper<ir_opcode::div>::mapper
    : llvm_arith_mapper_base<T>
  { };

  template <>
  template <typename T>
  struct llvm_arith_mapper<ir_opcode::mod>::mapper
    : llvm_arith_mapper_base<T>
  { };

  template <>
  template <typename T>
  struct llvm_arith_mapper<ir_opcode::rem>::mapper
    : llvm_arith_mapper_base<T>
  { };

  // template <>
  // template <typename T>
  // struct llvm_arith_mapper<ir_opcode::div>::mapper<T>
  // {
  //   constexpr
  //   auto
  //   operator() (void) const
  //   {
  //     return arith_creator {
  //       [](llvm::IRBuilderBase&, llvm::Value *, llvm::Value *, const llvm::Twine&, llvm::MDNode *)
  //         -> llvm::Value *
  //       {
  //         throw std::logic_error { "No llvm function maps to these types." };
  //       }
  //     };
  //   }
  // };
  //
  // template <>
  // template <typename T>
  // struct llvm_arith_mapper<ir_opcode::div>::mapper<
  //   T,
  //   std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>>>
  // {
  //   constexpr
  //   auto
  //   operator() (void) const noexcept
  //   {
  //     return arith_creator {
  //       [](llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
  //          const llvm::Twine& name, llvm::MDNode *)
  //         -> llvm::Value *
  //       {
  //         return std::invoke (&llvm::IRBuilderBase::CreateSDiv, builder, lhs, rhs, name, false);
  //       }
  //     };
  //   }
  // };
  //
  // template <>
  // template <typename T>
  // struct llvm_arith_mapper<ir_opcode::div>::mapper<
  //   T,
  //   std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>>>
  // {
  //   constexpr
  //   auto
  //   operator() (void) const noexcept
  //   {
  //     return arith_creator {
  //       [](llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
  //          const llvm::Twine& name, llvm::MDNode *)
  //         -> llvm::Value *
  //       {
  //         return std::invoke (&llvm::IRBuilderBase::CreateUDiv, builder, lhs, rhs, name, false);
  //       }
  //     };
  //   }
  // };
  //
  // template <>
  // template <typename T>
  // struct llvm_arith_mapper<ir_opcode::div>::mapper<
  //   T,
  //   std::enable_if_t<std::is_floating_point_v<T>>>
  // {
  //   constexpr
  //   auto
  //   operator() (void) const noexcept
  //   {
  //     return static_unbound_function_v<&llvm::IRBuilderBase::CreateFDiv>;
  //   }
  // };
  //
  // template <>
  // template <typename T>
  // struct llvm_arith_mapper<ir_opcode::rem>::mapper<T>
  // {
  //   constexpr
  //   auto
  //   operator() (void) const
  //   {
  //     return arith_creator {
  //       [](llvm::IRBuilderBase&, llvm::Value *, llvm::Value *, const llvm::Twine&, llvm::MDNode *)
  //         -> llvm::Value *
  //       {
  //         throw std::logic_error { "No llvm function maps to these types." };
  //       }
  //     };
  //   }
  // };
  //
  // template <>
  // template <typename T>
  // struct llvm_arith_mapper<ir_opcode::rem>::mapper<
  //   T,
  //   std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>>>
  // {
  //   constexpr
  //   auto
  //   operator() (void) const noexcept
  //   {
  //     return arith_creator {
  //       [](llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
  //          const llvm::Twine& name, llvm::MDNode *)
  //         -> llvm::Value *
  //       {
  //         return std::invoke (&llvm::IRBuilderBase::CreateSRem, builder, lhs, rhs, name);
  //       }
  //     };
  //   }
  // };
  //
  // template <>
  // template <typename T>
  // struct llvm_arith_mapper<ir_opcode::rem>::mapper<
  //   T,
  //   std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>>>
  // {
  //   constexpr
  //   auto
  //   operator() (void) const noexcept
  //   {
  //     return arith_creator {
  //       [](llvm::IRBuilderBase& builder, llvm::Value *lhs, llvm::Value *rhs,
  //          const llvm::Twine& name, llvm::MDNode *)
  //         -> llvm::Value *
  //       {
  //         return std::invoke (&llvm::IRBuilderBase::CreateURem, builder, lhs, rhs, name);
  //       }
  //     };
  //   }
  // };
  //
  // template <>
  // template <typename T>
  // struct llvm_arith_mapper<ir_opcode::rem>::mapper<
  //   T,
  //   std::enable_if_t<std::is_floating_point_v<T>>>
  // {
  //   constexpr
  //   auto
  //   operator() (void) const noexcept
  //   {
  //     return static_unbound_function_v<&llvm::IRBuilderBase::CreateFRem>;
  //   }
  // };

}

#endif // OCTAVE_IR_COMPILER_LLVM_ARITH_MAPPERS_HPP

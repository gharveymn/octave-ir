/** llvm-type.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_LLVM_TYPE_HPP
#define OCTAVE_IR_COMPILER_LLVM_LLVM_TYPE_HPP

#include "llvm-common.hpp"
#include "ir-type-traits.hpp"

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

GCH_ENABLE_WARNINGS_MSVC

#include <cfloat>
#include <complex>
#include <iostream>

#if LDBL_MANT_DIG == 113 // IEEE quad
#  ifndef GCH_LONG_DOUBLE_BIT_WIDTH
inline constexpr std::size_t GCH_LONG_DOUBLE_BIT_WIDTH = 128;
#  endif
#  ifndef GCH_LONG_DOUBLE_LLVM_TYPE_ID
inline constexpr llvm::Type::TypeID GCH_LONG_DOUBLE_LLVM_TYPE_ID = llvm::Type::FP128TyID;
#  endif
#elif LDBL_MANT_DIG == 106 // PowerPC double-double
#  ifndef GCH_LONG_DOUBLE_BIT_WIDTH
inline constexpr std::size_t GCH_LONG_DOUBLE_BIT_WIDTH = 128;
#  endif
#  ifndef GCH_LONG_DOUBLE_LLVM_TYPE_ID
inline constexpr llvm::Type::TypeID GCH_LONG_DOUBLE_LLVM_TYPE_ID = llvm::Type::PPC_FP128TyID;
#  endif
#elif LDBL_MANT_DIG == 64 // X87 extended precision
#  ifndef GCH_LONG_DOUBLE_BIT_WIDTH
inline constexpr std::size_t GCH_LONG_DOUBLE_BIT_WIDTH = 80;
#  endif
#  ifndef GCH_LONG_DOUBLE_LLVM_TYPE_ID
inline constexpr llvm::Type::TypeID GCH_LONG_DOUBLE_LLVM_TYPE_ID = llvm::Type::X86_FP80TyID;
#  endif
#elif LDBL_MANT_DIG == 53 // Same as double
#  ifndef GCH_LONG_DOUBLE_BIT_WIDTH
inline constexpr std::size_t GCH_LONG_DOUBLE_BIT_WIDTH = 60;
#  endif
#  ifndef GCH_LONG_DOUBLE_LLVM_TYPE_ID
inline constexpr llvm::Type::TypeID GCH_LONG_DOUBLE_LLVM_TYPE_ID = llvm::Type::DoubleTyID;
#  endif
#else
#  error Unrecognized long double.
#endif

namespace gch
{
  class octave_base_value;
  class ir_block;
  class ir_block_id;
  class ir_external_function_info;

  using any = octave_base_value *;

  template <typename T, typename Enable = void>
  struct llvm_type_function
  {
    static constexpr
    auto
    value = [](llvm::LLVMContext&) -> llvm::Type * {
      if constexpr (is_complete_v<T>)
        std::cerr << "LLVM type not mapped for type `" << typeid (T).name () << "`." << std::endl;
      else
        std::cerr << "LLVM type not mapped for incomplete type." << std::endl;
      return nullptr;
    };
  };

  template <typename T>
  inline constexpr
  auto
  llvm_type_function_v = llvm_type_function<T>::value;

  template <>
  struct llvm_type_function<void>
  {
    static constexpr
    auto
    value = &llvm::Type::getVoidTy;
  };

  template <>
  struct llvm_type_function<long double>
  {
    static constexpr
    auto
    value = [](llvm::LLVMContext& ctx) {
      return llvm::Type::getPrimitiveType (ctx, GCH_LONG_DOUBLE_LLVM_TYPE_ID);
    };
  };

  template <>
  struct llvm_type_function<double>
  {
    static constexpr
    auto
    value = &llvm::Type::getDoubleTy;
  };

  template <>
  struct llvm_type_function<float>
  {
    static constexpr
    auto
    value = &llvm::Type::getFloatTy;
  };

  template <>
  struct llvm_type_function<int64_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt64Ty;
  };

  template <>
  struct llvm_type_function<int32_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt32Ty;
  };

  template <>
  struct llvm_type_function<int16_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt16Ty;
  };

  template <>
  struct llvm_type_function<int8_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt8Ty;
  };

  template <>
  struct llvm_type_function<uint64_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt64Ty;
  };

  template <>
  struct llvm_type_function<uint32_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt32Ty;
  };

  template <>
  struct llvm_type_function<uint16_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt16Ty;
  };

  template <>
  struct llvm_type_function<uint8_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt8Ty;
  };

  template <>
  struct llvm_type_function<char>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt8Ty;
  };

  template <>
  struct llvm_type_function<bool>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt1Ty;
  };

  template <>
  struct llvm_type_function<std::complex<double>>
  {
    static constexpr
    auto
    value = [](llvm::LLVMContext& c) {
      return llvm::ArrayType::get (llvm::Type::getDoubleTy (c), 2);
    };
  };

  template <>
  struct llvm_type_function<std::complex<float>>
  {
    static constexpr
    auto
    value = [](llvm::LLVMContext& c) {
      return llvm::ArrayType::get (llvm::Type::getFloatTy (c), 2);
    };
  };

  template <typename Scalar>
  struct llvm_type_function<
    Scalar,
    std::enable_if_t<std::is_arithmetic_v<Scalar> && ! std::is_const_v<Scalar>>>
  {
    static constexpr
    auto
    value = &llvm::Type::getScalarTy<Scalar>;
  };

  template <typename T>
  struct llvm_type_function<const T>
  {
    // FIXME: Not working.
    static constexpr
    auto
    value = llvm_type_function<T>::value;
  };

  template <>
  struct llvm_type_function<any>
  {
    // FIXME: Not working.
    static constexpr
    auto
    value = [](llvm::LLVMContext&) -> llvm::Type * {
      // std::cerr << "Not implemented yet for `any`." << std::endl;
      return nullptr;
    };
  };

  template <>
  struct llvm_type_function<ir_block_id>
  {
    static constexpr
    auto
    value = [](llvm::LLVMContext&) -> llvm::Type * {
      // This should never be used.
      return nullptr;
    };
  };

  template <>
  struct llvm_type_function<ir_block *>
  {
    static constexpr
    auto
    value = [](llvm::LLVMContext&) -> llvm::Type * {
      // This should never be used.
      return nullptr;
    };
  };

  template <>
  struct llvm_type_function<ir_external_function_info>
  {
    static constexpr
    auto
    value = [](llvm::LLVMContext&) -> llvm::Type * {
      // This should never be used.
      return nullptr;
    };
  };

  template <typename T>
  struct llvm_type_function<T *>
  {
    static constexpr
    auto
    value = [](llvm::LLVMContext& ctx) -> llvm::Type * {
      return llvm::PointerType::getUnqual (llvm_type_function_v<T> (ctx));
    };
  };

  template <typename CharT>
  struct llvm_type_function<std::basic_string<CharT>>
  {
    static constexpr
    auto
    value = llvm_type_function_v<CharT *>;
  };

} // namespace gch

#endif // OCTAVE_IR_COMPILER_LLVM_LLVM_TYPE_HPP

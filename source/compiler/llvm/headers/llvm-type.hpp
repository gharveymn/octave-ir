/** llvm-type.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_LLVM_TYPE_HPP
#define OCTAVE_IR_COMPILER_LLVM_LLVM_TYPE_HPP

#include "llvm-common.hpp"

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

GCH_ENABLE_WARNINGS_MSVC

#include <complex>

namespace gch
{

  using single = float;

  template <typename T, typename Enable = void>
  struct llvm_type_function
  {
    static constexpr
    auto
    value = [](llvm::LLVMContext&) { return nullptr; };
  };

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
    value = &llvm::Type::getFP128Ty;
  };

  template <>
  struct llvm_type_function<double>
  {
    static constexpr
    auto
    value = &llvm::Type::getDoubleTy;
  };

  template <>
  struct llvm_type_function<single>
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
  struct llvm_type_function<std::complex<single>>
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
  struct llvm_type_function<T *>
  {
    static constexpr
    auto
    value = [](llvm::LLVMContext& ctx) {
      llvm::Type *type = llvm_type_function<T>::value (ctx);
      return llvm::PointerType::getUnqual (llvm_type_function<char>::value (ctx));
    };
  };

  template <typename T>
  inline constexpr
  auto
  llvm_type_function_v = llvm_type_function<T>::value;

} // namespace gch

#endif // OCTAVE_IR_COMPILER_LLVM_LLVM_TYPE_HPP

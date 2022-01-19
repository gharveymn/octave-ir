/** llvm-constant.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_LLVM_CONSTANT_HPP
#define OCTAVE_IR_COMPILER_LLVM_LLVM_CONSTANT_HPP

#include "llvm-common.hpp"
#include "llvm-value-map.hpp"

#include "ir-constant.hpp"
#include "ir-type-util.hpp"

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/IR/Constants.h>

GCH_ENABLE_WARNINGS_MSVC

#include <string>

namespace gch
{
  template <typename T, std::enable_if_t<std::is_object_v<T>> * = nullptr>
  llvm::Value&
  get_constant (llvm_module_interface& module, const T& val)
  {
    llvm::Type& type = module.get_llvm_type<T> ();
    if constexpr (std::is_floating_point_v<T>)
      return *llvm::ConstantFP::get (&type, val);
    else if constexpr (std::is_integral_v<T>)
    {
      if constexpr (std::is_signed_v<T>)
        return *llvm::ConstantInt::getSigned (&type, val);
      else
        return *llvm::ConstantInt::get (&type, val);
    }
    else if constexpr (std::is_same_v<T, std::complex<double>>)
    {
      llvm::ArrayRef<uint64_t> a (reinterpret_cast<const uint64_t (&)[2]> (val));
      return *llvm::ConstantDataArray::getFP (&type, a);
    }
    else if constexpr (std::is_same_v<T, std::complex<single>>)
    {
      llvm::ArrayRef<uint32_t> a (reinterpret_cast<const uint32_t (&)[2]> (val));
      return *llvm::ConstantDataArray::getFP (&type, a);
    }
    else if constexpr (std::is_convertible_v<const char *, T>)
    {
      return *module.invoke_with_module ([&](llvm::Module& m) {
        llvm::LLVMContext& ctx = m.getContext ();
        llvm::Constant *c = llvm::ConstantDataArray::getString (ctx, val);
        auto *global_var = new llvm::GlobalVariable (
          m,
          c->getType (),
          true,
          llvm::GlobalValue::PrivateLinkage,
          c);
        global_var->setUnnamedAddr (llvm::GlobalValue::UnnamedAddr::Global);
        global_var->setAlignment(llvm::Align(1));

        llvm::Constant *zero = llvm::ConstantInt::get (llvm::Type::getInt32Ty (ctx), 0);
        llvm::ArrayRef<llvm::Constant *> indices { zero, zero };
        return llvm::ConstantExpr::getInBoundsGetElementPtr (
          global_var->getValueType (),
          global_var,
          indices);
      });
    }
    else
    {
      throw std::logic_error {
        "Cannot create constant of type `" + get_name (ir_type_v<T>) + "`."
      };
    }
  }

  template <typename T>
  struct llvm_constant_mapper
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](llvm_module_interface& module, const ir_constant& c) -> llvm::Value& {
        if constexpr (std::is_object_v<T>)
          return get_constant (module, as<T> (c));
        else
        {
          throw std::logic_error {
            "Cannot create constant of non-object type `" + get_name (c.get_type ()) + "`."
          };
        }
      };
    }
  };

  llvm::Value&
  get_constant (llvm_module_interface& module, const ir_constant& c);

} // namespace gch

#endif // OCTAVE_IR_COMPILER_LLVM_LLVM_CONSTANT_HPP

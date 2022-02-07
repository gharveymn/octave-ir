/** llvm-constant.cpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "llvm-constant.hpp"

#include "llvm-common.hpp"
#include "llvm-value-map.hpp"

#include "ir-common.hpp"
#include "ir-constant.hpp"
#include "ir-type-util.hpp"

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/Alignment.h>

GCH_ENABLE_WARNINGS_MSVC

#include <complex>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace llvm
{

  class LLVMContext;

}

namespace gch
{

  template <typename T, typename Enable = void>
  struct llvm_constant_map
  {
    static
    llvm::Value&
    get_constant (llvm_module_interface&, const ir_constant& c)
    {
      if constexpr (std::is_object_v<T>)
      {
        throw std::logic_error {
          "Cannot create constant of type `" + get_name (ir_type_v<T>) + "`."
        };
      }
      else
      {
        throw std::logic_error {
          "Cannot create constant of non-object type `" + get_name (c.get_type ()) + "`."
        };
      }
    }
  };

  template <typename T>
  struct llvm_constant_map<T, std::enable_if_t<std::is_integral_v<T>>>
  {
    static
    llvm::Value&
    get_constant (llvm_module_interface& module, const ir_constant& c)
    {
      llvm::Type& type = module.get_llvm_type<T> ();
      const auto& val = as<T> (c);
      if constexpr (std::is_signed_v<T>)
        return *llvm::ConstantInt::getSigned (&type, val);
      else
        return *llvm::ConstantInt::get (&type, val);
    }
  };

  template <typename T>
  struct llvm_constant_map<T, std::enable_if_t<std::is_floating_point_v<T>>>
  {
    static
    llvm::Value&
    get_constant (llvm_module_interface& module, const ir_constant& c)
    {
      llvm::Type& type = module.get_llvm_type<T> ();
      const auto& val = as<T> (c);
      if constexpr (std::is_same_v<T, long double>)
      {
        llvm::ArrayRef<std::uint64_t> arr (
          reinterpret_cast<const std::uint64_t *> (&val),
          reinterpret_cast<const std::uint64_t *> (std::next (&val)));

        return *llvm::ConstantFP::get (
          &type,
          { type.getFltSemantics (), llvm::APInt { GCH_LONG_DOUBLE_BIT_WIDTH, arr } });
      }
      else
        return *llvm::ConstantFP::get (&type, llvm::APFloat { val });
    }
  };

  template <typename T>
  struct llvm_constant_map<T, std::enable_if_t<is_complex_v<T>>>
  {
    static
    llvm::Value&
    get_constant (llvm_module_interface& module, const ir_constant& c)
    {
      llvm::Type& type = module.get_llvm_type<T> ();
      const auto& val = as<T> (c);
      if constexpr (std::is_same_v<typename T::value_type, double>)
      {
        llvm::ArrayRef<uint64_t> a (reinterpret_cast<const uint64_t (&)[2]> (val));
        return *llvm::ConstantDataArray::getFP (&type, a);
      }
      else if constexpr (std::is_same_v<typename T::value_type, float>)
      {
        llvm::ArrayRef<uint32_t> a (reinterpret_cast<const uint32_t (&)[2]> (val));
        return *llvm::ConstantDataArray::getFP (&type, a);
      }
      else
      {
        throw std::runtime_error {
          "Could not convert complex type: `" + get_name (ir_type_v<T>) + "`."
        };
      }
    }
  };

  template <typename T>
  struct llvm_constant_map<T, std::enable_if_t<std::is_constructible_v<llvm::StringRef, T>>>
  {
    static
    llvm::Value&
    get_constant (llvm_module_interface& module, const ir_constant& c)
    {
      const auto& val = as<T> (c);
      return *module.invoke_with_module ([&](llvm::Module& mod) {
        llvm::LLVMContext& ctx = mod.getContext ();
        llvm::Constant *llvm_val = llvm::ConstantDataArray::getString (ctx, llvm::StringRef (val));
        auto *global_var = new llvm::GlobalVariable (
          mod,
          llvm_val->getType (),
          true,
          llvm::GlobalValue::PrivateLinkage,
          llvm_val);
        global_var->setUnnamedAddr (llvm::GlobalValue::UnnamedAddr::Global);
        global_var->setAlignment (llvm::Align (1));

        llvm::Constant *zero = llvm::ConstantInt::getNullValue (llvm::Type::getInt32Ty (ctx));
        llvm::ArrayRef<llvm::Constant *> indices { zero, zero };
        return llvm::ConstantExpr::getInBoundsGetElementPtr (
          global_var->getValueType (),
          global_var,
          indices);
      });
    }
  };

  template <>
  struct llvm_constant_map<void *>
  {
    static
    llvm::Value&
    get_constant (llvm_module_interface& module, const ir_constant& c)
    {
      const auto& val = as<void *> (c);

      llvm::Constant& addr = *llvm::ConstantInt::get (
        &module.get_llvm_type<std::uint64_t> (),
        reinterpret_cast<std::size_t> (val));

      return *llvm::ConstantExpr::getIntToPtr (&addr, &module.get_llvm_type<void *> ());
    }
  };

  template <typename T>
  struct llvm_constant_mapper
  {
    constexpr
    auto
    operator() (void) const
    {
      return llvm_constant_map<T>::get_constant;
    }
  };

  llvm::Value&
  get_constant (llvm_module_interface& module, const ir_constant& c)
  {
    constexpr auto map = generate_ir_type_map<llvm_constant_mapper> ();
    return map[c.get_type ()] (module, c);
  }

}

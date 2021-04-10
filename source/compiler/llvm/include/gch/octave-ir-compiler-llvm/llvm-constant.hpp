/** llvm-constant.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_LLVM_CONSTANT_HPP
#define OCTAVE_IR_COMPILER_LLVM_LLVM_CONSTANT_HPP

#include "llvm-common.hpp"
#include "llvm-value-map.hpp"

#include "gch/octave-ir-static-ir/ir-constant.hpp"
#include "gch/octave-ir-static-ir/ir-type-util.hpp"

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/IR/Constants.h>

GCH_ENABLE_WARNINGS_MSVC

#include <string>

namespace gch
{

  template <typename T>
  struct llvm_constant_mapper
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map&, const ir_constant& c) -> llvm::Constant *
             {
               throw std::logic_error { "Cannot create constant of type `"
                                        + get_name (c.get_type ())
                                        + "`." };
             };
    }
  };

  template <>
  struct llvm_constant_mapper<double>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
             {
               return llvm::ConstantFP::get (map.get_llvm_type<double> (), as<double> (c));
             };
    }
  };

  template <>
  struct llvm_constant_mapper<float>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
             {
               return llvm::ConstantFP::get (map.get_llvm_type<float> (), as<float> (c));
             };
    }
  };

  template <>
  struct llvm_constant_mapper<std::int64_t>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
             {
               return llvm::ConstantInt::getSigned (map.get_llvm_type<std::int64_t> (),
                                                    as<std::int64_t> (c));
             };
    }
  };

  template <>
  struct llvm_constant_mapper<std::int32_t>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
             {
               return llvm::ConstantInt::getSigned (map.get_llvm_type<std::int32_t> (),
                                                    as<std::int32_t> (c));
             };
    }
  };

  template <>
  struct llvm_constant_mapper<std::int16_t>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
             {
               return llvm::ConstantInt::getSigned (map.get_llvm_type<std::int16_t> (),
                                                    as<std::int16_t> (c));
             };
    }
  };

  template <>
  struct llvm_constant_mapper<std::int8_t>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
             {
               return llvm::ConstantInt::getSigned (map.get_llvm_type<std::int8_t> (),
                                                    as<std::int8_t> (c));
             };
    }
  };

  template <>
  struct llvm_constant_mapper<std::uint64_t>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
             {
               return llvm::ConstantInt::get (map.get_llvm_type<std::uint64_t> (),
                                              as<std::uint64_t> (c));
             };
    }
  };

  template <>
  struct llvm_constant_mapper<std::uint32_t>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
             {
               return llvm::ConstantInt::get (map.get_llvm_type<std::uint32_t> (),
                                              as<std::uint32_t> (c));
             };
    }
  };

  template <>
  struct llvm_constant_mapper<std::uint16_t>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
             {
               return llvm::ConstantInt::get (map.get_llvm_type<std::uint16_t> (),
                                              as<std::uint16_t> (c));
             };
    }
  };

  template <>
  struct llvm_constant_mapper<std::uint8_t>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
             {
               return llvm::ConstantInt::get (map.get_llvm_type<std::uint8_t> (),
                                              as<std::uint8_t> (c));
             };
    }
  };

  template <>
  struct llvm_constant_mapper<char>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
      {
        return llvm::ConstantInt::getSigned (map.get_llvm_type<char> (), as<char> (c));
      };
    }
  };

  template <>
  struct llvm_constant_mapper<wchar_t>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
      {
        return llvm::ConstantInt::getSigned (map.get_llvm_type<wchar_t> (), as<wchar_t> (c));
      };
    }
  };

  template <>
  struct llvm_constant_mapper<char32_t>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
      {
        return llvm::ConstantInt::getSigned (map.get_llvm_type<char32_t> (), as<char32_t> (c));
      };
    }
  };

  template <>
  struct llvm_constant_mapper<char16_t>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
      {
        return llvm::ConstantInt::getSigned (map.get_llvm_type<char16_t> (), as<char16_t> (c));
      };
    }
  };

#ifdef GCH_CHAR8_T

  template <>
  struct llvm_constant_mapper<char8_t>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
      {
        return llvm::ConstantInt::getSigned (map.get_llvm_type<char8_t> (), as<char8_t> (c));
      };
    }
  };

#endif

  template <>
  struct llvm_constant_mapper<bool>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
      {
        return llvm::ConstantInt::get (map.get_llvm_type<bool> (), as<bool> (c));
      };
    }
  };

  template <>
  struct llvm_constant_mapper<std::complex<double>>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
      {
        const auto& z = as<std::complex<double>> (c);
        llvm::ArrayRef<uint64_t> a (reinterpret_cast<const uint64_t (&)[2]>(z));
        return llvm::ConstantDataArray::getFP (map.get_llvm_type<double> (), a);
      };
    }
  };

  template <>
  struct llvm_constant_mapper<std::complex<single>>
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](const llvm_value_map& map, const ir_constant& c)
      {
        const auto& z = as<std::complex<single>> (c);
        llvm::ArrayRef<uint32_t> a (reinterpret_cast<const uint32_t (&)[2]>(z));
        return llvm::ConstantDataArray::getFP (map.get_llvm_type<single> (), a);
      };
    }
  };

  inline constexpr
  auto
  llvm_constant_map = generate_ir_type_map<llvm_constant_mapper> ();

} // namespace gch

#endif // OCTAVE_IR_COMPILER_LLVM_LLVM_CONSTANT_HPP

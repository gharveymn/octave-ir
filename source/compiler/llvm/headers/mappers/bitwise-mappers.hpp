/** bitwise-mappers.hpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_BITWISE_MAPPERS_HPP
#define OCTAVE_IR_BITWISE_MAPPERS_HPP

#include "llvm-common.hpp"

#include "ir-metadata.hpp"

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/IR/IRBuilder.h>

GCH_ENABLE_WARNINGS_MSVC

namespace gch
{

  template <ir_opcode Op>
  struct llvm_binary_bitwise_mapper
  {
    template <typename T>
    struct mapper
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
              if constexpr (Op == ir_opcode::band)
                return builder.CreateAnd (lhs, rhs, name);
              else if constexpr (Op == ir_opcode::bor)
                return builder.CreateOr (lhs, rhs, name);
              else if constexpr (Op == ir_opcode::bxor)
                return builder.CreateXor (lhs, rhs, name);
              else if constexpr (Op == ir_opcode::bshiftl)
                return builder.CreateShl (lhs, rhs, name, false, false);
              else if constexpr (Op == ir_opcode::bashiftr)
                return builder.CreateAShr (lhs, rhs, name, false);
              else if constexpr (Op == ir_opcode::blshiftr)
                return builder.CreateLShr (lhs, rhs, name, false);
            }
            else
              throw std::logic_error { "No llvm function maps to these types." };
          };
      }
    };
  };

  template <ir_opcode Op>
  struct llvm_unary_bitwise_mapper
  {
    template <typename T>
    struct mapper
    {
      constexpr
      auto
      operator() (void) const
      {
        return [](llvm::IRBuilderBase& builder, llvm::Value *val, const llvm::Twine& name = "")
                 -> llvm::Value *
          {
            if constexpr (std::is_integral_v<T>)
            {
              if constexpr (Op == ir_opcode::bnot)
                return builder.CreateNot (val, name);
            }
            else
              throw std::logic_error { "No llvm function maps to these types." };
          };
      }
    };
  };

}

#endif // OCTAVE_IR_BITWISE_MAPPERS_HPP

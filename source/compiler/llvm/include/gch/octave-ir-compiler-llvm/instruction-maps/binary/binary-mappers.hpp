/** binary-mappers.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_BINARY_MAPPERS_HPP
#define OCTAVE_IR_COMPILER_LLVM_BINARY_MAPPERS_HPP

#include "cmp-mappers.hpp"

namespace gch
{

  struct llvm_binary_mapper
  {
    constexpr
    auto
    operator() (void) const
    {
      return [](llvm::IRBuilder<>&, llvm::Value *, llvm::Value *, const llvm::Twine&)
                -> llvm::Value *
             {
               throw std::logic_error { "No llvm function maps to these types." };
             };
    }
  };

}

#endif // OCTAVE_IR_COMPILER_LLVM_BINARY_MAPPERS_HPP

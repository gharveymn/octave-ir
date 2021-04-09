/** llvm-common.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_LLVM_COMMON_HPP
#define OCTAVE_IR_COMPILER_LLVM_LLVM_COMMON_HPP

#ifdef _MSC_VER
#  pragma warning (push, 0)
#endif

#include <llvm/ADT/Twine.h>

#ifdef _MSC_VER
#  pragma warning (pop)
#endif

#include <string_view>

namespace gch
{

  inline
  llvm::Twine
  create_twine (std::string_view view)
  {
    return { view.data () };
  }

}

#endif // OCTAVE_IR_COMPILER_LLVM_LLVM_COMMON_HPP

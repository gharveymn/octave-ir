/** llvm-common.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_LLVM_COMMON_HPP
#define OCTAVE_IR_COMPILER_LLVM_LLVM_COMMON_HPP

#ifndef GCH_DISABLE_WARNINGS_MSVC
#  ifdef _MSC_VER
#    define GCH_DISABLE_WARNINGS_MSVC _Pragma ("warning (push, 0)")
#  else
#    define GCH_DISABLE_WARNINGS_MSVC
#  endif
#endif

#ifndef GCH_ENABLE_WARNINGS_MSVC
#  ifdef _MSC_VER
#    define GCH_ENABLE_WARNINGS_MSVC _Pragma ("warning (pop)")
#  else
#    define GCH_ENABLE_WARNINGS_MSVC
#  endif
#endif

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/ADT/Twine.h>

GCH_ENABLE_WARNINGS_MSVC

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

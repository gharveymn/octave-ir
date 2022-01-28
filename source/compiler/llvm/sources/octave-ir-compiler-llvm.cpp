/** octave-ir-compiler-llvm.cpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-ir-compiler-llvm.hpp"

#include "llvm-interface.hpp"

#include <iostream>

namespace gch
{

  octave_jit_compiler_llvm::
  octave_jit_compiler_llvm (void)
    : m_interface (llvm::cantFail (llvm_interface::create ()))
  { }

  octave_jit_compiler_llvm::
  ~octave_jit_compiler_llvm (void) = default;

  void *
  octave_jit_compiler_llvm::
  compile (const ir_static_function& func)
  {
    llvm::ExitOnError exit_on_error { };
    exit_on_error (m_interface->add_ast (func));
    auto sym = exit_on_error (m_interface->find_symbol (func.get_name ()));
    return reinterpret_cast<void *> (sym.getAddress ());
  }

  void
  octave_jit_compiler_llvm::
  enable_printing (bool printing)
  {
    m_interface->enable_printing (printing);
  }

}

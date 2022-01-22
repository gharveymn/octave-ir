/** octave-ir-compiler-llvm.hpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_OCTAVE_IR_COMPILER_LLVM_HPP
#define OCTAVE_IR_OCTAVE_IR_COMPILER_LLVM_HPP

#include "gch/octave-ir-compiler-interface.hpp"

#include <memory>

namespace gch
{

  class llvm_interface;

  class octave_jit_compiler_llvm : public octave_jit_compiler_impl
  {
  public:
    octave_jit_compiler_llvm            (void);
    octave_jit_compiler_llvm            (const octave_jit_compiler_llvm&)     = delete;
    octave_jit_compiler_llvm            (octave_jit_compiler_llvm&&) noexcept = default;
    octave_jit_compiler_llvm& operator= (const octave_jit_compiler_llvm&)     = delete;
    octave_jit_compiler_llvm& operator= (octave_jit_compiler_llvm&&) noexcept = default;
    ~octave_jit_compiler_llvm           (void) override;

    std::size_t
    compile (const ir_static_function& func) override;

    void
    enable_printing (bool printing = true) override;

  private:
    std::unique_ptr<llvm_interface> m_interface;
  };

}

#endif // OCTAVE_IR_OCTAVE_IR_COMPILER_LLVM_HPP

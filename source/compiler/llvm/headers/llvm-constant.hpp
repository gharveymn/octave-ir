/** llvm-constant.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_LLVM_CONSTANT_HPP
#define OCTAVE_IR_COMPILER_LLVM_LLVM_CONSTANT_HPP

namespace llvm
{

  class Value;

}

namespace gch
{

  class ir_constant;
  class llvm_module_interface;

  llvm::Value&
  get_constant (llvm_module_interface& module, const ir_constant& c);

} // namespace gch

#endif // OCTAVE_IR_COMPILER_LLVM_LLVM_CONSTANT_HPP

/** test-call.cpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "test-templates.hpp"

using namespace gch;

int
main (void)
{
  ir_function my_func;

  ir_block& block = get_entry_block (my_func);

  block.append_instruction<ir_opcode::call> (
    { "printf", ir_external_function_info::variadic<true> },
    "%s\n",
    "myerror");

  ir_static_function my_static_func = generate_static_function (my_func);

  std::cout << my_static_func << std::endl << std::endl;

  auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();
  jit.enable_printing ();

  try
  {
    auto *proto = reinterpret_cast<void (*)(void)> (jit.compile (my_static_func));
    proto ();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what () << std::endl;
    return 1;
  }

  return 0;
}

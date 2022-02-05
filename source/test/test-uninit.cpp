/** test-uninit.cpp
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

  ir_variable& var_x = my_func.get_variable ("x");
  var_x.set_type<int> ();

  block.append_instruction_with_def<ir_opcode::assign> (var_x, var_x);

  ir_static_function my_static_func = generate_static_function (my_func);

  std::cout << my_static_func << std::endl << std::endl;

  auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();
  jit.enable_printing ();

  try
  {
    invoke_compiled_function (jit.compile (my_static_func));
  }
  catch (const std::exception& e)
  {
    // We expect to catch an exception.
    std::cerr << e.what () << std::endl;
    return 0;
  }

  return 1;
}

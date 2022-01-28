/** test-if.cpp
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
  static constexpr bool expected = false;

  ir_function my_func ("y", "myfunc");
  ir_variable& var_x = my_func.get_variable ("x");
  var_x.set_type<int> ();

  ir_variable& var_y = my_func.get_variable ("y");
  var_y.set_type<bool> ();

  ir_variable& var_tmp = my_func.get_variable ("tmp");
  var_tmp.set_type<int> ();

  auto& seq = dynamic_cast<ir_component_sequence&> (my_func.get_body ());
  ir_block& entry = get_entry_block (seq);
  entry.append_instruction<ir_opcode::assign> (var_x, ir_constant (0));

  auto& fork = seq.emplace_back<ir_component_fork> (var_tmp);
  ir_block& condition_block = fork.get_condition ();
  auto& true_block = fork.add_case<ir_block> ();
  auto& false_block = fork.add_case<ir_block> ();

  condition_block.append_instruction<ir_opcode::eq> (var_tmp, var_x, ir_constant (1));

  true_block.append_instruction<ir_opcode::assign> (var_y, ir_constant (true));
  false_block.append_instruction<ir_opcode::assign> (var_y, ir_constant (false));

  auto& after = seq.emplace_back<ir_block> ();
  after.append_instruction<ir_opcode::ret> (var_y);

  ir_static_function my_static_func = generate_static_function (my_func);

  std::cout << my_static_func << std::endl << std::endl;

  auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();
  jit.enable_printing ();

  try
  {
    auto *proto = reinterpret_cast<bool (*)(void)> (jit.compile (my_static_func));
    bool res = proto ();

    std::cout << "Result:    " << res << "\n";
    std::cout << "Expected:  " << expected << std::endl;

    if (expected != res)
      return 1;
  }
  catch (std::exception& e)
  {
    std::cerr << e.what () << std::endl;
    return 1;
  }

  return 0;
}

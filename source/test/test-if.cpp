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
  static constexpr bool expected = true;

  ir_function my_func ({ "out", ir_type_v<bool> }, { { "in", ir_type_v<int> } }, "myfunc");
  ir_variable& var_in = my_func.get_variable ("in");
  ir_variable& var_out = my_func.get_variable ("out");

  my_func.set_anonymous_variable_type<bool> ();

  auto& seq = dynamic_cast<ir_component_sequence&> (my_func.get_body ());
  ir_block& entry = get_entry_block (seq);

  auto& fork = seq.emplace_back<ir_component_fork> (my_func.get_variable ());
  ir_block& condition_block = fork.get_condition ();
  auto& true_block = fork.add_case<ir_block> ();
  auto& false_block = fork.add_case<ir_block> ();

  entry          .set_name ("entry");
  condition_block.set_name ("condition");
  true_block     .set_name ("true");
  false_block    .set_name ("false");

  condition_block.append_with_def<ir_opcode::eq> (
    condition_block.get_condition_variable (),
    var_in,
    ir_constant (1));

  true_block.append_with_def<ir_opcode::assign> (var_out, expected);
  // Do nothing in the false block.

  ir_static_function my_static_func = generate_static_function (my_func);

  std::cout << my_static_func << std::endl << std::endl;

  auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();
  jit.enable_printing ();

  void *func = jit.compile (my_static_func);
  try
  {
    auto *proto = reinterpret_cast<bool (*)(int)> (func);
    bool res = proto (1);

    std::cout << "Result:    " << res << "\n";
    std::cout << "Expected:  " << expected << std::endl;

    if (expected != res)
      return 1;
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what () << std::endl;
    return 1;
  }

  // We expect to fail the second time.
  try
  {
    invoke_compiled_function (func, 0);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what () << std::endl;
    return 0;
  }

  return 1;

}

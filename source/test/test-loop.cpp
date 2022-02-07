/** test-loop.cpp
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
  static constexpr int expected = 11;

  ir_function my_func ({ "x", ir_type_v<int> }, "myloopfunc");

  ir_variable& var_x = my_func.get_variable ("x");
  ir_variable& var_i = my_func.create_variable<int> ("i");
  my_func.set_anonymous_variable_type<bool> ();

  auto& seq = dynamic_cast<ir_component_sequence&> (my_func.get_body ());

  ir_block& entry_block     = get_entry_block (seq);
  auto&     loop            = seq.emplace_back<ir_component_loop> (my_func.get_variable ());
  auto&     start_block     = static_cast<ir_block&> (loop.get_start ());
  ir_block& condition_block = loop.get_condition ();
  auto&     body_seq        = static_cast<ir_component_sequence&> (loop.get_body ());
  auto&     body_block      = static_cast<ir_block&> (body_seq.front ());
  auto&     update_block    = static_cast<ir_block&> (loop.get_update ());
  auto&     after_block     = seq.emplace_back<ir_block> ();

  entry_block    .set_name ("entry");
  start_block    .set_name ("start");
  condition_block.set_name ("condition");
  body_block     .set_name ("body");
  update_block   .set_name ("update");
  after_block    .set_name ("after");

  entry_block    .append_with_def<ir_opcode::assign> (var_x, 1);
  start_block    .append_with_def<ir_opcode::assign> (var_i, 0);
  update_block   .append_with_def<ir_opcode::add>    (var_i, var_i, 1);
  body_block     .append_with_def<ir_opcode::add>    (var_x, var_x, 2);
  condition_block.append_with_def<ir_opcode::lt>     (condition_block.get_condition_variable (), var_i, 5);

  ir_static_function my_static_func = generate_static_function (my_func);

  std::cout << my_static_func << std::endl << std::endl;

  auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();
  jit.enable_printing ();

  try
  {
    int res = invoke_compiled_function<int> (jit.compile (my_static_func));

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

  return 0;
}

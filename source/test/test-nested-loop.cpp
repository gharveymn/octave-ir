/** test-nested-loop.cpp
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
  static constexpr int expected = 16;

  ir_function my_func ("x", "myloopfunc");
  ir_variable& var_x = my_func.get_variable ("x");
  var_x.set_type<int> ();

  ir_variable& var_i = my_func.create_variable<int> ("i");

  ir_variable& var_j = my_func.create_variable<int> ("j");

  ir_variable& var_tmp = my_func.create_variable<bool> ();

  auto& seq = dynamic_cast<ir_component_sequence&> (my_func.get_body ());

  ir_block& entry_block     = get_entry_block (seq);
  auto&     loop            = seq.emplace_back<ir_component_loop> (var_tmp);
  auto&     start_block     = static_cast<ir_block&> (loop.get_start ());
  ir_block& condition_block = loop.get_condition ();
  auto&     body_seq        = static_cast<ir_component_sequence&> (loop.get_body ());
  auto&     body_block      = static_cast<ir_block&> (body_seq.front ());
  auto&     update_block    = static_cast<ir_block&> (loop.get_update ());

  auto&     loop2            = body_seq.emplace_back<ir_component_loop> (var_tmp);
  auto&     start_block2     = static_cast<ir_block&> (loop2.get_start ());
  ir_block& condition_block2 = loop2.get_condition ();
  auto&     body_seq2        = static_cast<ir_component_sequence&> (loop2.get_body ());
  auto&     body_block2      = static_cast<ir_block&> (body_seq2.front ());
  auto&     update_block2    = static_cast<ir_block&> (loop2.get_update ());

  auto&     after_block     = seq.emplace_back<ir_block> ();

  entry_block     .set_name ("entry");

  start_block     .set_name ("start");
  condition_block .set_name ("condition");
  body_block      .set_name ("body");
  update_block    .set_name ("update");

  start_block2    .set_name ("start2");
  condition_block2.set_name ("condition2");
  body_block2     .set_name ("body2");
  update_block2   .set_name ("update2");

  after_block     .set_name ("after");

  auto print_dt = [](const ir_block& block, const ir_variable& var) {
    block.maybe_get_def_timeline (var) >>= [](auto& dt) { std::cout << dt << '\n' << std::endl; };
  };

  auto print_dts = [&] {
    print_dt (entry_block,     var_x);
    print_dt (start_block,     var_x);
    print_dt (condition_block, var_x);
    print_dt (body_block,      var_x);
    print_dt (update_block,    var_x);
    print_dt (start_block2,    var_x);
    print_dt (condition_block2,var_x);
    print_dt (body_block2,     var_x);
    print_dt (update_block2,   var_x);
    print_dt (after_block,     var_x);
  };

  entry_block.append_instruction<ir_opcode::assign> (var_x, 1);

  start_block.append_instruction<ir_opcode::assign> (var_i, 0);
  update_block.append_instruction<ir_opcode::add> (var_i, var_i, 1);

  start_block2.append_instruction<ir_opcode::assign> (var_j, 0);
  update_block2.append_instruction<ir_opcode::add> (var_j, var_j, 1);

  body_block2.append_instruction<ir_opcode::add> (var_x, var_x, 2);

  print_dts ();

  condition_block2.append_instruction<ir_opcode::lt> (var_tmp, var_j, 3);

  condition_block.append_instruction<ir_opcode::lt> (var_tmp, var_i, 5);

  after_block.append_instruction<ir_opcode::ret> (var_x);

  ir_static_function my_static_func = generate_static_function (my_func);

  std::cout << my_static_func << std::endl << std::endl;

  auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();
  jit.enable_printing ();

  try
  {
    auto *proto = reinterpret_cast<int (*)(void)> (jit.compile (my_static_func));
    int res = proto ();

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

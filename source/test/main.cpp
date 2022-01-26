/** main.cpp
 * Short description here.
 *
 * Copyright © 2019 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-ir-compiler-llvm.hpp"
#include "ir-static-function.hpp"
#include "ir-all-components.hpp"
#include "component/inspectors/ir-static-function-generator.hpp"

#include "ir-functional.hpp"
#include <functional>
#include <utility>
#include <iostream>

using namespace gch;

template <ir_opcode Op, int LHS, int RHS, bool Expected>
void
test_binary (void)
{
  ir_function my_func;

  ir_block& block = get_entry_block (my_func);

  ir_variable& var_x = my_func.get_variable ("x");
  var_x.set_type<int> ();

  ir_variable& var_y = my_func.get_variable ("y");
  var_y.set_type<int> ();

  ir_variable& var_z = my_func.get_variable ("z");
  var_z.set_type<bool> ();

  block.append_instruction<ir_opcode::assign> (var_x, ir_constant (LHS));
  block.append_instruction<ir_opcode::assign> (var_y, ir_constant (RHS));
  block.append_instruction<Op> (var_z, var_x, var_y);
  block.append_instruction<ir_opcode::ret> (var_z);

  ir_static_function my_static_func = generate_static_function (my_func);

  auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();

  try
  {
    auto *proto = reinterpret_cast<bool (*)(void)> (jit.compile (my_static_func));
    bool res = proto ();
    std::cout << "Result: " << proto () << " (Expected: " << Expected << ")" << std::endl;
    assert (res == Expected);
  }
  catch (std::exception& e)
  {
    std::cerr << e.what () << std::endl;
    exit (1);
  }
}

template <ir_opcode Op, int Val, bool Expected>
void
test_unary (void)
{
  ir_function my_func;

  ir_block& block = get_entry_block (my_func);

  ir_variable& var_x = my_func.get_variable ("x");
  var_x.set_type<int> ();

  ir_variable& var_y = my_func.get_variable ("y");
  var_y.set_type<bool> ();

  block.append_instruction<ir_opcode::assign> (var_x, ir_constant (Val));
  block.append_instruction<Op> (var_y, var_x);
  block.append_instruction<ir_opcode::ret> (var_y);

  ir_static_function my_static_func = generate_static_function (my_func);

  auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();

  try
  {
    auto *proto = reinterpret_cast<bool (*)(void)> (jit.compile (my_static_func));
    bool res = proto ();
    std::cout << "Result: " << proto () << " (Expected: " << Expected << ")" << std::endl;
    assert (res == Expected);
  }
  catch (std::exception& e)
  {
    std::cerr << e.what () << std::endl;
    exit (1);
  }
}

static
void
invalid_func (void)
{
  ir_function my_func;

  ir_block& block = get_entry_block (my_func);

  ir_variable& var_x = my_func.get_variable ("x");
  var_x.set_type<int> ();

  block.append_instruction<ir_opcode::assign> (var_x, var_x);

  ir_static_function my_static_func = generate_static_function (my_func);

  std::cout << my_static_func << std::endl << std::endl;

  auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();
  jit.enable_printing ();

  try
  {
    auto *proto = reinterpret_cast<bool (*)(void)> (jit.compile (my_static_func));
    bool res = proto ();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what () << std::endl;
  }
}

static
void
call_func (void)
{
  ir_function my_func;

  ir_block& block = get_entry_block (my_func);

  block.append_instruction<ir_opcode::call> (
    ir_constant ("print_error"),
    ir_constant ("myerror"));

  ir_static_function my_static_func = generate_static_function (my_func);

  std::cout << my_static_func << std::endl << std::endl;

  auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();
  jit.enable_printing ();

  try
  {
    auto *proto = reinterpret_cast<bool (*)(void)> (jit.compile (my_static_func));
    bool res = proto ();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what () << std::endl;
  }
}

static
void
test_if (void)
{
  ir_function my_func ("y", { }, "myfunc");
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
  // false_block.append_instruction<ir_opcode::assign> (var_y, ir_constant (false));

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
    std::cout << "Result: " << proto () << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << e.what () << std::endl;
    exit (1);
  }
}

static
void
test_loop (void)
{
  ir_function my_func ("x", { }, "myloopfunc");

  ir_variable& var_x = my_func.get_variable ("x");
  var_x.set_type<int> ();

  ir_variable& var_i = my_func.create_variable<int> ("i");

  ir_variable& var_tmp = my_func.create_variable<bool> ();

  auto& seq = dynamic_cast<ir_component_sequence&> (my_func.get_body ());

  ir_block& entry_block     = get_entry_block (seq);
  auto&     loop            = seq.emplace_back<ir_component_loop> (var_tmp);
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

  entry_block.append_instruction<ir_opcode::assign> (var_x, 1);

  start_block.append_instruction<ir_opcode::assign> (var_i, 0);

  update_block.append_instruction<ir_opcode::add> (var_i, var_i, 1);

  body_block.append_instruction<ir_opcode::add> (var_x, var_x, 2);

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
    std::cout << "Result: " << proto () << " (Expected: 11)" << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << e.what () << std::endl;
    exit (1);
  }
}

static
void
test_double_loop (void)
{
  ir_function my_func ("x", { }, "myloopfunc");
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

  entry_block.append_instruction<ir_opcode::assign> (var_x, 1);

  start_block.append_instruction<ir_opcode::assign> (var_i, 0);
  update_block.append_instruction<ir_opcode::add> (var_i, var_i, 1);

  start_block2.append_instruction<ir_opcode::assign> (var_j, 0);
  update_block2.append_instruction<ir_opcode::add> (var_j, var_j, 1);

  body_block2.append_instruction<ir_opcode::add> (var_x, var_x, 2);

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
    std::cout << "Result: " << proto () << " (Expected: 16)" << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << e.what () << std::endl;
    exit (1);
  }
}

int
main (void)
{
  test_loop ();

  invalid_func ();

  std::cout << "Test land 1" << std::endl;

  test_binary<ir_opcode::land, 0, 0, false> ();
  test_binary<ir_opcode::land, 1, 0, false> ();
  test_binary<ir_opcode::land, 0, 1, false> ();
  test_binary<ir_opcode::land, 1, 1, true> ();
  std::cout << std::endl;

  std::cout << "Test lor 1" << std::endl;

  test_binary<ir_opcode::lor, 0, 0, false> ();
  test_binary<ir_opcode::lor, 1, 0, true> ();
  test_binary<ir_opcode::lor, 0, 1, true> ();
  test_binary<ir_opcode::lor, 1, 1, true> ();
  std::cout << std::endl;

  std::cout << "Test lnot 1" << std::endl;

  test_unary<ir_opcode::lnot, 0, true> ();
  test_unary<ir_opcode::lnot, 1, false> ();
  std::cout << std::endl;

  std::cout << "Test land 2" << std::endl;

  test_binary<ir_opcode::land, 0b00, 0b00, false> ();
  test_binary<ir_opcode::land, 0b10, 0b00, false> ();
  test_binary<ir_opcode::land, 0b00, 0b10, false> ();
  test_binary<ir_opcode::land, 0b10, 0b10, true> ();
  std::cout << std::endl;

  std::cout << "Test lor 2" << std::endl;

  test_binary<ir_opcode::lor, 0b00, 0b00, false> ();
  test_binary<ir_opcode::lor, 0b10, 0b00, true> ();
  test_binary<ir_opcode::lor, 0b00, 0b10, true> ();
  test_binary<ir_opcode::lor, 0b10, 0b10, true> ();
  std::cout << std::endl;

  std::cout << "Test lnot 2" << std::endl;

  test_unary<ir_opcode::lnot, 0b00, true> ();
  test_unary<ir_opcode::lnot, 0b10, false> ();
  std::cout << std::endl;

  call_func ();

  return 0;
}

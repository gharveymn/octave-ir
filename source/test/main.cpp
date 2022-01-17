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

struct test_struct
{
  void
  f (void) &&
  { }

  void
  g (void) const volatile && noexcept
  { }

  int x;
};

static
short
func (int, long)
{
  return 0;
}

static
short
func1 (int, long = 0);

template <typename F, typename ...Args>
struct compat;

template <typename M, typename T, typename ...Args>
struct compat<M T::*, Args...>
  : std::is_invocable<M T::*, Args...>
{ };

template <typename Function>
struct function_result_impl1
  : pack_push_front<function_args_t<Function>, Function>
{ };

template <typename M, typename T>
struct function_result_impl1<M T::*>
  : pack_concatenate<type_pack<M T::*, match_function_cvref_t<M, T>>,
                          function_args_t<M>>
{ };

static
int
testfun (void)
{
  test_struct ts;
  auto bf = bind_front (&test_struct::f, ts);
  auto bff = bind_front (&test_struct::f, ts);
  // static_assert (std::is_same_v<void, decltype (bf)>);
  // static_assert (std::is_same_v<void, decltype (bff)>);
  std::invoke (std::move (bff));

  auto bf1 = bind_front (&test_struct::x, ts);
  int x = invoke (bf1);

  static_assert (std::is_same_v<short, function_result_t<decltype (func)>>);
  static_assert (std::is_same_v<short, function_result_t<decltype (&func)>>);
  static_assert (std::is_same_v<decltype (&test_struct::f), std::remove_pointer_t<decltype (&test_struct::f)>>);
  static_assert (std::is_same_v<decltype (&test_struct::f), std::decay_t<decltype (&test_struct::f)>>);
  static_assert (std::is_same_v<short, std::invoke_result_t<decltype (&func), int, long>>);

  static_assert (is_rvref_qualified_v<decltype (&test_struct::f)>);

  auto p = static_unbound_function_v<&test_struct::g>;

  static_assert (std::is_same_v<void (const volatile test_struct&&) noexcept,
                                unified_equivalent_function_t<decltype (&test_struct::g)>>);

  static_assert (std::is_same_v<void (const volatile test_struct&&) noexcept,
                                decltype (p)::function_type>);

  unbound_function<void (void)> uff ([] { });
  unbound_function<void (test_struct&&)> uf (static_function_v<&test_struct::f>);
  uf (test_struct { });
  return 0;
}



int
main (void)
{
  ir_function my_func;

  ir_block& block = get_entry_block (my_func);

  ir_variable& var_x = my_func.get_variable ("x");
  var_x.set_type (ir_type_v<int>);

  ir_variable& var_y = my_func.get_variable ("y");
  var_y.set_type (ir_type_v<int>);

  ir_variable& var_z = my_func.get_variable ("z");
  var_z.set_type (ir_type_v<int>);

  block.append_instruction<ir_opcode::assign> (var_x, ir_constant (1));
  block.append_instruction<ir_opcode::assign> (var_y, ir_constant (2));
  block.append_instruction<ir_opcode::add> (var_z, var_x, var_y);
  block.append_instruction<ir_opcode::ret> (var_z);

  ir_static_function my_static_func = generate_static_function (my_func);

  auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();

  try
  {
    auto *proto = reinterpret_cast<int (*)(void)> (jit.compile (my_static_func));
    std::cout << "Result: " << proto () << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << e.what () << std::endl;
    return 1;
  }

  return 0;
}

/** test-templates.hpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_TEST_TEMPLATES_HPP
#define OCTAVE_IR_TEST_TEMPLATES_HPP

#if defined (__cpp_nontype_template_args) && __cpp_nontype_template_args >= 201911L
#  ifndef GCH_FLOAT_NONTYPE_TEMPLATE_ARGS
#    define GCH_FLOAT_NONTYPE_TEMPLATE_ARGS
#  endif
#endif

#include "gch/octave-ir-compiler-llvm.hpp"
#include "ir-static-function.hpp"
#include "ir-all-components.hpp"
#include "ir-type-util.hpp"

#include <iostream>

namespace gch
{

  template <typename LHSType, typename RHSType>
  bool
  binary_compare (LHSType lhs, RHSType rhs)
  {
    if constexpr (std::is_floating_point_v<LHSType> || std::is_floating_point_v<RHSType>)
    {
      LHSType abs_lhs = std::abs (lhs);
      RHSType abs_rhs = std::abs (rhs);

      return std::abs (lhs - rhs) <= (
        (abs_lhs < abs_rhs) ? abs_rhs * std::numeric_limits<RHSType>::epsilon ()
                            : abs_lhs * std::numeric_limits<LHSType>::epsilon ());
    }
    else
      return lhs == rhs;
  }

  template <ir_opcode Op, typename ResultType, typename LHSType, typename RHSType>
  void
  test_binary (ResultType expected, LHSType lhs, RHSType rhs, bool printing_enabled = false)
  {
    using result_type = ResultType;
    using lhs_type    = LHSType;
    using rhs_type    = RHSType;

    using prototype = result_type (*) (lhs_type, rhs_type);

    ir_function my_func (
      { "z", ir_type_v<result_type> },
      { { "x", ir_type_v<lhs_type> } ,
        { "y", ir_type_v<rhs_type> } });

    ir_variable& var_x = my_func.get_variable ("x");
    ir_variable& var_y = my_func.get_variable ("y");
    ir_variable& var_z = my_func.get_variable ("z");

    ir_block& block = get_entry_block (my_func);
    block.set_name ("entry");

    block.append_instruction<Op> (var_z, var_x, var_y);

    ir_static_function my_static_func = generate_static_function (my_func);

    if (printing_enabled)
      std::cout << my_static_func << std::endl;

    auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();
    jit.enable_printing (printing_enabled);

    auto sym = reinterpret_cast<prototype> (jit.compile (my_static_func));
    result_type res = sym (lhs, rhs);
    if (! binary_compare (expected, res))
    {
      std::ostringstream sout;

      sout << "Result:    " << res << "\n";
      sout << "Expected:  " << expected << "\n";
      sout << "Operation: " << ir_instruction_traits<Op>::name << " ("
           << ir_type_v<lhs_type> << " x = " << lhs << ", "
           << ir_type_v<rhs_type> << " y = " << rhs << ")\n";
      sout << "Static IR:\n" << my_static_func;
      sout << std::endl;

      throw std::runtime_error (sout.str ());
    }

    std::cout << "OK: " << ir_instruction_traits<Op>::name << " ("
                        << ir_type_v<lhs_type> << " x = " << lhs << ", "
                        << ir_type_v<rhs_type> << " y = " << rhs << ") == "
                        << ir_type_v<result_type> << " z = " << expected << std::endl;
  }

  template <ir_opcode Op, typename ResultType, typename ArgType>
  void
  test_unary (ResultType expected, ArgType arg, bool printing_enabled = false)
  {
    using result_type = ResultType;
    using arg_type    = ArgType;

    using prototype = result_type (*) (arg_type);

    ir_function my_func ({ "z", ir_type_v<result_type> }, { { "x", ir_type_v<arg_type> } });
    ir_variable& var_x = my_func.get_variable ("x");
    ir_variable& var_z = my_func.get_variable ("z");

    ir_block& block = get_entry_block (my_func);
    block.set_name ("entry");

    block.append_instruction<Op> (var_z, var_x);

    ir_static_function my_static_func = generate_static_function (my_func);

    auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();
    jit.enable_printing (printing_enabled);

    if (printing_enabled)
      std::cout << my_static_func << std::endl;

    auto sym = reinterpret_cast<prototype> (jit.compile (my_static_func));
    result_type res = sym (arg);
    if (! binary_compare (expected, res))
    {
      std::ostringstream sout;

      sout << "Result:    " << res << "\n";
      sout << "Expected:  " << expected << "\n";
      sout << "Operation: " << ir_instruction_traits<Op>::name << " ("
           << ir_type_v<arg_type> << " x = " << arg << ")\n";
      sout << "Static IR:\n" << my_static_func;
      sout << std::endl;

      throw std::runtime_error (sout.str ());
    }

    std::cout << "OK: " << ir_instruction_traits<Op>::name << " ("
                        << ir_type_v<arg_type> << " x = " << arg << ") == "
                        << ir_type_v<result_type> << " z = " << expected << std::endl;
  }

}

#endif // OCTAVE_IR_TEST_TEMPLATES_HPP

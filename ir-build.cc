/*

Copyright (C) 2019 Gene Harvey

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "ir-build.h"
#include "ir-function.h"
#include "pt-all.h"

namespace octave
{
  
  void
  ir_builder1::compile (tree& tr)
  {
    try
      {
        tr.accept (*this);
      }
    catch (compile_exception& e)
      {
        if (e.is_fatal ())
          reset ();
        throw;
      }
  }

  void
  ir_builder1::visit_anon_fcn_handle (tree_anon_fcn_handle& handle)
  {
  }

  void
  ir_builder1::visit_argument_list (tree_argument_list& list)
  {
  }

  void
  ir_builder1::visit_binary_expression (tree_binary_expression& expression)
  {
  }

  void
  ir_builder1::visit_break_command (tree_break_command& command)
  {
  }
  void
  ir_builder1::visit_colon_expression (tree_colon_expression& expression)
  {
  }

  void
  ir_builder1::visit_continue_command (tree_continue_command& command)
  {
  }

  void
  ir_builder1::visit_decl_command (tree_decl_command& command)
  {
  }

  void
  ir_builder1::visit_decl_elt (tree_decl_elt& elt)
  {
  }

  void
  ir_builder1::visit_decl_init_list (tree_decl_init_list& list)
  {
  }

  void
  ir_builder1::visit_simple_for_command (tree_simple_for_command& command)
  {
  }

  void
  ir_builder1::visit_complex_for_command (tree_complex_for_command& command)
  {
  }

  void
  ir_builder1::visit_octave_user_script (octave_user_script& script)
  {
  }

  void
  ir_builder1::visit_octave_user_function (octave_user_function& function)
  {
  }

  void
  ir_builder1::visit_function_def (tree_function_def& def)
  {
  }

  void
  ir_builder1::visit_identifier (tree_identifier& identifier)
  {
  }

  void
  ir_builder1::visit_if_clause (tree_if_clause& clause)
  {
  }

  void
  ir_builder1::visit_if_command (tree_if_command& command)
  {
  }

  void
  ir_builder1::visit_if_command_list (tree_if_command_list& list)
  {
  }

  void
  ir_builder1::visit_switch_case (tree_switch_case& a_case)
  {
  }

  void
  ir_builder1::visit_switch_case_list (tree_switch_case_list& list)
  {
  }

  void
  ir_builder1::visit_switch_command (tree_switch_command& cmd)
  {
    tree_expression *expr = cmd.switch_value ();
  
    if (! expr)
      error ("missing value in switch command near line %d, column %d",
             cmd.line (), cmd.column ());
  }

  void
  ir_builder1::visit_index_expression (tree_index_expression& expression)
  {
  }

  void
  ir_builder1::visit_matrix (tree_matrix& matrix)
  {
  }

  void
  ir_builder1::visit_cell (tree_cell& cell)
  {
  }

  void
  ir_builder1::visit_multi_assignment (tree_multi_assignment& assignment)
  {
  }

  void
  ir_builder1::visit_no_op_command (tree_no_op_command& command)
  {
  }

  void
  ir_builder1::visit_constant (tree_constant& constant)
  {
  }

  void
  ir_builder1::visit_fcn_handle (tree_fcn_handle& handle)
  {
  }

  void
  ir_builder1::visit_parameter_list (tree_parameter_list& list)
  {
  }

  void
  ir_builder1::visit_postfix_expression (tree_postfix_expression& expression)
  {
  }

  void
  ir_builder1::visit_prefix_expression (tree_prefix_expression& expression)
  {
  }

  void
  ir_builder1::visit_return_command (tree_return_command& command)
  {
  }

  void
  ir_builder1::visit_return_list (tree_return_list& list)
  {
  }

  void
  ir_builder1::visit_simple_assignment (tree_simple_assignment& assignment)
  {
  }

  void
  ir_builder1::visit_statement (tree_statement& statement)
  {
  }

  void
  ir_builder1::visit_statement_list (tree_statement_list& list)
  {
  }

  void
  ir_builder1::visit_try_catch_command (tree_try_catch_command& command)
  {
  }

  void
  ir_builder1::visit_unwind_protect_command (tree_unwind_protect_command& command)
  {
  }

  void
  ir_builder1::visit_while_command (tree_while_command& command)
  {
  }

  void
  ir_builder1::visit_do_until_command (tree_do_until_command& command)
  {
  }

  void
  ir_builder1::visit_superclass_ref (tree_superclass_ref& ref)
  {
  
  }

  void
  ir_builder1::visit_metaclass_query (tree_metaclass_query& query)
  {
  
  }

  void
  ir_builder1::visit_classdef_attribute (tree_classdef_attribute& attribute)
  {
  
  }

  void
  ir_builder1::visit_classdef_attribute_list (tree_classdef_attribute_list& list)
  {
  
  }

  void
  ir_builder1::visit_classdef_superclass (tree_classdef_superclass& superclass)
  {
  
  }

  void
  ir_builder1::visit_classdef_superclass_list (
    tree_classdef_superclass_list& list)
  {
    tree_walker::visit_classdef_superclass_list (list);
  }

  void
  ir_builder1::visit_classdef_property (
    tree_classdef_property& a_property)
  {
    tree_walker::visit_classdef_property (a_property);
  }

  void
  ir_builder1::visit_classdef_property_list (
    tree_classdef_property_list& list)
  {
    tree_walker::visit_classdef_property_list (list);
  }

  void
  ir_builder1::visit_classdef_properties_block (
    tree_classdef_properties_block& block)
  {
    tree_walker::visit_classdef_properties_block (block);
  }

  void
  ir_builder1::visit_classdef_methods_list (
    tree_classdef_methods_list& list)
  {
    tree_walker::visit_classdef_methods_list (list);
  }

  void
  ir_builder1::visit_classdef_methods_block (
    tree_classdef_methods_block& block)
  {
    tree_walker::visit_classdef_methods_block (block);
  }

  void
  ir_builder1::visit_classdef_event (tree_classdef_event& event)
  {
    tree_walker::visit_classdef_event (event);
  }

  void
  ir_builder1::visit_classdef_events_list (
    tree_classdef_events_list& list)
  {
    tree_walker::visit_classdef_events_list (list);
  }

  void
  ir_builder1::visit_classdef_events_block (
    tree_classdef_events_block& block)
  {
    tree_walker::visit_classdef_events_block (block);
  }
  void
  ir_builder1::visit_classdef_enum (tree_classdef_enum& an_enum)
  {
    tree_walker::visit_classdef_enum (an_enum);
  }

  void
  ir_builder1::visit_classdef_enum_list (
    tree_classdef_enum_list& list)
  {
    tree_walker::visit_classdef_enum_list (list);
  }

  void
  ir_builder1::visit_classdef_enum_block (
    tree_classdef_enum_block& block)
  {
    tree_walker::visit_classdef_enum_block (block);
  }

  void
  ir_builder1::visit_classdef_body (tree_classdef_body& body)
  {
    tree_walker::visit_classdef_body (body);
  }

  void
  ir_builder1::visit_classdef (tree_classdef& classdef)
  {
    tree_walker::visit_classdef (classdef);
  }
}

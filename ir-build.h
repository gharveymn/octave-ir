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

#if ! defined (octave_ir_build_h)
#define octave_ir_build_h 1

#include "octave-config.h"

#include "pt-walk.h"
#include "ir-function.h"

namespace octave
{
  class tree;
  
  class ir_builder1 : public tree_walker
  {
  public:
    
    class compile_exception : public std::exception
    {
    public:
      
      enum class state
      {
        STABLE,
        FATAL
      };
      
      compile_exception (tree& loc, state module_state, const char *str)
        : m_location (loc),
          m_function_state (module_state),
          m_str (str)
      { }

      compile_exception (tree& loc, state module_state, std::string str)
        : m_location (loc),
          m_function_state (module_state),
          m_str (std::move (str))
      { }
  
      const char* what (void) const noexcept override
      {
        return m_str.c_str ();
      }
      
      constexpr bool is_fatal (void) const
      {
        return m_function_state == state::FATAL;
      }
      
    private:
      tree& m_location;
      const state m_function_state;
      std::string m_str;
    };
    
    //
    // members
    //
    
    void compile (tree& tr);

    ir_function
    transfer (void) noexcept
    {
      return std::move (m_function);
    }
    
    void reset (void) noexcept
    {
      m_function.reset ();
    }
    
    //
    // overrides
    //
    
    void visit_anon_fcn_handle (tree_anon_fcn_handle& handle) override;
    void visit_argument_list (tree_argument_list& list) override;
    void visit_binary_expression (tree_binary_expression& expression) override;
    void visit_break_command (tree_break_command& command) override;
    void visit_colon_expression (tree_colon_expression& expression) override;
    void visit_continue_command (tree_continue_command& command) override;
    void visit_decl_command (tree_decl_command& command) override;
    void visit_decl_elt (tree_decl_elt& elt) override;
    void visit_decl_init_list (tree_decl_init_list& list) override;
    void visit_simple_for_command (tree_simple_for_command& command) override;
    void visit_complex_for_command (tree_complex_for_command& command) override;
    void visit_octave_user_script (octave_user_script& script) override;
    void visit_octave_user_function (octave_user_function& function) override;
    void visit_function_def (tree_function_def& def) override;
    void visit_identifier (tree_identifier& identifier) override;
    void visit_if_clause (tree_if_clause& clause) override;
    void visit_if_command (tree_if_command& command) override;
    void visit_if_command_list (tree_if_command_list& list) override;
    void visit_switch_case (tree_switch_case& a_case) override;
    void visit_switch_case_list (tree_switch_case_list& list) override;
    void visit_switch_command (tree_switch_command& command) override;
    void visit_index_expression (tree_index_expression& expression) override;
    void visit_matrix (tree_matrix& matrix) override;
    void visit_cell (tree_cell& cell) override;
    void visit_multi_assignment (tree_multi_assignment& assignment) override;
    void visit_no_op_command (tree_no_op_command& command) override;
    void visit_constant (tree_constant& constant) override;
    void visit_fcn_handle (tree_fcn_handle& handle) override;
    void visit_parameter_list (tree_parameter_list& list) override;
    void visit_postfix_expression (tree_postfix_expression& expression) override;
    void visit_prefix_expression (tree_prefix_expression& expression) override;
    void visit_return_command (tree_return_command& command) override;
    void visit_return_list (tree_return_list& list) override;
    void visit_simple_assignment (tree_simple_assignment& assignment) override;
    void visit_statement (tree_statement& statement) override;
    void visit_statement_list (tree_statement_list& list) override;
    void visit_try_catch_command (tree_try_catch_command& command) override;
    void visit_unwind_protect_command (tree_unwind_protect_command& command) override;
    void visit_while_command (tree_while_command& command) override;
    void visit_do_until_command (tree_do_until_command& command) override;
    void visit_superclass_ref (tree_superclass_ref& ref) override;
    void visit_metaclass_query (tree_metaclass_query& query) override;
    void visit_classdef_attribute (tree_classdef_attribute& attribute) override;
    void visit_classdef_attribute_list (tree_classdef_attribute_list& list) override;
    void visit_classdef_superclass (tree_classdef_superclass& superclass) override;
    void visit_classdef_superclass_list (tree_classdef_superclass_list& list) override;
    void visit_classdef_property (tree_classdef_property& a_property) override;
    void visit_classdef_property_list (tree_classdef_property_list& list) override;
    void visit_classdef_properties_block (tree_classdef_properties_block& block) override;
    void visit_classdef_methods_list (tree_classdef_methods_list& list) override;
    void visit_classdef_methods_block (tree_classdef_methods_block& block) override;
    void visit_classdef_event (tree_classdef_event& event) override;
    void visit_classdef_events_list (tree_classdef_events_list& list) override;
    void visit_classdef_events_block (tree_classdef_events_block& block) override;
    void visit_classdef_enum (tree_classdef_enum& an_enum) override;
    void visit_classdef_enum_list (tree_classdef_enum_list& list) override;
    void visit_classdef_enum_block (tree_classdef_enum_block& block) override;
    void visit_classdef_body (tree_classdef_body& body) override;
    void visit_classdef (tree_classdef& classdef) override;
    
  private:
    
    enum class state
    {
      OK,
      ERROR
    } m_state;

    ir_function m_function;
  
  };
}

#endif

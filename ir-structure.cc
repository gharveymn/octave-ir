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

#include "ir-structure.h"
#include "ir-component.h"
#include "ir-block.h"

#include "pt-all.h"

#include <algorithm>

namespace octave
{
  
  class ir_builder : public tree_walker
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
        : m_location (&loc),
          m_function_state (module_state),
          m_str (str)
      { }
      
      compile_exception (tree& loc, state module_state, std::string str)
        : m_location (&loc),
          m_function_state (module_state),
          m_str (std::move (str))
      { }
      
      compile_exception (state module_state, const char *str)
        : m_function_state (module_state),
          m_str (str)
      { }
      
      compile_exception (state module_state, std::string str)
        : m_function_state (module_state),
          m_str (std::move (str))
      { }
  
      explicit compile_exception (const char *str)
        : m_str (str)
      { }
  
      explicit compile_exception (std::string str)
        : m_str (std::move (str))
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
      tree *m_location             = nullptr;
      const state m_function_state = state::FATAL;
      std::string m_str;
    };
    
    //
    // constructors
    //
    
    ir_builder (ir_structure& parent)
      : m_parent (parent)
    { }
    
    //
    // members
    //
    
    void compile (tree& tr)
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
    
    void reset (void) noexcept
    {
      m_parent.reset ();
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
    
    void visit_superclass_ref (tree_superclass_ref& tr) override
    {
      classdef_error ();
    }
    
    void visit_metaclass_query (tree_metaclass_query& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_attribute (tree_classdef_attribute& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_attribute_list (tree_classdef_attribute_list& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_superclass (tree_classdef_superclass& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_superclass_list (tree_classdef_superclass_list& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_property (tree_classdef_property& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_property_list (tree_classdef_property_list& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_properties_block (tree_classdef_properties_block& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_methods_list (tree_classdef_methods_list& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_methods_block (tree_classdef_methods_block& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_event (tree_classdef_event& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_events_list (tree_classdef_events_list& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_events_block (tree_classdef_events_block& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_enum (tree_classdef_enum& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_enum_list (tree_classdef_enum_list& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_enum_block (tree_classdef_enum_block& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef_body (tree_classdef_body& tr) override
    {
      classdef_error ();
    }
    
    void visit_classdef (tree_classdef& tr) override
    {
      classdef_error ();
    }
  
  private:
    
    [[noreturn]]
    static void classdef_error (void)
    {
      throw compile_exception ("function not implemented for classdef");
    }
    
    ir_structure& m_parent;
    
  };
  
  //
  // ir_structure
  //

  ir_structure::~ir_structure (void) noexcept = default;

  void
  ir_structure::leaf_push_back (ir_basic_block *blk)
  {
    m_leaf_cache.push_back (blk);
  }

  template <typename It>
  void
  ir_structure::leaf_push_back (It first, It last)
  {
    std::copy (first, last, std::back_inserter (m_leaf_cache));
  }

  ir_structure::link_iter
  ir_structure::leaf_begin (void)
  {
    if (leaf_cache_empty ())
      generate_leaf_cache ();
    return link_iter (m_leaf_cache.begin ());
  }

  ir_structure::link_iter
  ir_structure::leaf_end (void)
  {
    if (leaf_cache_empty ())
      generate_leaf_cache ();
    return link_iter (m_leaf_cache.end ());
  }

  //
  // ir_fork_component
  //

  ir_fork_component::~ir_fork_component (void) noexcept = default;

  ir_component::link_iter
  ir_fork_component::pred_begin (ir_component *c)
  {
    if (c == &m_condition)
      return m_parent.pred_begin (this);
    return link_iter (&m_condition);
  }

  ir_component::link_iter
  ir_fork_component::pred_end (ir_component *c)
  {
    if (c == &m_condition)
      return m_parent.pred_end (this);
    return ++link_iter (&m_condition);
  }

  ir_component::link_iter
  ir_fork_component::succ_begin (ir_component *c)
  {
    if (c == &m_condition)
      return sub_entry_begin ();
    return m_parent.succ_begin (this);
  }

  ir_component::link_iter
  ir_fork_component::succ_end (ir_component *c)
  {
    if (c == &m_condition)
      return sub_entry_end ();
    return m_parent.succ_end (this);
  }

  bool
  ir_fork_component::is_leaf_component (ir_component *c) noexcept
  {
    // assumes that c is in the component
    return c != &m_condition;
  }

  void
  ir_fork_component::generate_leaf_cache (void)
  {
    for (comp_ref c_uptr : m_subcomponents)
      leaf_push_back (c_uptr->leaf_begin (), c_uptr->leaf_end ());
  }

  ir_component::link_iter
  ir_fork_component::sub_entry_begin (void)
  {
    if (m_sub_entry_cache.empty ())
      generate_sub_entry_cache ();
    return link_iter (m_sub_entry_cache.begin ());
  }

  ir_component::link_iter
  ir_fork_component::sub_entry_end (void)
  {
    if (m_sub_entry_cache.empty ())
      generate_sub_entry_cache ();
    return link_iter (m_sub_entry_cache.end ());
  }

  void
  ir_fork_component::generate_sub_entry_cache (void)
  {
    for (comp_ref c_uptr : m_subcomponents)
      leaf_push_back (c_uptr->get_entry_block ());
  }

  //
  // ir_loop_component
  //

  ir_loop_component::~ir_loop_component (void) noexcept = default;

  ir_component::link_iter
  ir_loop_component::pred_begin (ir_component *c)
  {
    if (c == &m_entry)
      return m_parent.pred_begin (this);
    else if (c == &m_condition)
      return link_iter (m_cond_preds.begin ());
    else if (c == &m_body)
      return link_iter (&m_condition);
    throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::pred_end (ir_component *c)
  {
    if (c == &m_entry)
      return m_parent.pred_end (this);
    else if (c == &m_condition)
      return link_iter (m_cond_preds.end ());
    else if (c == &m_body)
      return ++link_iter (&m_condition);
    throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::succ_begin (ir_component *c)
  {
    if (c == &m_entry)
      return link_iter (&m_condition);
    else if (c == &m_condition)
      return cond_succ_begin ();
    else if (c == &m_body)
      return link_iter (&m_condition);
    throw ir_exception ("Component was not in the loop component.");
  }

  ir_component::link_iter
  ir_loop_component::succ_end (ir_component *c)
  {
    if (c == &m_entry)
      return m_parent.pred_end (this);
    else if (c == &m_condition)
      return cond_succ_end ();
    else if (c == &m_body)
      return ++link_iter (&m_condition);
    throw ir_exception ("Component was not in the loop component.");
  }
  
  ir_basic_block *
  ir_loop_component::get_update_block (void) const noexcept
  {
    return static_cast<ir_basic_block *> (m_body.back ().get ());
  }

  ir_component::link_iter
  ir_loop_component::cond_succ_begin (void)
  {
    m_succ_cache.front () = m_body.get_entry_block ();
    return link_iter (m_succ_cache.begin ());
  }

  ir_component::link_iter
  ir_loop_component::cond_succ_end (void)
  {
    return link_iter (m_succ_cache.end ());
  }

}

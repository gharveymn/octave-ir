/** ir-function.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include "ir-function.hpp"

#include "ir-block.hpp"
#include "ir-component-sequence.hpp"
#include "ir-all-component-visitors.hpp"
#include "ir-all-structure-visitors.hpp"

#include "ir-static-function.hpp"
#include "ir-error.hpp"

#include <gch/nonnull_ptr.hpp>

namespace gch
{

  ir_function::
  ir_function (std::string_view name)
    : ir_structure (),
      m_name (name),
      m_body (allocate_subcomponent<ir_component_sequence> (ir_subcomponent_type<ir_block>))
  { }

  ir_function::
  ir_function (std::initializer_list<std::string_view> ret_names,
               std::initializer_list<std::string_view> arg_names,
               std::string_view name)
    : ir_function (ret_names.begin (), ret_names.end (), arg_names.begin (), arg_names.end (),
                   name)
  { }

  ir_function::
  ir_function (std::string_view ret_name,
               std::initializer_list<std::string_view> arg_names,
               std::string_view name)
    : ir_function ({ ret_name }, arg_names, name)
  { }

  ir_function::
  ir_function (std::initializer_list<std::string_view> arg_names, std::string_view name)
    : ir_function (decltype (arg_names) { }, arg_names.begin (), arg_names.end (), name)
  { }

  ir_function::
  ir_function (std::initializer_list<std::string_view> ret_names,
               std::initializer_list<ir_variable_pair> args,
               std::string_view name)
    : ir_function (ret_names.begin (), ret_names.end (), args.begin (), args.end (), name)
  { }

  ir_function::
  ir_function (std::string_view ret_name,
               std::initializer_list<ir_variable_pair> args,
               std::string_view name)
    : ir_function ({ ret_name }, args.begin (), args.end (), name)
  { }

  ir_function::
  ir_function (std::initializer_list<ir_variable_pair> rets,
               std::initializer_list<std::string_view> arg_names,
               std::string_view name)
    : ir_function (rets.begin (), rets.end (), arg_names.begin (), arg_names.end (), name)
  { }

  ir_function::
  ir_function (ir_variable_pair ret,
               std::initializer_list<std::string_view> arg_names,
               std::string_view name)
    : ir_function ({ ret }, arg_names.begin (), arg_names.end (), name)
  { }

  ir_function::
  ir_function (std::initializer_list<ir_variable_pair> rets,
               std::initializer_list<ir_variable_pair> args,
               std::string_view name)
    : ir_function (rets.begin (), rets.end (), args.begin (), args.end (), name)
  { }

  ir_function::
  ir_function (ir_variable_pair ret,
               std::initializer_list<ir_variable_pair> args,
               std::string_view name)
    : ir_function ({ ret }, args.begin (), args.end (), name)
  { }

  ir_function::
  ir_function (std::initializer_list<ir_variable_pair> args,
               std::string_view name)
    : ir_function (decltype (args) { }, args.begin (), args.end (), name)
  { }

  ir_function::
  ir_function (ir_variable_pair ret, std::string_view name)
    : ir_function (ret, std::initializer_list<ir_variable_pair> { }, name)
  { }

  ir_function::
  ir_function (std::string_view ret_name, std::string_view name)
    : ir_function (ret_name, std::initializer_list<std::string_view> { }, name)
  { }

  ir_function::
  ~ir_function (void) = default;

  void
  ir_function::
  create_entry_instruction (ir_variable& var)
  {
    // Note: This will never have any incoming timelines.
    ir_subcomponent& entry_component = static_cast<ir_component_sequence&> (*m_body).front ();
    ir_block& entry_block = static_cast<ir_block&> (entry_component);
    ir_def_timeline& var_dt = entry_block.get_def_timeline (var);
    var_dt.create_incoming_timeline ();
  }

  ir_subcomponent&
  ir_function::
  get_body (void) noexcept
  {
    return *m_body;
  }

  const ir_subcomponent&
  ir_function::
  get_body (void) const noexcept
  {
    return as_mutable (*this).get_body ();
  }

  bool
  ir_function::
  is_body (const ir_subcomponent& sub) const noexcept
  {
    return &sub == &get_body ();
  }

  ir_variable&
  ir_function::
  create_variable (ir_type type)
  {
    auto [it, ins] = m_variable_map.try_emplace (ir_variable::anonymous_name, *this, type);
    assert (ins && "Tried to create a variable which already exists.");
    return std::get<ir_variable> (*it);
  }

  ir_variable&
  ir_function::
  create_variable (ir_variable_pair pair)
  {
    return create_variable (pair.name, pair.type);
  }

  void
  ir_function::
  set_variable_type (const std::string& name, ir_type type)
  {
    auto found = m_variable_map.find (name);
    assert (found != m_variable_map.end ()
        &&  "Tried to set the variable type of a nonexistent variable.");
    found->second.set_type (type);
  }

  auto
  ir_function::
  variables_begin (void) noexcept
    -> variables_iter
  {
    return m_variable_map.begin ();
  }

  auto
  ir_function::
  variables_begin (void) const noexcept
    -> variables_citer
  {
    return as_mutable (*this).variables_begin ();
  }

  auto
  ir_function::
  variables_cbegin (void) const noexcept
    -> variables_citer
  {
    return variables_begin ();
  }

  auto
  ir_function::
  variables_end (void) noexcept
    -> variables_iter
  {
    return m_variable_map.end ();
  }

  auto
  ir_function::
  variables_end (void) const noexcept
    -> variables_citer
  {
    return as_mutable (*this).variables_end ();
  }

  auto
  ir_function::
  variables_cend (void) const noexcept
    -> variables_citer
  {
    return variables_end ();
  }

  auto
  ir_function::
  variables_front (void)
    -> variables_ref
  {
    return *variables_begin ();
  }

  auto
  ir_function::
  variables_front (void) const
    -> variables_cref
  {
    return as_mutable (*this).variables_front ();
  }

  auto
  ir_function::
  variables_back (void)
    -> variables_ref
  {
    return *std::prev (variables_end ());
  }

  auto
  ir_function::
  variables_back (void) const
    -> variables_cref
  {
    return as_mutable (*this).variables_back ();
  }

  auto
  ir_function::
  num_variables (void) const noexcept
    -> variables_size_ty
  {
    return m_variable_map.size ();
  }

  bool
  ir_function::
  has_variables (void) const noexcept
  {
    return m_variable_map.empty ();
  }

  auto
  ir_function::
  args_begin (void) noexcept
    -> args_iter
  {
    return m_args.begin ();
  }

  auto
  ir_function::
  args_begin (void) const noexcept
    -> args_citer
  {
    return as_mutable (*this).args_begin ();
  }

  auto
  ir_function::
  args_cbegin (void) const noexcept
    -> args_citer
  {
    return args_begin ();
  }

  auto
  ir_function::
  args_end (void) noexcept
    -> args_iter
  {
    return m_args.end ();
  }

  auto
  ir_function::
  args_end (void) const noexcept
    -> args_citer
  {
    return as_mutable (*this).args_end ();
  }

  auto
  ir_function::
  args_cend (void) const noexcept
    -> args_citer
  {
    return args_end ();
  }

  auto
  ir_function::
  args_rbegin (void) noexcept
    -> args_riter
  {
    return m_args.rbegin ();
  }

  auto
  ir_function::
  args_rbegin (void) const noexcept
    -> args_criter
  {
    return as_mutable (*this).args_rbegin ();
  }

  auto
  ir_function::
  args_crbegin (void) const noexcept
    -> args_criter
  {
    return args_rbegin ();
  }

  auto
  ir_function::
  args_rend (void) noexcept
    -> args_riter
  {
    return m_args.rend ();
  }

  auto
  ir_function::
  args_rend (void) const noexcept
    -> args_criter
  {
    return as_mutable (*this).args_rend ();
  }

  auto
  ir_function::
  args_crend (void) const noexcept
    -> args_criter
  {
    return args_rend ();
  }

  auto
  ir_function::
  args_front (void)
    -> args_ref
  {
    return *args_begin ();
  }

  auto
  ir_function::
  args_front (void) const
    -> args_cref
  {
    return as_mutable (*this).args_front ();
  }

  auto
  ir_function::
  args_back (void)
    -> args_ref
  {
    return *args_rbegin ();
  }

  auto
  ir_function::
  args_back (void) const
    -> args_cref
  {
    return as_mutable (*this).args_back ();
  }

  auto
  ir_function::
  args_size (void) const noexcept
    -> args_size_ty
  {
   return m_args.size ();
  }

  bool
  ir_function::
  args_empty (void) const noexcept
  {
    return m_args.empty ();
  }

  auto
  ir_function::
  returns_begin (void) noexcept
    -> returns_iter
  {
    return m_ret.begin ();
  }

  auto
  ir_function::
  returns_begin (void) const noexcept
    -> returns_citer
  {
    return as_mutable (*this).returns_begin ();
  }

  auto
  ir_function::
  returns_cbegin (void) const noexcept
    -> returns_citer
  {
    return returns_begin ();
  }

  auto
  ir_function::
  returns_end (void) noexcept
    -> returns_iter
  {
    return m_ret.end ();
  }

  auto
  ir_function::
  returns_end (void) const noexcept
    -> returns_citer
  {
    return as_mutable (*this).returns_end ();
  }

  auto
  ir_function::
  returns_cend (void) const noexcept
    -> returns_citer
  {
    return returns_end ();
  }

  auto
  ir_function::
  returns_rbegin (void) noexcept
    -> returns_riter
  {
    return m_ret.rbegin ();
  }

  auto
  ir_function::
  returns_rbegin (void) const noexcept
    -> returns_criter
  {
    return as_mutable (*this).returns_rbegin ();
  }

  auto
  ir_function::
  returns_crbegin (void) const noexcept
    -> returns_criter
  {
    return returns_rbegin ();
  }

  auto
  ir_function::
  returns_rend (void) noexcept
    -> returns_riter
  {
    return m_ret.rend ();
  }

  auto
  ir_function::
  returns_rend (void) const noexcept
    -> returns_criter
  {
    return as_mutable (*this).returns_rend ();
  }

  auto
  ir_function::
  returns_crend (void) const noexcept
    -> returns_criter
  {
    return returns_rend ();
  }

  auto
  ir_function::
  returns_front (void)
    -> returns_ref
  {
    return *returns_begin ();
  }

  auto
  ir_function::
  returns_front (void) const
    -> returns_cref
  {
    return as_mutable (*this).returns_front ();
  }

  auto
  ir_function::
  returns_back (void)
    -> returns_ref
  {
    return *returns_rbegin ();
  }

  auto
  ir_function::
  returns_back (void) const
    -> returns_cref
  {
    return as_mutable (*this).returns_back ();
  }

  auto
  ir_function::
  num_returns (void) const noexcept
    -> returns_size_ty
  {
    return m_ret.size ();
  }

  bool
  ir_function::
  has_returns (void) const noexcept
  {
    return ! m_ret.empty ();
  }

  std::string
  ir_function::
  get_name (void) const noexcept
  {
    if (m_name.empty ())
      return "function@" + std::to_string (reinterpret_cast<std::size_t> (this));
    return m_name;
  }

  ir_block&
  get_entry_block (ir_function& func)
  {
    return as_mutable (get_entry_block (std::as_const (func)));
  }

  const ir_block&
  get_entry_block (const ir_function& func)
  {
    return get_entry_block (func.get_body ());
  }

}

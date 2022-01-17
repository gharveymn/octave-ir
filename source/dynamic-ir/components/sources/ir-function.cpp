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
  ir_function (void)
    : m_body (allocate_subcomponent<ir_component_sequence> (ir_subcomponent_type<ir_block>))
  { }

  ir_function::
  ~ir_function (void) = default;

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
  get_variable (const std::string& identifier)
  {
    auto [it, inserted] = m_variable_map.try_emplace (identifier, *this, identifier);
    return std::get<ir_variable> (*it);
  }

  ir_variable&
  ir_function::
  get_variable (std::string&& identifier)
  {
    auto [it, inserted] = m_variable_map.try_emplace (identifier, *this, std::move (identifier));
    return std::get<ir_variable> (*it);
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
  num_args (void) const noexcept
    -> args_size_ty
  {
   return args_size ();
  }

  bool
  ir_function::
  has_args (void) const noexcept
  {
    return ! args_empty ();
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

/** ir-fork-component.cpp.cc
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-component-fork.hpp"
#include "ir-block.hpp"
#include "structure/inspectors/ir-ascending-def-resolution-builder.hpp"
#include "ir-all-substructure-visitors.hpp"

#include <algorithm>

namespace gch
{

  ir_component_fork::
  ir_component_fork (ir_structure& parent,
                     ir_variable& condition_var,
                     std::initializer_list<ir_component_mover> init)
    : ir_substructure (parent),
      m_condition     (*this, condition_var),
      m_cases         (init.begin (), init.end ())
  { }

  ir_component_fork::
  ir_component_fork (ir_structure& parent, ir_variable& condition_var, ir_component_mover init)
    : ir_component_fork (parent, condition_var, { init })
  { }

  ir_component_fork::
  ir_component_fork (ir_structure& parent, ir_variable& condition_var)
    : ir_component_fork (parent, condition_var, { })
  { }

  ir_component_fork::
  ~ir_component_fork (void) noexcept = default;

  ir_block&
  ir_component_fork::
  get_condition (void) noexcept
  {
    return m_condition;
  }

  const ir_block&
  ir_component_fork::
  get_condition (void) const noexcept
  {
    return as_mutable (*this).get_condition ();
  }

  bool
  ir_component_fork::
  is_condition (const ir_subcomponent& sub) const noexcept
  {
    return &sub == &get_condition ();
  }

  auto
  ir_component_fork::
  cases_begin (void) noexcept
    -> cases_iter
  {
    return make_ptr (m_cases.begin ());
  }

  auto
  ir_component_fork::
  cases_begin (void) const noexcept
    -> cases_citer
  {
    return as_mutable (*this).cases_begin ();
  }

  auto
  ir_component_fork::
  cases_cbegin (void) const noexcept
    -> cases_citer
  {
    return cases_begin ();
  }

  auto
  ir_component_fork::
  cases_end (void) noexcept
    -> cases_iter
  {
    return make_ptr (m_cases.end ());
  }

  auto
  ir_component_fork::
  cases_end (void) const noexcept
    -> cases_citer
  {
    return as_mutable (*this).cases_end ();
  }

  auto
  ir_component_fork::
  cases_cend (void) const noexcept
    -> cases_citer
  {
    return cases_end ();
  }

  auto
  ir_component_fork::
  cases_rbegin (void) noexcept
    -> cases_riter
  {
    return cases_riter { cases_end () };
  }

  auto
  ir_component_fork::
  cases_rbegin (void) const noexcept
    -> cases_criter
  {
    return as_mutable (*this).cases_rbegin ();
  }

  auto
  ir_component_fork::
  cases_crbegin (void) const noexcept
    -> cases_criter
  {
    return cases_rbegin ();
  }

  auto
  ir_component_fork::
  cases_rend (void) noexcept
    -> cases_riter
  {
    return cases_riter { cases_begin () };
  }

  auto
  ir_component_fork::
  cases_rend (void) const noexcept
    -> cases_criter
  {
    return as_mutable (*this).cases_rend ();
  }

  auto
  ir_component_fork::
  cases_crend (void) const noexcept
    -> cases_criter
  {
    return cases_rend ();
  }

  auto
  ir_component_fork::
  cases_front (void)
    -> cases_ref
  {
    return *cases_begin ();
  }

  auto
  ir_component_fork::
  cases_front (void) const
    -> cases_cref
  {
    return as_mutable (*this).cases_front ();
  }

  auto
  ir_component_fork::
  cases_back (void)
    -> cases_ref
  {
    return *cases_rbegin ();
  }

  auto
  ir_component_fork::
  cases_back (void) const
    -> cases_cref
  {
    return as_mutable (*this).cases_back ();
  }

  auto
  ir_component_fork::
  num_cases (void) const noexcept
    -> cases_size_ty
  {
    return m_cases.size ();
  }

  bool
  ir_component_fork::
  has_cases (void) const noexcept
  {
    return ! m_cases.empty ();
  }

  auto
  ir_component_fork::
  find_case (ir_component& c) const
    -> cases_iter
  {
    return std::find_if (as_mutable (*this).cases_begin (), as_mutable (*this).cases_end (),
                         [&](const ir_component& cmp) { return &cmp == &c; });
  }

  auto
  ir_component_fork::
  find_case (const ir_component& c) const
    -> cases_citer
  {
    return find_case (as_mutable (c));
  }

}

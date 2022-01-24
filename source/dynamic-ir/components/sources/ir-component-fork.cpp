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
#include <numeric>

namespace gch
{

  ir_component_fork::
  ir_component_fork (ir_structure& parent, ir_variable& condition_var)
    : ir_substructure (parent),
      m_condition     (*this, condition_var)
  { }

  ir_component_fork::
  ~ir_component_fork (void) noexcept = default;

  bool
  ir_component_fork::
  is_condition (const ir_subcomponent& sub) const noexcept
  {
    return &sub == &get_condition ();
  }

  auto
  ir_component_fork::
  find_case (ir_component& c) const
    -> cases_iter
  {
    return std::find_if (as_mutable (*this).cases_begin (), as_mutable (*this).cases_end (),
                         [&](const ir_component& cmp) { return &cmp == &c; });
  }

}

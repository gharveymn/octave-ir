/** ir-static-variable.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "values/static/ir-static-variable.hpp"

#include "utilities/ir-iterator.hpp"
#include "values/ir-variable.hpp"

namespace gch
{

  ir_static_variable::
  ir_static_variable (const ir_variable& var, std::size_t num_defs)
    : m_type     (var.get_type ()),
      m_name     (var.get_name ()),
      m_num_defs (num_defs)
  { }

  std::string_view
  ir_static_variable::
  get_name (void) const noexcept
  {
    return m_name;
  }

  ir_type
  ir_static_variable::
  get_type (void) const noexcept
  {
    return m_type;
  }

}

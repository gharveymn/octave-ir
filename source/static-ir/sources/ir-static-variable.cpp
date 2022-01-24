/** ir-static-variable.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-static-variable.hpp"

#include "ir-iterator.hpp"

namespace gch
{

  ir_static_variable::
  ir_static_variable (std::string_view name, ir_type type)
    : m_name (name),
      m_type (type)
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

  ir_static_def_id
  ir_static_variable::
  create_id (void) noexcept
  {
    return static_cast<ir_static_def_id> (m_num_defs++);
  }

  std::size_t
  ir_static_variable::
  get_num_defs (void) const noexcept
  {
    return m_num_defs;
  }

}

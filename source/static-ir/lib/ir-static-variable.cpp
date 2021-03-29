/** ir-static-variable.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-static-ir/ir-static-variable.hpp"

#include "gch/octave-ir-utilities/ir-iterator.hpp"

namespace gch
{

  ir_static_variable::
  ir_static_variable (std::string_view name, ir_type type, std::size_t num_defs)
    : m_name     (name),
      m_type     (type),
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

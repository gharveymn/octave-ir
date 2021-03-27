/** ir-static-def.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "values/static/ir-static-def.hpp"

#include "values/static/ir-static-variable.hpp"

namespace gch
{

  ir_static_def::
  ir_static_def (const ir_static_variable& var, ir_static_def_id id)
    : m_variable (var),
      m_id       (id)
  { }

  const ir_static_variable&
  ir_static_def::
  get_variable (void) const noexcept
  {
    return *m_variable;
  }

  ir_static_def_id
  ir_static_def::
  get_id (void) const noexcept
  {
    return m_id;
  }

  std::string_view
  ir_static_def::
  get_variable_name (void) const noexcept
  {
    return get_variable ().get_name ();
  }

  ir_type
  ir_static_def::
  get_type (void) const noexcept
  {
    return get_variable ().get_type ();
  }

}

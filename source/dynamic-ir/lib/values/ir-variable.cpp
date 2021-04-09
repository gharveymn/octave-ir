/** ir-variable.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "values/ir-variable.hpp"

#include "gch/octave-ir-static-ir/ir-type-util.hpp"

namespace gch
{

  ir_variable::
  ir_variable (const ir_component& c, std::string_view name)
    : m_component (c),
      m_name      (name)
  { }

  ir_variable::
  ir_variable (const ir_component& c, std::string_view name, ir_type type)
    : m_component (c),
      m_name      (name),
      m_type      (type)
  { }

  std::string_view
  ir_variable::
  get_name (void) const noexcept
  {
    return m_name;
  }

  const ir_component&
  ir_variable::
  get_component () const noexcept
  {
    return *m_component;
  }

  ir_type
  ir_variable::
  get_type (void) const noexcept
  {
    return m_type;
  }

  void
  ir_variable::
  set_type (ir_type ty)
  {
    m_type = ty;
  }

  auto
  ir_variable::
  create_id (void) noexcept
    -> id_type
  {
    return m_curr_id++;
  }

  ir_type
  common_type (const ir_variable& lhs, const ir_variable& rhs)
  {
    return lhs.get_type () ^ rhs.get_type ();
  }

}

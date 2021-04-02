/** ir-static-use.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-ir-static-ir/ir-static-use.hpp"

#include "gch/octave-ir-static-ir/ir-static-def.hpp"
#include "gch/octave-ir-static-ir/ir-static-variable.hpp"

#include <ostream>

namespace gch
{

  ir_static_use::
  ir_static_use (const ir_static_variable& var, ir_static_def_id id)
    : m_variable (var),
      m_id       (id)
  { }

  const ir_static_variable&
  ir_static_use::
  get_variable (void) const noexcept
  {
    return *m_variable;
  }

  ir_static_def_id
  ir_static_use::
  get_def_id (void) const noexcept
  {
    return m_id;
  }

  std::string_view
  ir_static_use::
  get_variable_name (void) const
  {
    return get_variable ().get_name ();
  }

  ir_type
  ir_static_use::
  get_type (void) const noexcept
  {
    return get_variable ().get_type ();
  }

  std::ostream&
  operator<< (std::ostream& out, const ir_static_use& use)
  {
    return out << use.get_variable_name () << use.get_def_id ();
  }

}
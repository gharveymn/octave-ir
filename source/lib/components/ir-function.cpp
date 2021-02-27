/*

Copyright (C) 2019 Gene Harvey

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/


#include "components/ir-function.hpp"

#include "components/ir-block.hpp"
#include "components/ir-component-sequence.hpp"

#include "utilities/ir-error.hpp"

#include <gch/nonnull_ptr.hpp>

namespace gch
{
  ir_function::
  ir_function (void)
    : m_body (allocate_subcomponent<ir_component_sequence> (ir_subcomponent_type<ir_block>))
  { }

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
  get_variable (const variable_key_type& identifier)
  {
    auto [it, inserted] = m_variable_map.try_emplace (identifier, *this, identifier);
    return std::get<ir_variable> (*it);
  }

  ir_variable&
  ir_function::
  get_variable (variable_key_type&& identifier)
  {
    auto [it, inserted] = m_variable_map.try_emplace (identifier, *this, std::move (identifier));
    return std::get<ir_variable> (*it);
  }

  ir_variable&
  ir_function::
  get_variable (const variable_identifier_char_type *identifier)
  {
    auto [it, inserted] = m_variable_map.try_emplace (identifier, *this, identifier);
    return std::get<ir_variable> (*it);
  }

}

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

#ifndef OCTAVE_IR_IR_FUNCTION_HPP
#define OCTAVE_IR_IR_FUNCTION_HPP

#include "components/ir-structure.hpp"
#include "values/ir-variable.hpp"

#include <unordered_map>

namespace gch
{

  class ir_function final
    : public ir_component,
      public ir_structure,
      public visitable<ir_function, consolidated_visitors_t<ir_component, ir_structure>>
  {
  public:
    using variable_identifier_char_type = char;
    using variable_key_type             = std::basic_string<variable_identifier_char_type>;

    ir_function            (void);
    ir_function            (const ir_function&)     = delete;
    ir_function            (ir_function&&) noexcept = default;
    ir_function& operator= (const ir_function&)     = delete;
    ir_function& operator= (ir_function&&) noexcept = delete;
    ~ir_function           (void) override          = default;

    [[nodiscard]]
    ir_subcomponent&
    get_body (void) noexcept;

    [[nodiscard]]
    const ir_subcomponent&
    get_body (void) const noexcept;

    [[nodiscard]]
    bool
    is_body (const ir_subcomponent& sub) const noexcept;

    ir_variable&
    get_variable (const variable_key_type& identifier);

    ir_variable&
    get_variable (variable_key_type&& identifier);

    ir_variable&
    get_variable (const variable_identifier_char_type *identifier);

  private:
    std::unordered_map<variable_key_type, ir_variable> m_variable_map;
    ir_component_storage                               m_body;
  };

  ir_block&
  get_entry_block (ir_function& c);

  const ir_block&
  get_entry_block (const ir_function& c);

}

#endif

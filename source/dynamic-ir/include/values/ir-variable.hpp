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

#ifndef OCTAVE_IR_IR_VARIABLE_HPP
#define OCTAVE_IR_IR_VARIABLE_HPP

#include "gch/octave-ir-static-ir/ir-type-base.hpp"

#include <gch/nonnull_ptr.hpp>

#include <string>
#include <string_view>

namespace gch
{

  class ir_component;

  class ir_variable
  {
  public:
    using id_type = int;

    static constexpr
    const char
    anonymous_name[] = "<@>";

    ir_variable            (void)                   = delete;
    ir_variable            (const ir_variable&)     = delete;
    ir_variable            (ir_variable&&) noexcept = delete;
    ir_variable& operator= (const ir_variable&)     = delete;
    ir_variable& operator= (ir_variable&&) noexcept = delete;
    ~ir_variable           (void)                   = default;

    ir_variable (const ir_component& c, std::string_view name);
    ir_variable (const ir_component& c, std::string_view name, ir_type type);

    [[nodiscard]]
    std::string_view
    get_name (void) const noexcept;

    [[nodiscard]]
    const ir_component&
    get_component (void) const noexcept;

    [[nodiscard]]
    ir_type
    get_type (void) const noexcept;

    void
    set_type (ir_type ty);

    id_type
    create_id (void) noexcept;

   private:
    nonnull_cptr<ir_component> m_component;
    const std::string          m_name     = anonymous_name;
    ir_type                    m_type     = ir_type_v<void>;
    id_type                    m_curr_id  = 0;
  };

  ir_type
  common_type (const ir_variable& lhs, const ir_variable& rhs);

  template <typename ...Vars>
  inline
  ir_type
  common_type (const Vars&... vars)
  {
    return (vars.get_type () ^ ...);
  }

}

#endif

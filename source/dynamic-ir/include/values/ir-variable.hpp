/** ir-variable.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_VARIABLE_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_VARIABLE_HPP

#include "gch/octave-ir-static-ir/ir-type.hpp"

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

#endif // OCTAVE_IR_DYNAMIC_IR_IR_VARIABLE_HPP

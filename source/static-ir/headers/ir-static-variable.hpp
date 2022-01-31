/** ir-static-variable.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_STATIC_IR_IR_STATIC_VARIABLE_HPP
#define OCTAVE_IR_STATIC_IR_IR_STATIC_VARIABLE_HPP

#include "ir-static-id.hpp"
#include "ir-type.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace gch
{

  class ir_variable;

  class ir_static_variable
  {
  public:
    ir_static_variable            (void)                          = delete;
    ir_static_variable            (const ir_static_variable&)     = delete;
    ir_static_variable            (ir_static_variable&&) noexcept = default;
    ir_static_variable& operator= (const ir_static_variable&)     = delete;
    ir_static_variable& operator= (ir_static_variable&&) noexcept = delete;
    ~ir_static_variable           (void)                          = default;

    ir_static_variable (std::string_view name, ir_type type);

    [[nodiscard]]
    std::string_view
    get_name (void) const noexcept;

    [[nodiscard]]
    ir_type
    get_type (void) const noexcept;

    [[nodiscard]]
    ir_static_def_id
    create_id (void) noexcept;

    [[nodiscard]]
    std::size_t
    get_num_defs (void) const noexcept;

  private:
    const std::string m_name;
    const ir_type     m_type;
    std::size_t       m_num_defs = 0;
  };

}

#endif // OCTAVE_IR_STATIC_IR_IR_STATIC_VARIABLE_HPP

/** ir-static-variable.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_STATIC_VARIABLE_HPP
#define OCTAVE_IR_IR_STATIC_VARIABLE_HPP

#include "values/static/ir-static-def.hpp"

#include <gch/small_vector.hpp>

namespace gch
{

  class ir_variable;

  class ir_static_variable
  {
  public:
    ir_static_variable            (void)                          = delete;
    ir_static_variable            (const ir_static_variable&)     = delete;
    ir_static_variable            (ir_static_variable&&) noexcept = delete;
    ir_static_variable& operator= (const ir_static_variable&)     = delete;
    ir_static_variable& operator= (ir_static_variable&&) noexcept = delete;
    ~ir_static_variable           (void)                          = default;

    ir_static_variable (const ir_variable& var, std::size_t num_defs);

    [[nodiscard]]
    std::string_view
    get_name (void) const noexcept;

    [[nodiscard]]
    ir_type
    get_type (void) const noexcept;

  private:
    const ir_type     m_type;
    const std::string m_name;
    const std::size_t m_num_defs;
  };

}

#endif // OCTAVE_IR_IR_STATIC_VARIABLE_HPP

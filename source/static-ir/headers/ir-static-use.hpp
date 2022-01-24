/** ir-static-use.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_STATIC_IR_IR_STATIC_USE_HPP
#define OCTAVE_IR_STATIC_IR_IR_STATIC_USE_HPP

#include "ir-static-def.hpp"

namespace gch
{

  class ir_static_variable;

  class ir_static_use
  {
  public:
    ir_static_use            (void)                     = delete;
    ir_static_use            (const ir_static_use&)     = default;
    ir_static_use            (ir_static_use&&) noexcept = default;
    ir_static_use& operator= (const ir_static_use&)     = default;
    ir_static_use& operator= (ir_static_use&&) noexcept = default;
    ~ir_static_use           (void)                     = default;

    constexpr
    ir_static_use (ir_static_variable_id var_id, std::optional<ir_static_def_id> id)
      : m_var_id (var_id),
        m_id     (id)
    { }

    [[nodiscard]]
    constexpr
    ir_static_variable_id
    get_variable_id (void) const noexcept
    {
      return m_var_id;
    }

    [[nodiscard]]
    constexpr
    bool
    has_def_id (void) const noexcept
    {
      return m_id.has_value ();
    }

    [[nodiscard]]
    constexpr
    ir_static_def_id
    get_def_id (void) const
    {
      return *m_id;
    }

    [[nodiscard]]
    constexpr
    std::optional<ir_static_def_id>
    maybe_get_def_id (void) const noexcept
    {
      return m_id;
    }

  private:
    ir_static_variable_id           m_var_id;
    std::optional<ir_static_def_id> m_id;
  };

}

#endif // OCTAVE_IR_STATIC_IR_IR_STATIC_USE_HPP

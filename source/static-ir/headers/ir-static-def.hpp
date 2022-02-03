/** ir-static-def.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_STATIC_IR_IR_STATIC_DEF_HPP
#define OCTAVE_IR_STATIC_IR_IR_STATIC_DEF_HPP

#include "ir-object-id.hpp"

namespace gch
{

  class ir_static_def
  {
  public:
    ir_static_def            (void)                     = delete;
    ir_static_def            (const ir_static_def&)     = default;
    ir_static_def            (ir_static_def&&) noexcept = default;
    ir_static_def& operator= (const ir_static_def&)     = default;
    ir_static_def& operator= (ir_static_def&&) noexcept = default;
    ~ir_static_def           (void)                     = default;

    constexpr
    ir_static_def (ir_variable_id var_id, ir_def_id id) noexcept
      : m_var_id (var_id),
        m_id     (id)
    { }

    [[nodiscard]]
    constexpr
    ir_variable_id
    get_variable_id (void) const noexcept
    {
      return m_var_id;
    }

    [[nodiscard]]
    constexpr
    ir_def_id
    get_id (void) const noexcept
    {
      return m_id;
    }

  private:
    ir_variable_id m_var_id;
    ir_def_id      m_id;
  };

  constexpr
  bool
  operator== (const ir_static_def& lhs, const ir_static_def& rhs) noexcept
  {
    return (lhs.get_variable_id () == rhs.get_variable_id ()) && (lhs.get_id () == rhs.get_id ());
  }

  constexpr
  bool
  operator!= (const ir_static_def& lhs, const ir_static_def& rhs) noexcept
  {
    return ! (lhs == rhs);
  }

}

#endif // OCTAVE_IR_STATIC_IR_IR_STATIC_DEF_HPP

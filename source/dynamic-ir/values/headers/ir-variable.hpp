/** ir-variable.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_VARIABLE_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_VARIABLE_HPP

#include "ir-type.hpp"
#include "ir-object-id.hpp"
#include "ir-static-variable.hpp"

#include <gch/nonnull_ptr.hpp>

#include <string>
#include <string_view>

namespace gch
{

  class ir_component;

  class ir_variable
  {
  public:
    static constexpr
    const char
    anonymous_name[] = ".";

    ir_variable            (void)                   = delete;
    ir_variable            (const ir_variable&)     = delete;
    ir_variable            (ir_variable&&) noexcept = delete;
    ir_variable& operator= (const ir_variable&)     = delete;
    ir_variable& operator= (ir_variable&&) noexcept = delete;
    ~ir_variable           (void)                   = default;

    ir_variable (const ir_component& c, ir_variable_id id, std::string_view name, ir_type type);

    ir_variable (const ir_component& c, ir_variable_id id, std::string_view name);

    ir_variable (const ir_component& c, ir_variable_id id, ir_type type);

    ir_variable (const ir_component& c, ir_variable_id id);

    ir_variable_id
    get_id (void) const noexcept;

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

    template <typename T>
    void
    set_type (void)
    {
      set_type (ir_type_v<T>);
    }

    ir_def_id
    create_def_id (void) noexcept;

    std::size_t
    get_num_defs (void) const noexcept;

   private:
    nonnull_cptr<ir_component> m_component;
    const ir_variable_id       m_id;
    const std::string          m_name        = anonymous_name;
    ir_type                    m_type        = ir_type_v<void>;
    ir_def_id                  m_curr_def_id;
  };

  ir_type
  common_type (const ir_variable& lhs, const ir_variable& rhs);

  template <typename ...Vars,
            std::enable_if_t<std::conjunction_v<std::is_same<ir_variable, Vars>...>> * = nullptr>
  inline
  ir_type
  common_type (const Vars&... vars)
  {
    return (vars.get_type () ^ ...);
  }

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_VARIABLE_HPP

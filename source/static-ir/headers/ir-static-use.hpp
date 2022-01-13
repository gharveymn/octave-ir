/** ir-static-use.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_STATIC_IR_IR_STATIC_USE_HPP
#define OCTAVE_IR_STATIC_IR_IR_STATIC_USE_HPP

#include "ir-static-def.hpp"
#include "ir-type.hpp"

#include "ir-common.hpp"

#include <gch/nonnull_ptr.hpp>

#include <iosfwd>
#include <string_view>

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

    ir_static_use (const ir_static_variable& var, ir_static_def_id id);

    [[nodiscard]]
    const ir_static_variable&
    get_variable (void) const noexcept;

    [[nodiscard]]
    ir_static_def_id
    get_def_id (void) const noexcept;

    [[nodiscard]]
    std::string_view
    get_variable_name (void) const;

    [[nodiscard]]
    ir_type
    get_type (void) const noexcept;

  private:
    nonnull_cptr<ir_static_variable> m_variable;
    ir_static_def_id                 m_id;
  };

  std::ostream&
  operator<< (std::ostream& out, const ir_static_use& use);

}

#endif // OCTAVE_IR_STATIC_IR_IR_STATIC_USE_HPP

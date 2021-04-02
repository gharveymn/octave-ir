/** ir-static-def.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_STATIC_DEF_HPP
#define OCTAVE_IR_IR_STATIC_DEF_HPP

#include "gch/octave-ir-utilities/ir-utility.hpp"
#include "gch/octave-ir-static-ir/ir-type.hpp"

#include <gch/nonnull_ptr.hpp>

#include <iosfwd>
#include <string_view>

namespace gch
{

  class ir_static_variable;

  class ir_static_def_id
    : public named_type<std::size_t>
  {
  public:
    using named_type<std::size_t>::named_type;

    ir_static_def_id&
    operator++ (void) noexcept
    {
      return *this = ir_static_def_id { static_cast<value_type> (*this) + 1 };
    }

    ir_static_def_id
    operator++ (int) noexcept
    {
      ir_static_def_id ret { *this };
      ++(*this);
      return ret;
    }
  };

  inline constexpr ir_static_def_id ir_undefined_def_id { static_cast<std::size_t> (-1) };

  class ir_static_def
  {
  public:
    ir_static_def            (void)                     = delete;
    ir_static_def            (const ir_static_def&)     = default;
    ir_static_def            (ir_static_def&&) noexcept = default;
    ir_static_def& operator= (const ir_static_def&)     = default;
    ir_static_def& operator= (ir_static_def&&) noexcept = default;
    ~ir_static_def           (void)                     = default;

    ir_static_def (const ir_static_variable& var, ir_static_def_id id);

    [[nodiscard]]
    const ir_static_variable&
    get_variable (void) const noexcept;

    [[nodiscard]]
    ir_static_def_id
    get_id (void) const noexcept;

    [[nodiscard]]
    std::string_view
    get_variable_name (void) const noexcept;

    [[nodiscard]]
    ir_type
    get_type (void) const noexcept;

  private:
    nonnull_cptr<ir_static_variable> m_variable;
    ir_static_def_id                 m_id;
  };

  bool
  operator== (const ir_static_def& lhs, const ir_static_def& rhs) noexcept;

  bool
  operator!= (const ir_static_def& lhs, const ir_static_def& rhs) noexcept;

  std::ostream&
  operator<< (std::ostream& out, const ir_static_def& def);

}

#endif // OCTAVE_IR_IR_STATIC_DEF_HPP

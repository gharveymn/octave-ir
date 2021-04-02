/** ir-static-operand.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_STATIC_OPERAND_HPP
#define OCTAVE_IR_IR_STATIC_OPERAND_HPP

#include "gch/octave-ir-static-ir/ir-static-use.hpp"
#include "gch/octave-ir-static-ir/ir-constant.hpp"

#include <iosfwd>

namespace gch
{

  class ir_static_operand
  {
  public:
    ir_static_operand            (void)                         = delete;
    ir_static_operand            (const ir_static_operand&);
    ir_static_operand            (ir_static_operand&&) noexcept;
    ir_static_operand& operator= (const ir_static_operand&);
    ir_static_operand& operator= (ir_static_operand&&) noexcept;
    ~ir_static_operand           (void);

    GCH_IMPLICIT_CONVERSION
    ir_static_operand (const ir_constant& c);

    GCH_IMPLICIT_CONVERSION
    ir_static_operand (ir_constant&& c) noexcept;

    GCH_IMPLICIT_CONVERSION
    ir_static_operand (ir_static_use use) noexcept;

    GCH_IMPLICIT_CONVERSION
    ir_static_operand (const ir_static_variable& var, ir_static_def_id id) noexcept;

    [[nodiscard]]
    bool
    is_constant (void) const noexcept;

    [[nodiscard]]
    bool
    is_use (void) const noexcept;

    [[nodiscard]]
    ir_type
    get_type (void) const noexcept;

    friend
    const ir_constant&
    get_constant (const ir_static_operand& op) noexcept;

    friend
    const ir_static_use&
    get_use (const ir_static_operand& op) noexcept;

    friend
    optional_ref<const ir_constant>
    maybe_get_constant (const ir_static_operand& op) noexcept;

    friend
    optional_ref<const ir_static_use>
    maybe_get_use (const ir_static_operand& op) noexcept;

  private:
    static_assert (std::is_trivially_destructible_v<ir_static_use>,
                   "ir_static_use should be trivially destructible");

    // not using std::variant for exceptions, space and efficiency reasons
    union
    {
      ir_constant   m_constant;
      ir_static_use m_use;
    };

    enum class tag
    {
      constant,
      use,
    } m_tag;
  };

  [[nodiscard]]
  const ir_constant&
  get_constant (const ir_static_operand& op) noexcept;

  [[nodiscard]]
  const ir_static_use&
  get_use (const ir_static_operand& op) noexcept;

  [[nodiscard]]
  optional_cref<ir_constant>
  maybe_get_constant (const ir_static_operand& op) noexcept;

  [[nodiscard]]
  optional_cref<ir_static_use>
  maybe_get_use (const ir_static_operand& op) noexcept;

  [[nodiscard]]
  ir_type
  get_type (const ir_static_operand& op) noexcept;

  template <typename Visitor>
  decltype (auto)
  visit (Visitor&& vis, const ir_static_operand& op)
  {
    if (optional_ref constant { maybe_get_constant (op) })
      return std::invoke (std::forward<Visitor> (vis), *constant);
    return std::invoke (std::forward<Visitor> (vis), get_use (op));
  }

  template <typename Result, typename Visitor>
  std::enable_if_t<std::is_same_v<void, std::remove_cv_t<Result>>, Result>
  visit (Visitor&& vis, const ir_static_operand& op)
  {
    if (optional_ref constant { maybe_get_constant (op) })
      std::invoke (std::forward<Visitor> (vis), *constant);
    std::invoke (std::forward<Visitor> (vis), get_use (op));
  }

  template <typename Result, typename Visitor>
  std::enable_if_t<! std::is_same_v<void, std::remove_cv_t<Result>>, Result>
  visit (Visitor&& vis, const ir_static_operand& op)
  {
    if (optional_ref constant { maybe_get_constant (op) })
      return std::invoke (std::forward<Visitor> (vis), *constant);
    return std::invoke (std::forward<Visitor> (vis), get_use (op));
  }

  std::ostream&
  operator<< (std::ostream& out, const ir_static_operand& operand);

}

#endif // OCTAVE_IR_IR_STATIC_OPERAND_HPP
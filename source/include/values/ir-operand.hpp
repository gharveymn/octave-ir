/** ir-operand.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_OPERAND_HPP
#define OCTAVE_IR_IR_OPERAND_HPP

#include "values/ir-constant.hpp"
#include "values/ir-use.hpp"

#include <gch/optional_ref.hpp>

#include <variant>

namespace gch
{

  class ir_operand_in
  {
  public:
    ir_operand_in (void)                                = delete;
    ir_operand_in (const ir_operand_in&)                = default;
    ir_operand_in (ir_operand_in&&) noexcept            = default;
    ir_operand_in& operator= (const ir_operand_in&)     = default;
    ir_operand_in& operator= (ir_operand_in&&) noexcept = default;
    ~ir_operand_in (void)                               = default;

    ir_operand_in (const ir_use_info& info)
      : m_data (std::in_place_type<ir_use_info>, info)
    { }

    ir_operand_in (ir_constant&& c)
      : m_data (std::in_place_type<ir_constant>, std::move (c))
    { }

    [[nodiscard]] constexpr
    std::size_t
    index (void) const noexcept
    {
      return m_data.index ();
    }

    template <typename T>
    friend constexpr
    optional_ref<T>
    maybe_get (ir_operand_in&);

    template <typename T>
    friend constexpr
    optional_cref<T>
    maybe_get (const ir_operand_in&);

    template <typename T, typename U>
    friend constexpr
    std::enable_if_t<std::is_same_v<std::decay_t<U>, ir_operand_in>, match_cvref_t<U, T>>
    get (U&&);

  private:
    std::variant<ir_constant, ir_use_info> m_data;
  };

  template <typename T>
  [[nodiscard]] constexpr
  optional_ref<T>
  maybe_get (ir_operand_in& in)
  {
    return std::get_if<T> (&in.m_data);
  }

  template <typename T>
  [[nodiscard]] constexpr
  optional_cref<T>
  maybe_get (const ir_operand_in& in)
  {
    return std::get_if<T> (&in.m_data);
  }

  template <typename T, typename U>
  [[nodiscard]] constexpr
  std::enable_if_t<std::is_same_v<std::decay_t<U>, ir_operand_in>, match_cvref_t<U, T>>
  get (U&& in)
  {
    return static_cast<match_cvref_t<U, T>> (*std::get_if<T> (&in.m_data));
  }

  class ir_operand
  {
  public:
    ir_operand            (void)                  = delete;
    ir_operand            (const ir_operand&)     = delete;
    ir_operand            (ir_operand&&) noexcept = default;
    ir_operand& operator= (const ir_operand&)     = delete;
    ir_operand& operator= (ir_operand&&) noexcept = default;
    ~ir_operand           (void)                  = default;

    ir_operand (ir_instruction& instr, ir_operand_in&& in)
    {
      if (optional_ref<ir_constant> c = maybe_get<ir_constant> (in))
        m_data.emplace<ir_constant> (std::move (*c));
      else
        m_data.emplace<ir_use> (instr, get<ir_use_info> (std::move (in)));
    }

    ir_operand (ir_instruction&, ir_constant&& c)
      : m_data (std::in_place_type<ir_constant>, std::move (c))
    { }

    template <typename T>
    friend constexpr
    optional_ref<T>
    maybe_get (ir_operand&);

    template <typename T>
    friend constexpr
    optional_cref<T>
    maybe_get (const ir_operand&);

    template <typename T, typename U>
    friend constexpr
    std::enable_if_t<std::is_same_v<std::decay_t<U>, ir_operand>, match_cvref_t<U, T>>
    get (U&&);

    [[nodiscard]] constexpr
    ir_type
    get_type (void) const noexcept
    {
      if (auto u = maybe_get<ir_use> (*this))
        return u->get_type ();
      return std::visit ([] (auto&& x) noexcept -> ir_type { return x.get_type (); }, m_data);
    }

    [[nodiscard]] constexpr
    bool
    is_constant (void) const noexcept
    {
      return std::holds_alternative<ir_constant> (m_data);
    }

    [[nodiscard]] constexpr
    bool
    is_use (void) const noexcept
    {
      return std::holds_alternative<ir_use> (m_data);
    }

  private:
    std::variant<ir_constant, ir_use> m_data;
  };

  template <typename T>
  [[nodiscard]] constexpr
  optional_ref<T>
  maybe_get (ir_operand& op)
  {
    return std::get_if<T> (&op.m_data);
  }

  template <typename T>
  [[nodiscard]] constexpr
  optional_cref<T>
  maybe_get (const ir_operand& op)
  {
    return std::get_if<T> (&op.m_data);
  }

  template <typename T, typename U>
  [[nodiscard]] constexpr
  std::enable_if_t<std::is_same_v<std::decay_t<U>, ir_operand>, match_cvref_t<U, T>>
  get (U&& op)
  {
    return static_cast<match_cvref_t<U, T>> (*std::get_if<T> (&op.m_data));
  }

  [[nodiscard]] constexpr
  ir_type
  get_type (const ir_operand& op) noexcept
  {
    if (optional_cref<ir_use> u = maybe_get<ir_use> (op))
      return u->get_type ();
    return get<ir_constant> (op).get_type ();
  }

}

#endif // OCTAVE_IR_IR_OPERAND_HPP

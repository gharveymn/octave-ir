/** ir-constant.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_CONSTANT_HPP
#define OCTAVE_IR_IR_CONSTANT_HPP

#include "utilities/ir-common.hpp"
#include "utilities/ir-type-traits.hpp"
#include "values/types/ir-type-std.hpp"

#include <gch/optional_ref.hpp>

#include <any>

namespace gch
{

  class ir_constant
  {
  public:
    ir_constant            (void)                   = default;
    ir_constant            (const ir_constant&)     = default;
    ir_constant            (ir_constant&&) noexcept = default;
    ir_constant& operator= (const ir_constant&)     = default;
    ir_constant& operator= (ir_constant&&) noexcept = default;
    ~ir_constant           (void)                   = default;

    template <typename T,
              std::enable_if_t<! std::is_same_v<std::decay_t<T>, ir_constant>> * = nullptr>
    explicit
    ir_constant (T&& t)
      : m_type (ir_type_v<std::decay_t<T>>),
        m_data (std::forward<T> (t))
    { }

    template <typename ...Args>
    explicit
    ir_constant (ir_type type, Args&&... args)
      : m_type (type),
        m_data (std::forward<Args> (args)...)
    { }

    [[nodiscard]] constexpr
    ir_type
    get_type (void) const noexcept
    {
      return m_type;
    }

    template <typename T, typename ...Args>
    constexpr
    decltype (auto)
    emplace (Args&&... args)
    {
      m_type = ir_type_v<T>;
      return m_data.emplace<T> (std::forward<Args> (args)...);
    }

    template <typename T>
    friend constexpr
    optional_ref<T>
    maybe_as_type (ir_constant& c) noexcept;

    template <typename T>
    friend constexpr
    optional_ref<const T>
    maybe_as_type (const ir_constant& c) noexcept;

    template <typename T, typename U>
    friend constexpr
    std::enable_if_t<std::is_same_v<std::decay_t<U>, ir_constant>, match_cvref_t<U, T>>
    cast (U&&);

  private:
    ir_type  m_type = ir_type_v<void>;
    std::any m_data;
  };

  template <typename T>
  [[nodiscard]] constexpr
  optional_ref<T>
  maybe_as_type (ir_constant& c) noexcept
  {
    return std::any_cast<T> (&c.m_data);
  }

  template <typename T>
  [[nodiscard]] constexpr
  optional_ref<const T>
  maybe_as_type (const ir_constant& c) noexcept
  {
    return std::any_cast<T> (&c.m_data);
  }

  template <typename T, typename U>
  [[nodiscard]] constexpr
  std::enable_if_t<std::is_same_v<std::decay_t<U>, ir_constant>, match_cvref_t<U, T>>
  cast (U&& c)
  {
    return static_cast<match_cvref_t<U, T>> (*std::any_cast<T> (&c.m_data));
  }

}

#endif

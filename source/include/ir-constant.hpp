/*

Copyright (C) 2019 Gene Harvey

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if ! defined (octave_ir_constant_h)
#define octave_ir_constant_h 1

#include "ir-common.hpp"
#include "ir-type-std.hpp"

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
    decltype (auto)
    emplace (Args&&... args)
    {
      m_type = ir_type_v<T>;
      return m_data.emplace<T> (std::forward<Args> (args)...);
    }

    template <typename T>
    friend
    optional_ref<T>
    maybe_cast (ir_constant& c) noexcept;

    template <typename T>
    friend
    optional_ref<const T>
    maybe_cast (const ir_constant& c) noexcept;

    template <typename T, typename U>
    friend constexpr
    std::enable_if_t<std::is_same_v<std::decay_t<U>, ir_constant>, match_cvref_t<U, T>>
    cast (U&&);

  private:
    ir_type  m_type = ir_type_v<void>;
    std::any m_data;
  };

  template <typename T>
  [[nodiscard]] optional_ref<T>
  maybe_cast (ir_constant& c) noexcept
  {
    return std::any_cast<T> (&c.m_data);
  }

  template <typename T>
  [[nodiscard]] optional_ref<const T>
  maybe_cast (const ir_constant& c) noexcept
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

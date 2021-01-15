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
  
  class ir_constant;
  
  template <typename T>
  optional_ref<T> data_cast(ir_constant& c) noexcept;
  
  template <typename T>
  optional_ref<const T> data_cast(const ir_constant& c) noexcept;
  
  class ir_constant
  {
  public:
    ir_constant            (void)                   = default;
    ir_constant            (const ir_constant&)     = default;
    ir_constant            (ir_constant&&) noexcept = default;
    ir_constant& operator= (const ir_constant&)     = default;
    ir_constant& operator= (ir_constant&&) noexcept = default;
    ~ir_constant           (void)                   = default;
  
    template <typename ...Args>
    constexpr explicit ir_constant (Args&&... args)
      : m_type (ir_type_v<std::decay_t<Args>...>),
        m_data (std::forward<Args> (args)...)
    { }
    
    template <typename ...Args>
    constexpr explicit ir_constant (ir_type type, Args&&... args)
      : m_type (type),
        m_data (std::forward<Args> (args)...)
    { }
    
    [[nodiscard]] constexpr ir_type get_type (void) const noexcept { return m_type; }
    
    template <typename T>
    [[nodiscard]] constexpr const T& get_data (void) const { std::any_cast<T> (m_data); }
    
    template <typename T, typename ...Args>
    auto emplace (Args&&... args)
    {
      m_type = ir_type_v<T>;
      return m_data.emplace<T> (std::forward<Args> (args)...);
    }
    
    template <typename T>
    friend optional_ref<T> data_cast (ir_constant& c) noexcept
    {
      return std::any_cast<T> (&c.m_data);
    }
    
    template <typename T>
    friend optional_ref<const T> data_cast (const ir_constant& c) noexcept
    {
      return std::any_cast<T> (&c.m_data);
    }
  
  private:
    ir_type  m_type = ir_type_v<void>;
    std::any m_data;
  };
  
  class ir_operand;
  
  template <typename T>
  constexpr optional_ref<T> get_if (ir_operand& op) noexcept;
  
  template <typename T>
  constexpr optional_ref<const T> get_if (const ir_operand& op) noexcept;

}

#endif

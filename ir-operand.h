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

#if ! defined (octave_ir_operand_h)
#define octave_ir_operand_h 1

#include "octave-config.h"

#include "ir-common.h"
#include "ir-type-std.h"
#include <unordered_set>
#include <string>

namespace octave
{

  template <typename>
  struct ir_printer;

  class ir_operand;

  template <typename...>
  class ir_constant;

  template <typename T>
  class ir_constant<T>;

  class ir_def;
  class ir_use;

  //! An abstract base class for ir_instruction operands.
  class ir_operand
  {
  public:

    constexpr ir_operand (void) noexcept               = default;

    constexpr ir_operand (const ir_operand&) noexcept  = delete;
    constexpr ir_operand (ir_operand&&) noexcept       = default;

    ir_operand& operator= (const ir_operand&) noexcept = delete;
    ir_operand& operator= (ir_operand&&) noexcept      = default;

    virtual ~ir_operand (void) noexcept = 0;

    virtual ir_type get_type (void) const = 0;

    friend struct ir_printer<ir_operand>;

  protected:

  private:

  };

  template <typename ...Ts>
  class ir_constant : public ir_operand
  {
  public:

    using value_type = std::tuple<Ts...>;

    ir_constant (void) = delete;

    ir_constant (Ts... vals) noexcept
      : m_value { std::move (vals)... }
    { }

    ir_constant (const ir_constant&) = default; // change this to default once we really need it
    ir_constant (ir_constant&& o) noexcept
      : m_value (std::move (o.m_value))
    { }

    ir_constant& operator= (const ir_constant&) = default;
    ir_constant& operator= (ir_constant&& o) noexcept
    {
      m_value = std::move (o.m_value);
      return *this;
    }

    ~ir_constant (void) noexcept override = default;

    template <std::size_t I>
    typename std::tuple_element<I, value_type>::type&
    get(void) noexcept
    {
      return std::get<I> (m_value);
    }

    template <std::size_t I>
    constexpr const typename std::tuple_element<I, value_type>::type&
    get(void) const noexcept
    {
      return std::get<I> (m_value);
    }

    template <typename T>
    T&
    get (void) noexcept
    {
      return std::get<T> (m_value);
    }

    template <typename T>
    constexpr const T&
    get (void) const noexcept
    {
      return std::get<T> (m_value);
    }

    value_type& get (void) noexcept { return m_value; }

    constexpr const value_type&
    get (void) const noexcept { return m_value; }

    ir_type get_type (void) const override;

  protected:

    std::ostream& print (std::ostream& os) const;

  private:

    value_type m_value;

  };

  template <typename T>
  class ir_constant<T> : public ir_operand
  {
  public:

    using value_type = T;

    ir_constant (void) = delete;

    ir_constant (value_type val) noexcept
      : m_value {std::move (val)}
    { }

    ir_constant (const ir_constant&) = default;

    ir_constant (ir_constant&& o) noexcept
      : m_value (std::move (o.m_value))
    { }

    ir_constant& operator= (const ir_constant&) = default;
    ir_constant& operator= (ir_constant&& o) noexcept
    {
      m_value = std::move (o.m_value);
      return *this;
    }

    ~ir_constant (void) noexcept override = default;

    value_type& value (void) noexcept { return m_value; }

    constexpr const value_type& value (void) const noexcept { return m_value; }

    ir_type get_type (void) const override;

  protected:

    std::ostream& print (std::ostream& os) const;

  private:

    value_type m_value;

  };

  template <typename T>
  class ir_constant<T&> : public ir_operand
  {
  public:

    using value_type = T&;

    ir_constant (void) = delete;

    ir_constant (value_type val) noexcept
      : m_value {val}
    { }

    ir_constant (const ir_constant&) = default;

    ir_constant (ir_constant&& o) noexcept
      : m_value (o.m_value)
    { }

    ir_constant& operator= (const ir_constant&) = default;
    ir_constant& operator= (ir_constant&& o) noexcept = default;

    ~ir_constant (void) noexcept override = default;

    constexpr value_type& value (void) const noexcept { return m_value; }

    ir_type get_type (void) const override;

  protected:

  private:

    value_type m_value;

  };

  template <>
  struct ir_type::instance<ir_operand>
  {
    using type = ir_operand;
    static constexpr
    impl m_impl = create_type<type> ("ir_operand");
  };

}

#endif

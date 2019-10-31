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

    constexpr explicit ir_operand (ir_type type) noexcept
      : m_type (type)
    { }

    virtual ~ir_operand (void) = 0;

    constexpr const ir_type& get_type (void) const { return m_type; }

    void replace_type (ir_type new_type) { m_type = new_type; }

    friend struct ir_printer<ir_operand>;

  protected:

    virtual std::ostream& print (std::ostream& os) const;

  private:

    ir_type m_type = ir_type::get<void> ();

  };

  template <typename ...Ts>
  class ir_constant : public ir_operand
  {
  public:

    using value_type = const std::tuple<Ts...>;

    ir_constant (void) = delete;

    ir_constant (Ts... args)  noexcept;
    constexpr ir_constant (const ir_constant&) noexcept  = delete;
    constexpr ir_constant (ir_constant&&) noexcept       = default;

    ir_constant& operator= (const ir_constant&) noexcept = delete;
    ir_constant& operator= (ir_constant&&) noexcept      = default;

    template <std::size_t I>
    constexpr const typename std::tuple_element<I, value_type>::type&
    get(void) const noexcept
    {
      return std::get<I> (m_value);
    }

    template <typename T>
    constexpr const T&
    get (void) const noexcept
    {
      return std::get<T> (m_value);
    }

    constexpr value_type&
    get (void) const noexcept { return m_value; }

  protected:

    std::ostream& print (std::ostream& os) const override;

  private:

    value_type m_value;
  };

  template <typename T>
  class ir_constant<T> : public ir_operand
  {
  public:

    using value_type = typename std::add_const<T>::type;

    ir_constant (void) = delete;

    ir_constant (value_type avalue) noexcept;

    ir_constant (const ir_constant&)  = delete;
    ir_constant (ir_constant&&)       = default;

    ir_constant& operator= (const ir_constant&) = delete;
    ir_constant& operator= (ir_constant&&)      = default;

    ~ir_constant (void) override = default;

    constexpr value_type& value (void) const noexcept { return m_value; }

  protected:

    std::ostream& print (std::ostream& os) const override;

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

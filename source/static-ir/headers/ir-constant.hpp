/** ir-constant.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_STATIC_IR_IR_CONSTANT_HPP
#define OCTAVE_IR_STATIC_IR_IR_CONSTANT_HPP

#include "ir-type.hpp"

#include "ir-common.hpp"
#include "ir-utility.hpp"
#include "ir-type-traits.hpp"

#include <gch/optional_ref.hpp>

#include <cassert>
#include <iosfwd>
#include <variant>

namespace gch
{

  class ir_constant
  {
  public:
    template <typename T>
    using make_const_pointee_t = std::remove_const_t<make_all_levels_const_t<T>>;

    using variant_pack = pack_for_each_t<pack_remove_t<ir_type_pack, void>, make_const_pointee_t>;
    using variant_type = pack_apply_t<variant_pack, std::variant>;

    ir_constant            (void)                   = default;
    ir_constant            (const ir_constant&)     = default;
    ir_constant            (ir_constant&&) noexcept = default;
    ir_constant& operator= (const ir_constant&)     = default;
    ir_constant& operator= (ir_constant&&) noexcept = default;
    ~ir_constant           (void)                   = default;

    template <typename T, typename ...Args,
              std::enable_if_t<is_ir_type_v<std::decay_t<T>>> * = nullptr>
    constexpr explicit
    ir_constant (std::in_place_type_t<T>, Args&&... args)
      : m_type (ir_type_v<std::decay_t<T>>),
        m_data (std::in_place_type<make_const_pointee_t<std::decay_t<T>>>,
                std::forward<Args> (args)...)
    { }

    template <typename T, std::enable_if_t<is_ir_type_v<std::decay_t<T>>> * = nullptr>
    constexpr explicit
    ir_constant (T&& t)
      : ir_constant (std::in_place_type<T>, std::forward<T&&> (t))
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
      m_type = ir_type_v<std::decay_t<T>>;
      return m_data.emplace<make_const_pointee_t<std::decay_t<T>>> (std::forward<Args> (args)...);
    }

    template <typename T>
    friend constexpr
    optional_ref<make_const_pointee_t<T>>
    maybe_as (ir_constant& c) noexcept;

    template <typename T>
    friend constexpr
    ir_constant::make_const_pointee_t<T>&
    as (ir_constant& c);

  private:
    ir_type      m_type = ir_type_v<void>;
    variant_type m_data;
  };

  template <typename T>
  [[nodiscard]] constexpr
  bool
  is_a (const ir_constant& c) noexcept
  {
    return ir_type_v<T> == c.get_type ();
  }

  template <typename T>
  [[nodiscard]] constexpr
  optional_ref<ir_constant::make_const_pointee_t<T>>
  maybe_as (ir_constant& c) noexcept
  {
    return std::get_if<ir_constant::make_const_pointee_t<T>> (&c.m_data);
  }

  template <typename T>
  [[nodiscard]] constexpr
  optional_cref<ir_constant::make_const_pointee_t<T>>
  maybe_as (const ir_constant& c) noexcept
  {
    return maybe_as<T> (as_mutable (c));
  }

  template <typename T>
  [[nodiscard]] constexpr
  ir_constant::make_const_pointee_t<T>&
  as (ir_constant& c)
  {
    assert (ir_type_v<T> == c.get_type ());
    return *maybe_as<T> (c);
  }

  template <typename T>
  [[nodiscard]] constexpr
  const ir_constant::make_const_pointee_t<T>&
  as (const ir_constant& c)
  {
    return as<T> (as_mutable (c));
  }

  std::ostream&
  operator<< (std::ostream& out, const ir_constant& c);

}

#endif

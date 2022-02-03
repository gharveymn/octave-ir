/** ir-static-operand.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-static-operand.hpp"

#include "ir-common.hpp"

#include <cassert>

namespace gch
{

  ir_static_operand::
  ir_static_operand (const ir_static_operand& other)
    : m_tag (other.m_tag)
  {
    if (other.is_constant ())
      ::new (&m_constant) ir_constant (other.m_constant);
    else
      m_use = other.m_use;
  }

  ir_static_operand::
  ir_static_operand (ir_static_operand&& other) noexcept
    : m_tag (other.m_tag)
  {
    if (other.is_constant ())
      ::new (&m_constant) ir_constant (std::move (other.m_constant));
    else
      m_use = other.m_use;
  }

  ir_static_operand&
  ir_static_operand::
  operator= (const ir_static_operand& other)
  {
    if (&other == this)
      return *this;

    if (other.is_constant ())
    {
      if (is_constant ())
        m_constant = other.m_constant;
      else
        ::new (&m_constant) ir_constant (other.m_constant);
    }
    else
    {
      if (is_constant ())
        m_constant.~ir_constant ();
      m_use = other.m_use;
    }

    m_tag = other.m_tag;

    return *this;
  }

  ir_static_operand&
  ir_static_operand::
  operator= (ir_static_operand&& other) noexcept
  {
    if (other.is_constant ())
    {
      if (is_constant ())
        m_constant = std::move (other.m_constant);
      else
        ::new (&m_constant) ir_constant (std::move (other.m_constant));
    }
    else
    {
      if (is_constant ())
        m_constant.~ir_constant ();
      m_use = other.m_use;
    }

    m_tag = other.m_tag;

    return *this;
  }

  ir_static_operand::
  ~ir_static_operand (void)
  {
    if (is_constant ())
      m_constant.~ir_constant ();
  }

  ir_static_operand::
  ir_static_operand (const ir_constant& c)
    : m_constant (c),
      m_tag      (tag::constant)
  { }

  ir_static_operand::
  ir_static_operand (ir_constant&& c) noexcept
    : m_constant (std::move (c)),
      m_tag      (tag::constant)
  { }

  ir_static_operand::
  ir_static_operand (ir_static_use u) noexcept
    : m_use (u),
      m_tag (tag::use)
  { }

  ir_static_operand::
  ir_static_operand (ir_variable_id var_id, std::optional<ir_def_id> id) noexcept
    : m_use (var_id, id),
      m_tag (tag::use)
  { }

  bool
  ir_static_operand::
  is_constant (void) const noexcept
  {
    return m_tag == tag::constant;
  }

  bool
  ir_static_operand::
  is_use (void) const noexcept
  {
    return m_tag == tag::use;
  }

  const ir_constant&
  ir_static_operand::
  as_constant (void) const noexcept
  {
    assert (is_constant ());
    return m_constant;
  }

  ir_static_use
  ir_static_operand::
  as_use (void) const noexcept
  {
    assert (is_use ());
    return m_use;
  }

  optional_cref<ir_constant>
  ir_static_operand::
  maybe_as_constant (void) const noexcept
  {
    if (is_constant ())
      return optional_ref { as_constant () };
    return nullopt;
  }

  std::optional<ir_static_use>
  ir_static_operand::
  maybe_as_use (void) const noexcept
  {
    if (is_use ())
      return { as_use () };
    return std::nullopt;
  }

  bool
  is_constant (const ir_static_operand& op) noexcept
  {
    return op.is_constant ();
  }

  bool
  is_use (const ir_static_operand& op) noexcept
  {
    return op.is_use ();
  }

  const ir_constant&
  as_constant (const ir_static_operand& op) noexcept
  {
    return op.as_constant ();
  }

  ir_static_use
  as_use (const ir_static_operand& op) noexcept
  {
    return op.as_use ();
  }

  optional_cref<ir_constant>
  maybe_as_constant (const ir_static_operand& op) noexcept
  {
    return op.maybe_as_constant ();
  }

  std::optional<ir_static_use>
  maybe_as_use (const ir_static_operand& op) noexcept
  {
    return op.maybe_as_use ();
  }

}

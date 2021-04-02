/** ir-static-operand.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-ir-static-ir/ir-static-operand.hpp"

#include "gch/octave-ir-utilities/ir-common.hpp"

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

  ir_type
  ir_static_operand::
  get_type (void) const noexcept
  {
    if (is_constant ())
      return m_constant.get_type ();
    return m_use.get_type ();
  }

  const ir_constant&
  get_constant (const ir_static_operand& op) noexcept
  {
    assert (op.is_constant ());
    return op.m_constant;
  }

  const ir_static_use&
  get_use (const ir_static_operand& op) noexcept
  {
    assert (op.is_use ());
    return op.m_use;
  }

  optional_cref<ir_constant>
  maybe_get_constant (const ir_static_operand& op) noexcept
  {
    if (op.is_constant ())
      return optional_ref { get_constant (op) };
    return nullopt;
  }

  optional_cref<ir_static_use>
  maybe_get_use (const ir_static_operand& op) noexcept
  {
    if (op.is_use ())
      return optional_ref { get_use (op) };
    return nullopt;
  }

  ir_type
  get_type (const ir_static_operand& op) noexcept
  {
    return op.get_type ();
  }

  std::ostream&
  operator<< (std::ostream& out, const ir_static_operand& operand)
  {
    return visit ([&](const auto& x) { return std::ref (out << x); }, operand);
  }

}

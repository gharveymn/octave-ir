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

#if ! defined (ir_common_util_h)
#define ir_common_util_h 1

#include "ir-common.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>

#include <iosfwd>
#include <memory>
#include <functional>
#include <unordered_set>
#include <type_traits>
#include <optional>

namespace gch
{

  template <typename T, typename U>
  inline
  bool
  is_a (U *x)
  {
    return dynamic_cast<const T *> (x) != nullptr;
  }

  template <typename T, typename U>
  inline
  bool
  is_a (U& x)
  {
    return is_a<T> (&x);
  }

  template <typename T>
  constexpr
  std::remove_const_t<T>&
  as_mutable (T& ref)
  {
    return const_cast<std::remove_const_t<T>&> (ref);
  }

  template <typename T>
  void
  as_mutable (const T&&) = delete;

  template <typename T>
  class mover
  {
  public:
    mover            (void)             = delete;
    mover            (const mover&)     = default;
    mover            (mover&&) noexcept = default;
    mover& operator= (const mover&)     = default;
    mover& operator= (mover&&) noexcept = default;
    ~mover           (void)             = default;

    constexpr explicit
    mover (T& ref)
      : m_ptr (ref)
    { }

    constexpr /* implicit */
    mover (T&& ref)
      : m_ptr (ref)
    { }

    constexpr /* implicit */
    operator T&& (void) const
    {
      return std::move (*m_ptr);
    }

  private:
    nonnull_ptr<T> m_ptr;
  };

  template <typename T>
  class value_mover
  {
  public:
    value_mover            (void)                   = delete;
    value_mover            (const value_mover&)     = delete;
    value_mover            (value_mover&&) noexcept = default;
    value_mover& operator= (const value_mover&)     = delete;
    value_mover& operator= (value_mover&&) noexcept = default;
    ~value_mover           (void)                   = default;

    constexpr /* implicit */
    value_mover (T&& ref)
      : m_value (std::move (ref))
    { }

    constexpr /* implicit */
    operator T&& (void)
    {
      return std::move (m_value);
    }

  private:
    T m_value;
  };

  template <typename T>
  struct init_counter
  {
    init_counter (void) noexcept
    {
      reset_nums ();
    }

    init_counter (const init_counter&) noexcept
    {
      ++num_copies;
    }

    init_counter (init_counter&&) noexcept
    {
      ++num_moves;
    }

    init_counter&
    operator= (const init_counter&) noexcept
    {
      ++num_copy_assigns;
      return *this;
    }

    init_counter&
    operator= (init_counter&&) noexcept
    {
      ++num_move_assigns;
      return *this;
    }

    static
    void
    reset_nums (void)
    {
      num_copies       = 0;
      num_moves        = 0;
      num_copy_assigns = 0;
      num_move_assigns = 0;
    }

    static inline std::size_t num_copies;
    static inline std::size_t num_moves;
    static inline std::size_t num_copy_assigns;
    static inline std::size_t num_move_assigns;
  };

  template <typename T>
  class named_type
  {
  public:
    using value_type = T;

    named_type            (void)                  = default;
    named_type            (const named_type&)     = default;
    named_type            (named_type&&) noexcept = default;
    named_type& operator= (const named_type&)     = default;
    named_type& operator= (named_type&&) noexcept = default;
    ~named_type           (void)                  = default;

    template <typename U, typename ...Args,
              std::enable_if_t<! std::is_same_v<named_type, std::decay_t<U>>
                             &&  std::is_constructible_v<value_type, U, Args...>> * = nullptr>
    explicit
    named_type (U&& u, Args&&... args)
          noexcept (std::is_nothrow_constructible_v<value_type, U, Args...>)
      : m_value (std::forward<U> (u), std::forward<Args> (args)...)
    { }

    operator const value_type& (void) const noexcept
    {
      return m_value;
    }

    operator value_type&& (void) && noexcept
    {
      return std::move (m_value);
    }

  private:
    value_type m_value;
  };

  template <typename T, typename ParameterTag>
  class named_parameter
    : public named_type<T>
  {
  public:
    using tag = ParameterTag;

    using named_type<T>::named_type;
  };

}

#endif

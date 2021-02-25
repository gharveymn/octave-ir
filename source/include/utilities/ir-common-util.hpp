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

}

#endif

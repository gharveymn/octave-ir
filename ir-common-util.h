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

#include "octave-config.h"

#include "ir-common.h"

#include <iosfwd>
#include <memory>

namespace octave
{
  class ir_type;

  template <typename T>
  struct ir_printer
  {
    using ir_class = T;

    static std::ostream& short_print (std::ostream& os, const ir_class&);

    static std::ostream& long_print (std::ostream& os, const ir_class&);

  };

  std::ostream& operator<< (std::ostream& os, const ir_type& ty);

  template <typename T, typename S>
  constexpr bool isa (S* x)
  {
    return dynamic_cast<T*> (x) != nullptr;
  }

  template <typename T, typename S>
  constexpr bool isa (S& x)
  {
    return dynamic_cast<T*> (&x) != nullptr;
  }

  template <class T>
  struct unique_if
  {
    using singular_type = std::unique_ptr<T>;
  };

  template <class T>
  struct unique_if<T[]>
  {
    using array_type = std::unique_ptr<T[]>;
  };

  template <class T, size_t N>
  struct unique_if<T[N]>
  {
    struct invalid_type
    { };
  };

  template <class T, class... Args>
  typename unique_if<T>::singular_type
  make_unique(Args&&... args)
  {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }

  template <class T>
  typename unique_if<T>::array_type
  make_unique(size_t n)
  {
    using base_type = typename std::remove_extent<T>::type;
    return std::unique_ptr<T>(new base_type[n]());
  }

  template<typename T, typename... Args>
  typename unique_if<T>::invalid_type
  make_unique(Args&&...) = delete;

  template <typename T>
  struct ptr_noexcept_hash
  {
    using value_type = T *;
    constexpr std::size_t operator() (const value_type& ptr) const noexcept
    {
      return reinterpret_cast<std::size_t> (ptr);
    }
  };

  template <typename T>
  struct ptr_noexcept_equal_to
  {
    using value_type = T *;
    constexpr bool operator() (const value_type& lhs,
                               const value_type& rhs) const noexcept
    {
      return lhs == rhs;
    }
  };

#define PRINT_SIZE(TYPE) \
char (*__fail)(void)[sizeof(TYPE)] = 1;

}

#endif

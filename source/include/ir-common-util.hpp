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

#include <iosfwd>
#include <memory>
#include <functional>
#include <unordered_set>
#include <type_traits>

namespace gch
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
  
  template <typename T>
  using is_pointer_ref
    = std::is_pointer<typename std::remove_reference<T>::type>;
  
  template <typename T, typename S>
  constexpr bool isa (const S* x)
  {
    return dynamic_cast<const T*> (x) != nullptr;
  }
  
//  template <typename T, typename S, typename>
//  bool isa (const S& x);

//  template <typename T, typename S>
//  constexpr bool isa (const enable_if_t<! std::is_pointer<S>::value, S>& x)
//  {
//    return dynamic_cast<const T*> (&x) != nullptr;
//  }
//
//  template <typename T, typename S>
//  constexpr bool isa (const enable_if_t<std::is_pointer<S>::value, S>& x)
//  {
//    return dynamic_cast<const T*> (x) != nullptr;
//  }
  
  template <typename T, typename S,
        typename std::enable_if<! std::is_pointer<S>::value>::type* = nullptr>
  constexpr bool isa (const S& x)
  {
    return dynamic_cast<const T*> (&x) != nullptr;
  }
  
  template <typename T, typename S,
          typename std::enable_if<std::is_pointer<S>::value>::type* = nullptr>
  constexpr bool isa (const S& x)
  {
    return dynamic_cast<const T*> (x) != nullptr;
  }
  
//  template <class T>
//  struct unique_if
//  {
//    using singular_type = std::unique_ptr<T>;
//  };
//
//  template <class T>
//  struct unique_if<T[]>
//  {
//    using array_type = std::unique_ptr<T[]>;
//  };
//
//  template <class T, size_t N>
//  struct unique_if<T[N]>
//  {
//    struct invalid_type
//    { };
//  };
//
//  template <class T, class... Args>
//  typename unique_if<T>::singular_type
//  make_unique(Args&&... args)
//  {
//    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
//  }
//
//  template <class T>
//  typename unique_if<T>::array_type
//  make_unique(size_t n)
//  {
//    using base_type = typename std::remove_extent<T>::type;
//    return std::unique_ptr<T>(new base_type[n]());
//  }
//
//  template<typename T, typename... Args>
//  typename unique_if<T>::invalid_type
//  make_unique(Args&&...) = delete;

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
  
  template <typename Container>
  class subrange
  {
  public:
    
    using iter = typename Container::iterator;
    using citer = typename Container::const_iterator;
    using riter  = std::reverse_iterator<iter>;
    using criter  = std::reverse_iterator<citer>;
  
    subrange            (void)                = delete;
    subrange            (const subrange&)     = default;
    subrange            (subrange&&) noexcept = default;
    subrange& operator= (const subrange&)     = default;
    subrange& operator= (subrange&&) noexcept = default;
    ~subrange           (void)                = default;
    
    template <typename Functor1, typename Functor2>
    constexpr subrange (Functor1&& begin_func, Functor2&& end_func)
    noexcept
      : m_begin (std::forward<Functor1> (begin_func)),
        m_end   (std::forward<Functor2> (end_func))
    { }
    
    [[nodiscard]] auto begin   (void)       noexcept { return m_begin (); }
    [[nodiscard]] auto begin   (void) const noexcept { return citer (m_begin ()); }
    [[nodiscard]] auto cbegin  (void) const noexcept { return citer (m_begin ()); }
    
    [[nodiscard]] auto end     (void)       noexcept { return m_end (); }
    [[nodiscard]] auto end     (void) const noexcept { return citer (m_end ()); }
    [[nodiscard]] auto cend    (void) const noexcept { return citer (m_end ()); }
    
    [[nodiscard]] auto rbegin  (void)       noexcept { return riter (end ());  }
    [[nodiscard]] auto rbegin  (void) const noexcept { return criter (end ());  }
    [[nodiscard]] auto crbegin (void) const noexcept { return criter (cend ()); }
    
    [[nodiscard]] auto rend  (void)       noexcept { return riter (begin ()); }
    [[nodiscard]] auto rend  (void) const noexcept { return criter (begin ()); }
    [[nodiscard]] auto crend (void) const noexcept { return criter (cbegin ()); }
    
    [[nodiscard]] auto& front   (void)       noexcept { return *begin ();   }
    [[nodiscard]] auto& front   (void) const noexcept { return *cbegin ();   }
    
    [[nodiscard]] auto& back    (void)       noexcept { return *(--end ());    }
    [[nodiscard]] auto& back    (void) const noexcept { return *(--cend ());    }
    
    [[nodiscard]] auto size  (void) const noexcept
    {
      using std::distance;
      return distance (begin (), end ());
    }
    
    [[nodiscard]] bool empty (void) const noexcept { return begin () == end (); }
    
    void swap (subrange other)
    {
      using std::swap;
      swap (m_begin, other.m_begin);
      swap (m_end, other.m_end);
    }
  
  private:
    
    std::function<iter ()> m_begin;
    std::function<iter ()> m_end;
  };
  
  template <typename Container>
  bool operator== (const subrange<Container>& lhs, const subrange<Container>& rhs)
  {
    return std::equal (lhs.begin (), lhs.end (), rhs.begin (), rhs.end ());
  }
  
  template <typename Container>
  bool operator!= (const subrange<Container>& lhs, const subrange<Container>& rhs)
  {
    return ! (lhs == rhs);
  }
  
  template <typename Container>
  void swap (subrange<Container>& lhs, subrange<Container>& rhs)
  {
    lhs.swap (rhs);
  }

}

#endif

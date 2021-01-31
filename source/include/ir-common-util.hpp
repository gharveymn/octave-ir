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

#include <gch/optional_ref.hpp>
#include <gch/nonnull_ptr.hpp>

#include <iosfwd>
#include <memory>
#include <functional>
#include <unordered_set>
#include <type_traits>
#include <optional>

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
  using is_pointer_ref = std::is_pointer<typename std::remove_reference<T>::type>;

  template <typename T, typename S>
  constexpr bool is_a (const S* x)
  {
    return dynamic_cast<const T*> (x) != nullptr;
  }

  template <typename T, typename S,
        typename std::enable_if<! std::is_pointer<S>::value>::type * = nullptr>
  constexpr bool is_a (const S& x)
  {
    return dynamic_cast<const T*> (&x) != nullptr;
  }

  template <typename T, typename S,
          typename std::enable_if<std::is_pointer<S>::value>::type * = nullptr>
  constexpr bool is_a (const S& x)
  {
    return dynamic_cast<const T*> (x) != nullptr;
  }

  template <typename ...Ts>
  struct overloaded: Ts...
  {
    using Ts::operator()...;
  };
  template <typename ...Ts> overloaded (Ts...) -> overloaded<Ts...>;

  template <typename R, typename Overloaded>
  struct overloaded_ret;

  template <typename R, typename ...Ts>
  struct overloaded_ret<R, overloaded<Ts...>>
    : overloaded<Ts...>
  {
    constexpr explicit overloaded_ret (Ts&&... ts)
      : overloaded<Ts...> (std::forward<Ts> (ts)...)
    { }

    template <typename T>
    R operator() (T&& t)
    {
      return overloaded<Ts...>::operator() (std::forward<T> (t));
    }
  };

  template <typename R, typename ...Ts>
  constexpr overloaded_ret<R, overloaded<Ts...>> make_overloaded (Ts&&... ts)
  {
    return overloaded_ret<R, overloaded<Ts...>> { std::forward<Ts> (ts)... };
  }

  template <typename Optional, typename Function>
  constexpr auto operator>>= (Optional&& opt, Function&& f)
    -> std::enable_if_t<std::is_invocable_v<Function, decltype (*opt)>,
                        decltype (std::invoke (std::forward<Function> (f), *opt))>
  {
    using ret_type = decltype (std::invoke (std::forward<Function> (f), *opt));
    if (opt)
      return std::invoke (std::forward<Function> (f), *opt);
    return ret_type ();
  }

  // nonconst member function
  template <typename Optional, typename Ret>
  constexpr auto
  operator>>= (Optional&& opt,
               Ret (std::decay_t<decltype (*std::declval<Optional> ())>::*f) (void))
  -> std::enable_if_t<std::is_invocable_v<decltype (f), decltype (*opt)>
                  &&! std::is_const_v<decltype (*opt)>,
                      decltype (std::invoke (f, *opt))>
  {
    using ret_type = decltype (std::invoke (f, *opt));
    if (opt)
      return std::invoke (f, *opt);
    return ret_type ();
  }

  // const member function
  template <typename Optional, typename Ret>
  constexpr auto
  operator>>= (Optional&& opt,
               Ret (std::decay_t<decltype (*std::declval<Optional> ())>::*f) (void) const)
    -> std::enable_if_t<std::is_invocable_v<decltype (f), decltype (*opt)>
                    &&  std::is_const_v<decltype (*opt)>,
                        decltype (std::invoke (f, *opt))>
  {
    using ret_type = decltype (std::invoke (f, *opt));
    if (opt)
      return std::invoke (f, *opt);
    return ret_type ();
  }

  template <typename Function>
  struct maybe
  {
    template <typename ...Args>
    decltype (auto) operator() (Args&&... args)
    {
      return std::make_optional (std::invoke (m_func, std::forward<Args> (args)...));
    }
    Function m_func;
  };

  template <typename Function> maybe (Function&&) -> maybe<Function>;

  template <typename T, typename Function>
  struct selected
  {
    template <typename ...Args>
    decltype (auto) operator() (Args&&... args)
    {
      return std::invoke (m_func, std::get<T> (std::forward<Args> (args))...);
    }
    Function m_func;
  };

  template <typename T, typename Function>
  selected<T, std::decay_t<Function>> make_selected (Function&& f)
  {
    return selected<T, std::decay_t<Function>> { std::forward<Function> (f) };
  }

  template <typename Function>
  struct applied
  {
    template <typename Tuple>
    decltype (auto) operator() (Tuple&& tup)
    {
      return std::apply (m_func, std::forward<Tuple> (tup));
    }

    Function m_func;
  };

  template <typename Function> applied (Function&&) -> applied<Function>;

  template <typename ...Ts>
  using ref_tuple = std::tuple<nonnull_ptr<Ts>...>;

  template <typename Value>
  class contiguous_read_iterator
  {
  public:
    using difference_type   = typename std::iterator_traits<Value *>::difference_type;
    using value_type        = typename std::iterator_traits<Value *>::value_type;
    using pointer           = typename std::iterator_traits<Value *>::pointer;
    using reference         = typename std::iterator_traits<Value *>::reference;
    using iterator_category = typename std::iterator_traits<Value *>::iterator_category;

  private:
    template <typename U> struct is_self                         : std::false_type { };
    template <typename U> struct is_self<contiguous_read_iterator<U>> : std::true_type  { };

    template <typename U>
    static constexpr
    bool
    is_self_v = is_self<U>::value;

    template <typename U>
    static constexpr
    bool
    pointer_is_convertible_v = std::is_convertible_v<decltype (&std::declval<U&> ()), pointer>;

  public:
//  contiguous_read_iterator            (void)                                = impl;
    contiguous_read_iterator            (const contiguous_read_iterator&)     = default;
    contiguous_read_iterator            (contiguous_read_iterator&&) noexcept = default;
    contiguous_read_iterator& operator= (const contiguous_read_iterator&)     = default;
    contiguous_read_iterator& operator= (contiguous_read_iterator&&) noexcept = default;
    ~contiguous_read_iterator (void)                                          = default;

#ifdef NDEBUG
    contiguous_iterator (void) = default;
#else
    constexpr
    contiguous_read_iterator (void) noexcept
      : m_ptr ()
    { }
#endif

    constexpr explicit
    contiguous_read_iterator (Value& v) noexcept
      : m_ptr (std::addressof (v))
    { }

    template <typename U, typename = std::enable_if_t<pointer_is_convertible_v<U>>>
    constexpr /* implicit */
    contiguous_read_iterator (const contiguous_read_iterator<U>& other) noexcept
      : m_ptr (other.base ())
    { }

    GCH_CPP14_CONSTEXPR
    contiguous_read_iterator&
    operator++ (void) noexcept
    {
      ++m_ptr;
      return *this;
    }

    GCH_CPP14_CONSTEXPR
    contiguous_read_iterator
    operator++ (int) noexcept
    {
      return contiguous_read_iterator (m_ptr++);
    }

    GCH_CPP14_CONSTEXPR
    contiguous_read_iterator&
    operator-- (void) noexcept
    {
      --m_ptr;
      return *this;
    }

    GCH_CPP14_CONSTEXPR
    contiguous_read_iterator
    operator-- (int) noexcept
    {
      return contiguous_read_iterator (m_ptr--);
    }

    GCH_CPP14_CONSTEXPR
    contiguous_read_iterator&
    operator+= (difference_type n) noexcept
    {
      m_ptr += n;
      return *this;
    }

    constexpr
    contiguous_read_iterator
    operator+ (difference_type n) const noexcept
    {
      return contiguous_read_iterator (m_ptr + n);
    }

    GCH_CPP14_CONSTEXPR
    contiguous_read_iterator&
    operator-= (difference_type n) noexcept
    {
      m_ptr -= n;
      return *this;
    }

    constexpr
    contiguous_read_iterator
    operator- (difference_type n) const noexcept
    {
      return contiguous_read_iterator (m_ptr - n);
    }

    constexpr
    reference operator* (void) const noexcept
    {
      return *m_ptr;
    }

    constexpr
    pointer
    operator-> (void) const noexcept
    {
      return m_ptr;
    }

    constexpr
    reference
    operator[] (difference_type n) const noexcept
    {
      return m_ptr[n];
    }

    constexpr
    const pointer&
    base () const noexcept
    {
      return m_ptr;
    }

    /* comparisons */

  private:
    pointer m_ptr;
  };

  template <typename ValueLHS,
            typename ValueRHS>
  constexpr
  bool
  operator== (const contiguous_read_iterator<ValueLHS>& lhs,
              const contiguous_read_iterator<ValueRHS>& rhs) noexcept
  {
    return lhs.base () == rhs.base ();
  }

  template <typename Value>
  constexpr
  bool
  operator== (const contiguous_read_iterator<Value>& lhs,
              const contiguous_read_iterator<Value>& rhs) noexcept
  {
    return lhs.base () == rhs.base ();
  }

  template <typename ValueLHS,
            typename ValueRHS>
  constexpr
  bool
  operator!= (const contiguous_read_iterator<ValueLHS>& lhs,
              const contiguous_read_iterator<ValueRHS>& rhs) noexcept
  {
    return lhs.base () != rhs.base ();
  }

  template <typename Value>
  constexpr
  bool
  operator!= (const contiguous_read_iterator<Value>& lhs,
              const contiguous_read_iterator<Value>& rhs) noexcept
  {
    return lhs.base () != rhs.base ();
  }

  template <typename ValueLHS,
            typename ValueRHS>
  constexpr
  bool
  operator< (const contiguous_read_iterator<ValueLHS>& lhs,
             const contiguous_read_iterator<ValueRHS>& rhs) noexcept
  {
    return lhs.base () < rhs.base ();
  }

  template <typename Value>
  constexpr
  bool
  operator< (const contiguous_read_iterator<Value>& lhs,
             const contiguous_read_iterator<Value>& rhs) noexcept
  {
    return lhs.base () < rhs.base ();
  }

  template <typename ValueLHS,
            typename ValueRHS>
  constexpr
  bool
  operator> (const contiguous_read_iterator<ValueLHS>& lhs,
             const contiguous_read_iterator<ValueRHS>& rhs) noexcept
  {
    return lhs.base () > rhs.base ();
  }

  template <typename Value>
  constexpr
  bool
  operator> (const contiguous_read_iterator<Value>& lhs,
             const contiguous_read_iterator<Value>& rhs) noexcept
  {
    return lhs.base () > rhs.base ();
  }

  template <typename ValueLHS,
            typename ValueRHS>
  constexpr
  bool
  operator<= (const contiguous_read_iterator<ValueLHS>& lhs,
              const contiguous_read_iterator<ValueRHS>& rhs) noexcept
  {
    return lhs.base () <= rhs.base ();
  }

  template <typename Value>
  constexpr
  bool
  operator<= (const contiguous_read_iterator<Value>& lhs,
              const contiguous_read_iterator<Value>& rhs) noexcept
  {
    return lhs.base () <= rhs.base ();
  }

  template <typename ValueLHS,
            typename ValueRHS>
  constexpr
  bool
  operator>= (const contiguous_read_iterator<ValueLHS>& lhs,
              const contiguous_read_iterator<ValueRHS>& rhs) noexcept
  {
    return lhs.base () >= rhs.base ();
  }

  template <typename Value>
  constexpr
  bool
  operator>= (const contiguous_read_iterator<Value>& lhs,
              const contiguous_read_iterator<Value>& rhs) noexcept
  {
    return lhs.base () >= rhs.base ();
  }

  template <typename ValueLHS,
            typename ValueRHS>
  constexpr
  auto
  operator- (const contiguous_read_iterator<ValueLHS>& lhs,
             const contiguous_read_iterator<ValueRHS>& rhs) noexcept
    -> decltype (lhs.base () - rhs.base ())
  {
    return lhs.base () - rhs.base ();
  }

  template <typename Value>
  constexpr
  auto
  operator- (const contiguous_read_iterator<Value>& lhs,
             const contiguous_read_iterator<Value>& rhs) noexcept
    -> decltype (lhs.base () - rhs.base ())
  {
    return lhs.base () - rhs.base ();
  }

  template <typename Value>
  constexpr
  contiguous_read_iterator<Value>
  operator+ (typename contiguous_read_iterator<Value>::difference_type n,
             const contiguous_read_iterator<Value>& it) noexcept
  {
    return it + n;
  }

  template <typename Iterator>
  auto
  as_contiguous_iterator (Iterator it)
    -> std::remove_reference_t<decltype (*it)>
  {
    return contiguous_read_iterator<std::remove_reference_t<decltype (*it)>> (*it);
  }

}

#endif

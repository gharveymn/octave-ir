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
  class ir_type;

  class ir_exception : public std::exception
  {
  public:
    explicit ir_exception (const char *str)
      : m_str (str)
    { }

    explicit ir_exception (std::string str)
      : m_str (std::move (str))
    { }

    const char* what (void) const noexcept override
    {
      return m_str.c_str ();
    }

  private:
    std::string m_str;
  };

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
  struct ir_printer
  {
    using ir_class = T;

    static std::ostream& short_print (std::ostream& os, const ir_class&);
    static std::ostream& long_print  (std::ostream& os, const ir_class&);
  };

  std::ostream& operator<< (std::ostream& os, const ir_type& ty);

  template <typename T>
  using is_pointer_ref = std::is_pointer<typename std::remove_reference<T>::type>;

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

  template <typename Optional, typename TrueFunction, typename FalseFunction>
  constexpr
  auto
  conditional_transform (Optional&& opt, TrueFunction&& tf, FalseFunction&& ff)
    -> std::enable_if_t<std::is_invocable_v<TrueFunction, decltype (*opt)>
                    &&  std::is_invocable_v<FalseFunction>,
                        decltype (std::invoke (std::forward<TrueFunction> (tf), *opt))>
  {
    if (opt)
      return std::invoke (std::forward<TrueFunction> (tf), *opt);
    return std::invoke (std::forward<FalseFunction> (ff));
  }

  template <typename T, typename Function>
  struct selected
  {
    template <typename ...Args>
    decltype (auto)
    operator() (Args&&... args)
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
    contiguous_read_iterator (void) = default;
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

  template <typename Container>
  class set_inserter
  {
  public:
    using container_type = Container;

    using difference_type   = std::ptrdiff_t;
    using value_type        = void;
    using pointer           = void;
    using reference         = void;
    using iterator_category = std::output_iterator_tag;

    set_inserter            (void)                    = delete;
    set_inserter            (const set_inserter&)     = default;
    set_inserter            (set_inserter&&) noexcept = default;
    set_inserter& operator= (const set_inserter&)     = default;
    set_inserter& operator= (set_inserter&&) noexcept = default;
    ~set_inserter           (void)                    = default;

    constexpr explicit
    set_inserter (Container& c)
      : m_container { c }
    { }

    constexpr
    set_inserter&
    operator= (const typename Container::value_type& val)
    {
      m_container->insert (val);
      return *this;
    }

    constexpr
    set_inserter&
    operator= (typename Container::value_type&& val)
    {
      m_container->insert (std::move (val));
      return *this;
    }

    constexpr
    set_inserter&
    operator* (void) noexcept
    {
      return *this;
    }

    constexpr
    set_inserter&
    operator++ (void) noexcept
    {
      return *this;
    }

    constexpr
    set_inserter&
    operator++ (int) noexcept
    {
      return *this;
    }

  private:
    nonnull_ptr<Container> m_container;
  };

  template <typename Container>
  class set_emplacer
  {
    template <typename Tuple, std::size_t... I>
    constexpr
    void
    assign (Tuple&& t, std::index_sequence<I...>)
    {
      m_container->emplace (std::get<I> (std::forward<Tuple> (t))...);
    }

  public:
    using container_type = Container;

    using difference_type   = std::ptrdiff_t;
    using value_type        = void;
    using pointer           = void;
    using reference         = void;
    using iterator_category = std::output_iterator_tag;

    set_emplacer            (void)                    = delete;
    set_emplacer            (const set_emplacer&)     = default;
    set_emplacer            (set_emplacer&&) noexcept = default;
    set_emplacer& operator= (const set_emplacer&)     = default;
    set_emplacer& operator= (set_emplacer&&) noexcept = default;
    ~set_emplacer           (void)                    = default;

    constexpr explicit
    set_emplacer (Container& c)
      : m_container { std::addressof (m_container) }
    { }

    template <typename Tuple,
              std::enable_if_t<! std::is_same_v<set_emplacer, std::decay_t<Tuple>>> * = nullptr>
    constexpr
    set_emplacer&
    operator= (Tuple&& t)
    {
      assign (std::forward<Tuple> (t),
              std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>> { });
      return *this;
    }

    constexpr
    set_emplacer&
    operator* (void) noexcept
    {
      return *this;
    }

    constexpr
    set_emplacer&
    operator++ (void) noexcept
    {
      return *this;
    }

    constexpr
    set_emplacer&
    operator++ (int) noexcept
    {
      return *this;
    }

  private:
    nonnull_ptr<Container> m_container;
  };

}

#endif

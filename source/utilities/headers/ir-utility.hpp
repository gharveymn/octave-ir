/** ir-utility.hpp
 * Defines an pointer wrapper which is not nullable.
 *
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_UTILITIES_IR_UTILITY_HPP
#define OCTAVE_IR_UTILITIES_IR_UTILITY_HPP

#include "ir-common.hpp"

#include <gch/nonnull_ptr.hpp>

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

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
    constexpr explicit
    named_type (U&& u, Args&&... args)
          noexcept (std::is_nothrow_constructible_v<value_type, U, Args...>)
      : m_value (std::forward<U> (u), std::forward<Args> (args)...)
    { }

    constexpr
    operator const value_type& (void) const noexcept
    {
      return m_value;
    }

    constexpr
    operator value_type&& (void) && noexcept
    {
      return std::move (m_value);
    }

  private:
    value_type m_value;
  };

  template <typename Derived, typename T>
  class transparent_named_type
    : public named_type<T>
  {
  public:
    using value_type = T;

    using named_type<T>::named_type;

    template <typename U,
              std::enable_if_t<! std::is_same_v<transparent_named_type, std::decay_t<U>>
                             &&  std::is_convertible_v<U, T>> * = nullptr>
    constexpr GCH_IMPLICIT_CONVERSION
    transparent_named_type (U&& u)
          noexcept (std::is_nothrow_constructible_v<T, U>)
      : named_type<T> (std::forward<U> (u))
    { }

    constexpr
    operator value_type& (void) & noexcept
    {
      T&& ret = static_cast<T&&> (std::move (*this));
      return static_cast<T&> (ret);
    }
  };

  template <typename Derived, typename T, typename U>
  constexpr
  Derived&
  operator+= (transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    static_cast<T&> (lhs) += rhs;
    return static_cast<Derived&> (lhs);
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived&
  operator-= (transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    static_cast<T&> (lhs) -= rhs;
    return static_cast<Derived&> (lhs);
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived&
  operator*= (transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    static_cast<T&> (lhs) *= rhs;
    return static_cast<Derived&> (lhs);
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived&
  operator/= (transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    static_cast<T&> (lhs) /= rhs;
    return static_cast<Derived&> (lhs);
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived&
  operator%= (transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    static_cast<T&> (lhs) %= rhs;
    return static_cast<Derived&> (lhs);
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived&
  operator&= (transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    static_cast<T&> (lhs) &= rhs;
    return static_cast<Derived&> (lhs);
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived&
  operator|= (transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    static_cast<T&> (lhs) |= rhs;
    return static_cast<Derived&> (lhs);
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived&
  operator^= (transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    static_cast<T&> (lhs) ^= rhs;
    return static_cast<Derived&> (lhs);
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived&
  operator<<= (transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    static_cast<T&> (lhs) <<= rhs;
    return static_cast<Derived&> (lhs);
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived&
  operator>>= (transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    static_cast<T&> (lhs) >>= rhs;
    return static_cast<Derived&> (lhs);
  }

  template <typename Derived, typename T>
  constexpr
  Derived&
  operator++ (transparent_named_type<Derived, T>& val)
  {
    ++static_cast<T&> (val);
    return static_cast<Derived&> (val);
  }

  template <typename Derived, typename T>
  constexpr
  Derived
  operator++ (transparent_named_type<Derived, T>& val, int)
  {
    return Derived { static_cast<T&> (val)++ };
  }

  template <typename Derived, typename T>
  constexpr
  Derived&
  operator-- (transparent_named_type<Derived, T>& val)
  {
    --static_cast<T&> (val);
    return static_cast<Derived&> (val);
  }

  template <typename Derived, typename T>
  constexpr
  Derived
  operator-- (transparent_named_type<Derived, T>& val, int)
  {
    return Derived { static_cast<T&> (val)-- };
  }

  template <typename Derived, typename T>
  constexpr
  Derived
  operator+ (const transparent_named_type<Derived, T>& val)
  {
    return Derived { +static_cast<const T&> (val) };
  }

  template <typename Derived, typename T>
  constexpr
  Derived
  operator- (const transparent_named_type<Derived, T>& val)
  {
    return Derived { -static_cast<const T&> (val) };
  }

  template <typename Derived, typename T>
  constexpr
  Derived
  operator~ (const transparent_named_type<Derived, T>& val)
  {
    return Derived { ~static_cast<const T&> (val) };
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived
  operator+ (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return Derived { Derived { static_cast<const T&> (lhs) + rhs } };
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived
  operator- (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return Derived { static_cast<const T&> (lhs) - rhs };
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived
  operator* (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return Derived { static_cast<const T&> (lhs) * rhs };
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived
  operator/ (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return Derived { static_cast<const T&> (lhs) / rhs };
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived
  operator% (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return Derived { static_cast<const T&> (lhs) % rhs };
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived
  operator& (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return Derived { static_cast<const T&> (lhs) & rhs };
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived
  operator| (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return Derived { static_cast<const T&> (lhs) | rhs };
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived
  operator^ (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return Derived { static_cast<const T&> (lhs) ^ rhs };
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived
  operator<< (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return Derived { static_cast<const T&> (lhs) << rhs };
  }

  template <typename Derived, typename T, typename U>
  constexpr
  Derived
  operator>> (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return Derived { static_cast<const T&> (lhs) >> rhs };
  }

  template <typename Derived, typename T>
  constexpr
  bool
  operator! (const transparent_named_type<Derived, T>& val)
  {
    return ! static_cast<const T&> (val);
  }

  template <typename Derived, typename T, typename U>
  constexpr
  bool
  operator&& (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return static_cast<const T&> (lhs) && rhs;
  }

  template <typename Derived, typename T, typename U>
  constexpr
  bool
  operator|| (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return static_cast<const T&> (lhs) || rhs;
  }

  template <typename Derived, typename T, typename U>
  constexpr
  bool
  operator== (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return static_cast<const T&> (lhs) == rhs;
  }

  template <typename Derived, typename T, typename U>
  constexpr
  bool
  operator!= (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return static_cast<const T&> (lhs) != rhs;
  }

  template <typename Derived, typename T, typename U>
  constexpr
  bool
  operator< (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return static_cast<const T&> (lhs) < rhs;
  }

  template <typename Derived, typename T, typename U>
  constexpr
  bool
  operator> (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return static_cast<const T&> (lhs) > rhs;
  }

  template <typename Derived, typename T, typename U>
  constexpr
  bool
  operator<= (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return static_cast<const T&> (lhs) <= rhs;
  }

  template <typename Derived, typename T, typename U>
  constexpr
  bool
  operator>= (const transparent_named_type<Derived, T>& lhs, const U& rhs)
  {
    return static_cast<const T&> (lhs) >= rhs;
  }

  template <typename T, typename ParameterTag>
  class named_parameter
    : public named_type<T>
  {
  public:
    using tag = ParameterTag;

    using named_type<T>::named_type;
  };

  namespace detail
  {

    namespace aggregate_rebinder_adl
    {

      using std::get;

      template <std::size_t I, typename T>
      void get (T&& t)
        noexcept (noexcept (get<I> (std::forward<T> (t))));

    } // namespace gch::detail::aggregate_rebinder_adl

    template <typename AggregateType, typename IndexSequence>
    struct aggregate_rebinder
    { };

    template <typename T, std::size_t ...I>
    struct aggregate_rebinder<T, std::index_sequence<I...>>
    {

      template <typename Tuple>
      constexpr
      T
      operator() (Tuple&& tup) const
        noexcept (noexcept (T { aggregate_rebinder_adl::get<I> (std::forward<Tuple> (tup))... }))
      {
        using std::get;
        return { get<I> (std::forward<Tuple> (tup))... };
      }

    };

  } // namespace gch::detail

  template <typename AggregateType, typename Tuple>
  constexpr
  AggregateType
  rebind_to_aggregate (Tuple&& tup)
  {
    using seq_type = std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>;
    return detail::aggregate_rebinder<AggregateType, seq_type> { } (std::forward<Tuple> (tup));
  }

  template <typename Enum, std::enable_if_t<std::is_enum_v<Enum>> * = nullptr>
  constexpr
  std::underlying_type_t<Enum>
  underlying_cast (Enum e) noexcept
  {
    return static_cast<std::underlying_type_t<Enum>> (e);
  }

} // namespace gch

#endif // OCTAVE_IR_UTILITIES_IR_UTILITY_HPP

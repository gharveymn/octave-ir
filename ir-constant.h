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

#if ! defined (octave_ir_constant_h)
#define octave_ir_constant_h 1

#include "octave-config.h"

#include "ir-common.h"
#include "ir-type-std.h"

namespace octave
{
  
  // The following code is basically the same as std::tuple without
  // some features, and extended for our purposes.
  
  template <typename>
  struct ir_printer;
  
  class ir_operand;
  
  template <typename...>
  class ir_constant;
  
  template <std::size_t Idx, typename T>
  struct ir_constant_base
  {
    using value_type = T;
    
    constexpr ir_constant_base (void) = default;
    
    explicit constexpr ir_constant_base (value_type&& v)
      : m_value (std::move (v))
    { }
    
    constexpr ir_constant_base (const ir_constant_base&) = default;
    constexpr ir_constant_base (ir_constant_base&&) noexcept = default;
    
    value_type& value (void) noexcept { return m_value; }
    
    constexpr const value_type& value (void) const noexcept
    {
      return m_value;
    }
    
  private:
    value_type m_value;
  };
  
  template <std::size_t Idx, typename ...Ts>
  struct ir_constant_impl;
  
  template <std::size_t Idx, typename Head, typename ...Tail>
  struct ir_constant_impl<Idx, Head, Tail...>
    : private ir_constant_base<Idx, Head>,
      public  ir_constant_impl<Idx + 1, Tail...>
      
  {
    template<std::size_t, typename ...>
    friend struct ir_constant_impl;
    
    using self_type = ir_constant_impl<Idx, Head, Tail...>;
    
    using base_type = ir_constant_base<Idx, Head>;
    using tail_type  = ir_constant_impl<Idx + 1, Tail...>;
  
    using value_type = typename base_type::value_type;
  
    value_type& base_value (void) noexcept
    {
      return base_type::value ();
    }
    
    constexpr const value_type& base_value (void) const noexcept
    {
      return base_type::value ();
    }
    
    tail_type& tail (ir_constant_impl& r) noexcept
    {
      return r;
    }
    
    constexpr const tail_type& tail (const ir_constant_impl& r) const noexcept
    {
      return r;
    }
  
    template <std::size_t N, typename Dummy = void>
    struct offset : tail_type::template offset<N - 1>
    { };
    
    template <typename Dummy>
    struct offset<0, Dummy>
    {
      using type = self_type;
    };
  
    template <typename T, typename E = void>
    struct match : tail_type::template match<T>
    { };
    
    template <typename T>
    struct match<T, std::enable_if_t<std::is_same<value_type, T>::value>>
    {
      using type = self_type;
    };
    
    template <std::size_t N = 1>
    struct count_proceeding
      : tail_type::template count_proceeding<std::size_t, N + 1>
    { };
    
    constexpr ir_constant_impl (void) = default;
    
    explicit constexpr ir_constant_impl (const value_type& val,
                                         const Tail... tail)
      : base_type (val),
        tail_type (tail...)
    { }
    
    constexpr ir_constant_impl (const ir_constant_impl&) = default;
    
    constexpr ir_constant_impl (ir_constant_impl&& o)
      noexcept (conjunction<std::is_nothrow_move_constructible<value_type>,
                        std::is_nothrow_move_constructible<tail_type>>::value)
      : base_type (std::forward<value_type> (o.value ())),
        tail_type (std::move (o.tail ()))
    { }
  };
  
  template <std::size_t Idx, typename Head>
  struct ir_constant_impl<Idx, Head>
    : private ir_constant_base<Idx, Head>
  {
    template<std::size_t, typename ...>
    friend struct ir_constant_impl;
  
    using base_type = ir_constant_base<Idx, Head>;
  
    using value_type = typename base_type::value_type;
  
    value_type& base_value (void) noexcept
    {
      return base_type::value ();
    }
  
    constexpr const value_type& base_value (void) const noexcept
    {
      return base_type::value ();
    }
  
    template <std::size_t N, typename Dummy = void>
    struct offset
    {
      static_assert (N == 0, "invalid offset");
    };
  
    template <typename Dummy>
    struct offset<0, Dummy>
    {
      using type = ir_constant_impl;
    };
  
  
    template <typename T, typename E = void>
    struct match
    {
      static_assert (std::is_same <value_type, T>::value, "type not found");
    };
    
    template <typename T>
    struct match<T, std::enable_if_t<std::is_same<value_type, T>::value>>
    {
      using type = ir_constant_impl;
    };
  
    template <std::size_t N = 1>
    struct count_proceeding : std::integral_constant<std::size_t, N>
    { };
    
    constexpr ir_constant_impl (void) = default;
    
    explicit constexpr ir_constant_impl (const value_type& val)
      : base_type (val)
    { }
    
    constexpr ir_constant_impl (const ir_constant_impl&) = default;
    
    constexpr ir_constant_impl (ir_constant_impl&& o)
      noexcept (std::is_nothrow_move_constructible<value_type>::value)
      : base_type (std::forward<value_type> (o.base_value ()))
    { }
    
  };
  
  template <typename ...Ts>
  class ir_constant : public ir_constant_impl<0, Ts...>
  {
  private:
    
    using impl_begin_type = ir_constant_impl<0, Ts...>;
    
    template <std::size_t N>
    using impl_type = typename impl_begin_type::template offset<N>::type;

    template <typename T>
    using match_impl_type = typename impl_begin_type::template match<T>::type;
    
  public:
    
    template <std::size_t N>
    using element_t = typename impl_type<N>::value_type;
    
    template <std::size_t N = 0>
    element_t<N>& get (void) noexcept
    {
      return impl_type<N>::base_value ();
    }
  
    template <std::size_t N = 0>
    constexpr const element_t<N>& get (void) const noexcept
    {
      return impl_type<N>::base_value ();
    }
    
    template <typename T>
    T& get (void) noexcept
    {
      return match_impl_type<T>::base_value ();
    }
  
    template <typename T>
    constexpr const T& get (void) const noexcept
    {
      return match_impl_type<T>::base_value ();
    }
    
    constexpr std::size_t cardinality (void) const noexcept
    {
      return impl_begin_type::count_proceeding::value;
    }
    
    template <std::size_t N = 0>
    constexpr ir_type get_ir_type (void) const noexcept
    {
      return ir_type::get<impl_type<N>::value_type> ();
    }
    
    constexpr ir_constant (void)
      : impl_begin_type ()
    { }
    
    template <typename ...Args>
    constexpr ir_constant (Args&&... args)
      : impl_begin_type (std::forward<Args> (args)...)
    { }
    
    constexpr ir_constant (const ir_constant&) = default;
    
    constexpr ir_constant (ir_constant&& o)
      noexcept (std::is_nothrow_move_constructible<impl_begin_type>::value)
      : impl_begin_type (std::forward <ir_constant_impl> (o))
    { }
  
  protected:
  
  };
  
//  constexpr ir_constant<int, long, char *> x;
//
//  constexpr decltype(x)::element_t<0> x0 = x.get<0> ();
//  constexpr decltype(x)::element_t<1> x1 = x.get<1> ();
//  constexpr decltype(x)::element_t<2> x2 = x.get<2> ();
//
//  constexpr int xi    = x.get<int> ();
//  constexpr long xl   = x.get<long> ();
//  constexpr char *xcp = x.get<char *> ();

}

#endif

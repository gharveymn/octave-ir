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

#if ! defined (ir_component_h)
#define ir_component_h 1

#include "octave-config.h"

#include "ir-common.h"

#include <type_traits>
#include <vector>
#include <variant>

namespace octave
{

  class ir_basic_block;
  class ir_function;

  // basically just an iterator-compatible wrapper for a pointer
  // to avoid UB of incrementing a non-associated pointer.
  template <typename Value>
  class value_iterator
  {
  public:

    using difference_type   = int8_t;
    using value_type        = std::remove_const_t<Value>;
    using pointer           = Value *;
    using reference         = Value&;
    using iterator_category = std::random_access_iterator_tag;

    constexpr explicit value_iterator (reference ref)
      : m_ptr (std::addressof (ref)),
        m_is_end (false)
    { }

    value_iterator& operator++ (void) noexcept
    {
      m_is_end = true;
      return *this;
    }

    value_iterator operator++ (int) noexcept
    {
      value_iterator save = *this;
      ++*this;
      return save;
    }

    value_iterator& operator--(void) noexcept
    {
      m_is_end = false;
      return *this;
    }

    value_iterator operator-- (int) noexcept
    {
      value_iterator save = *this;
      --*this;
      return save;
    }

    value_iterator& operator+= (difference_type n) noexcept
    {
      if (n > 0)
        return ++*this;
      else if (n < 0)
        return --*this;
      return *this;
    }

    constexpr value_iterator
    operator+ (difference_type n) const noexcept
    {
      return value_iterator (*this) += n;
    }

    friend constexpr value_iterator
    operator+(difference_type n, value_iterator it) noexcept
    {
      return it + n;
    }

    constexpr value_iterator& operator-= (difference_type n) noexcept
    {
      return operator+= (-n);
    }

    constexpr value_iterator
    operator- (difference_type n) const noexcept
    {
      return operator+ (-n);
    }

    difference_type operator- (const value_iterator& other) const noexcept
    {
      return m_is_end - other.m_is_end;
    }

    reference operator[] (difference_type n) const noexcept
    {
      return *(operator+ (n));
    }

    //
    // The below functions will be wrong if other does not refer to the same 
    // object. However, this should be fine because it works the same way
    // for std container iterators.
    //

    constexpr bool operator< (const value_iterator& other) const noexcept
    {
      return ! m_is_end && other.m_is_end;
    }

    constexpr bool operator> (const value_iterator& other) const noexcept
    {
      return m_is_end && ! other.m_is_end;
    }

    constexpr bool operator<= (const value_iterator& other) const noexcept
    {
      return m_is_end ? other.m_is_end : true;
    }

    constexpr bool operator>= (const value_iterator& other) const noexcept
    {
      return m_is_end ? true : ! other.m_is_end;
    }

    constexpr bool operator== (const value_iterator& other) const noexcept
    {
      return (*m_ptr == *other.m_ptr) && (m_is_end == other.m_is_end);
    }

    constexpr bool operator!= (const value_iterator& other) const noexcept
    {
      return ! operator== (other);
    }

    constexpr reference operator* (void) const noexcept
    {
      return *get ();
    }

    constexpr pointer operator-> (void) const noexcept
    {
      return get ();
    }

    void swap(value_iterator& other) noexcept
    {
      using std::swap;
      swap (m_ptr, other.m_ptr);
      swap (m_is_end, other.m_is_end);
    }

  private:

    [[nodiscard]]
    constexpr pointer get (void) const noexcept
    {
      return m_is_end ? nullptr : m_ptr;
    }

    pointer m_ptr    = nullptr;
    bool    m_is_end = true;

  };

  template <typename T>
  inline void
  swap(value_iterator<T>& it1, value_iterator<T>& it2)
  {
    it1.swap (it2);
  }

  template <class... FunctionObjects>
  struct overload : FunctionObjects...
  {
    using FunctionObjects::operator()...;
  };

  template <class... FunctionObjects> 
  overload (FunctionObjects...) -> overload<FunctionObjects...>;

  template <typename ...Its>
  class variant_iterator
  {

  public:

    template <std::size_t I, typename T, typename ...Ts>
    struct nth_element_impl {
      using type = typename nth_element_impl<I-1, Ts...>::type;
    };

    template <typename T, typename ...Ts>
    struct nth_element_impl<0, T, Ts...> {
      using type = T;
    };

    template <std::size_t I>
    using get_iter = typename nth_element_impl<I, Its...>::type;

    template <typename T, typename ...Ts>
    struct all_same : std::conjunction<std::is_same<T, Ts>...>
    { };

    static_assert (all_same<typename Its::value_type...>::value,
                   "value types must be equal.");

    static_assert (all_same<typename Its::reference...>::value,
                   "reference types must be equal.");

    static_assert (all_same<typename Its::pointer...>::value,
                   "pointer types must be equal.");
    
  private:
    
    template <typename It>
    using difference_type_t = typename It::difference_type;

    template <typename It>
    using value_type_t = typename It::value_type;

    template <typename It>
    using pointer_t = typename It::pointer;

    template <typename It>
    using reference_t = typename It::reference;

    template <typename It>
    using iterator_category_t = typename It::iterator_category;

  public:
    
    using difference_type   = std::common_type_t<difference_type_t<Its>...>;
    using value_type        = value_type_t<get_iter<0>>;
    using pointer           = pointer_t<get_iter<0>>;
    using reference         = reference_t<get_iter<0>>;
    using iterator_category = std::common_type_t<iterator_category_t<Its>...>;

    struct type_exception : std::exception
    {
      type_exception (void) = default;

      type_exception (const char *str)
        : m_str (str)
      { }

      const char* what() const noexcept override
      {
        return m_str;
      }
    private:
      const char* m_str = "Iterator types are not the same.";
    };

  private:

    template <template<typename...> class BinaryOperator>
    struct binary_visitor : BinaryOperator<Its>...
    {
      using BinaryOperator<Its>::operator()...;

      using return_type = std::invoke_result_t<BinaryOperator<get_iter<0>>,
        get_iter<0>, get_iter<0>>;

      static_assert (all_same<std::invoke_result_t<BinaryOperator<Its>,
                       Its, Its>...>::value,
                     "Binary operator return types must be uniform.");

      template <typename T, typename U,
        std::enable_if_t<! std::is_same<T, U>::value>* = nullptr>
      constexpr return_type operator() (T, U) { throw type_exception (); }
    };

  public:

    // DefaultConstructible
    constexpr variant_iterator (void) noexcept = default;

    template <typename It>
    constexpr explicit variant_iterator (It it)
      : m_variant (it)
    { }

    constexpr variant_iterator (const variant_iterator& other)
      : m_variant (other.m_variant)
    { }

    constexpr variant_iterator (variant_iterator&& other) noexcept
      : m_variant (std::move (other.m_variant))
    { }

    // CopyAssignable
    // ref-qualified to prevent assignment to rvalues
    variant_iterator& operator= (const variant_iterator& other) &
    {
      if (&other != this)
        m_variant = other.m_variant;
      return *this;
    }

    // MoveAssignable
    // ref-qualified to prevent assignment to rvalues
    variant_iterator& operator= (variant_iterator&& other) & noexcept
    {
      m_variant = other.m_variant;
      return *this;
    }

    variant_iterator& operator++ (void) noexcept
    {
      std::visit ([] (auto&& it) { ++it; }, m_variant);
      return *this;
    }

    variant_iterator operator++ (int) noexcept
    {
      return std::visit ([] (auto&& it) // -> variant_iterator 
                         { return variant_iterator (it++); }, m_variant);
    }

    variant_iterator& operator-- (void) noexcept
    {
      std::visit ([] (auto&& it) { --it; }, m_variant);
      return *this;
    }

    variant_iterator operator-- (int) noexcept
    {
      return std::visit ([] (auto&& it) // -> variant_iterator
                         { return variant_iterator (it--); }, m_variant);
    }

    variant_iterator& operator+= (difference_type n) noexcept
    {
      std::visit ([&n] (auto&& it) { it += n; }, m_variant);
      return *this;
    }

    constexpr variant_iterator operator+ (difference_type n) const noexcept
    {
      return std::visit ([&n] (auto&& it) // -> variant_iterator
                         { return variant_iterator (it + n); }, m_variant);
    }

    friend constexpr variant_iterator operator+ (difference_type n,
                                                 const variant_iterator& var)
    {
      return std::visit ([&n] (auto&& it) // -> variant_iterator
                         { return variant_iterator<Its...> (n + it); },
                         var.m_variant);
    }

    variant_iterator& operator-= (difference_type n) noexcept
    {
      std::visit ([&n] (auto&& it) { it -= n; }, m_variant);
      return *this;
    }

    constexpr variant_iterator operator- (difference_type n) const noexcept
    {
      return std::visit ([&n] (auto&& it) // -> variant_iterator
                         { return variant_iterator (it - n); }, m_variant);
    }

  public:

    constexpr difference_type operator- (const variant_iterator& other) const
    {
      using diff_type = difference_type;
      return std::visit (overload {
                           [] (Its lhs, Its rhs) -> diff_type { return lhs - rhs; }...,
                           [] (auto lhs, auto rhs) -> diff_type { throw type_exception (); } },
                         m_variant, other.m_variant);
    }

    constexpr reference operator[] (difference_type n) const noexcept
    {
      return std::visit ([&n] (auto&& it) -> reference
                         { return it[n]; }, m_variant);
    }

    constexpr bool operator< (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::less> { },
                         m_variant, other.m_variant);
    }

    constexpr bool operator> (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::greater> { },
                         m_variant, other.m_variant);
    }

    constexpr bool operator<= (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::less_equal> { },
                         m_variant, other.m_variant);
    }

    constexpr bool operator>= (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::greater_equal> { },
                         m_variant, other.m_variant);
    }

    // EqualityComparable
    constexpr bool operator== (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::equal_to> { },
                         m_variant, other.m_variant);
    }

    constexpr bool operator!= (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::not_equal_to> { },
                         m_variant, other.m_variant);
    }

    constexpr reference operator* (void) const noexcept
    {
      return std::visit ([] (auto&& it) -> reference
                         { return it.operator* (); }, m_variant);
    }

    pointer operator-> (void) const noexcept
    {
      return std::visit ([] (auto&& it) // -> pointer
                         { return it.operator-> (); }, m_variant);
    }

    void swap (variant_iterator& other)
    noexcept (std::is_nothrow_swappable<std::variant<Its...>>::value)
    {
      using std::swap;
      swap (this->m_variant, other.m_variant);
    }

  private:

    std::variant<Its...> m_variant;

  };

  template <typename ...Its>
  inline std::enable_if_t<std::is_swappable<std::variant<Its...>>::value>
  swap(variant_iterator<Its...>& lhs, variant_iterator<Its...>& rhs)
  noexcept (noexcept (lhs.swap(rhs)))
  {
    lhs.swap (rhs);
  }

  // abstract
  class ir_component
  {
  public:

    using link_cache_vec = std::vector<ir_basic_block *>;
    using link_cache_iter = link_cache_vec::iterator;
    using link_cache_citer = link_cache_vec::const_iterator;

    constexpr ir_component (void) noexcept = default;

    virtual ~ir_component (void) noexcept = 0;

    using link_iter  = variant_iterator<link_cache_iter, 
                                        value_iterator<ir_basic_block *>>;
    using link_citer = variant_iterator<link_cache_citer, 
                                      value_iterator<ir_basic_block * const>>;
  
    virtual void               reset           (void)       noexcept = 0;
    virtual link_iter          leaf_begin      (void)                = 0;
    virtual link_iter          leaf_end        (void)                = 0;
    virtual ir_basic_block *   get_entry_block (void)       noexcept = 0;
    virtual ir_function&       get_function    (void)       noexcept = 0;
    virtual const ir_function& get_function    (void) const noexcept = 0;
    
    template <typename T>
    using is_component = std::is_base_of<ir_component, T>;

  private:

  };

}

#endif

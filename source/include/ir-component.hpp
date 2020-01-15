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

#include "ir-common.hpp"

#include <type_traits>
#include <vector>
#include <variant>

namespace gch
{

  class ir_basic_block;
  class ir_function;

  // basically just an iterator-compatible wrapper for a pointer
  // to avoid UB of incrementing a non-associated pointer.
  template <typename Value>
  class reference_iterator
  {
  public:
  
    using difference_type   = int8_t;
    using value_type        = std::remove_cv_t<Value>;
    using pointer           = Value *;
    using reference         = Value&;
    using iterator_category = std::random_access_iterator_tag;
  
    friend reference_iterator<std::add_const_t<Value>>;
    friend reference_iterator<std::add_volatile_t<Value>>;
    friend reference_iterator<std::add_cv_t<Value>>;
  
    reference_iterator            (void)                          = default;
    reference_iterator            (const reference_iterator&)     = default;
    reference_iterator            (reference_iterator&&) noexcept = default;
    reference_iterator& operator= (const reference_iterator&)     = default;
    reference_iterator& operator= (reference_iterator&&) noexcept = default;
    ~reference_iterator           (void)                          = default;
  
    constexpr explicit reference_iterator (reference ref)
      : m_ptr    (std::addressof (ref)),
        m_is_end (false)
    { }
  
    // convert from value_iterator to const_value_iterator
    template<typename NonConst,
             typename = std::enable_if_t<std::is_same<
               std::remove_cv_t<Value>, NonConst>::value>>
    constexpr
    reference_iterator (const reference_iterator<NonConst>& it) noexcept
      : m_ptr (it.m_ptr),
        m_is_end (it.m_is_end)
    { }
  
    constexpr reference_iterator& operator++ (void) noexcept
    {
      m_is_end = true;
      return *this;
    }
  
    constexpr reference_iterator operator++ (int) noexcept
    {
      reference_iterator save = *this;
      ++*this;
      return save;
    }
  
    constexpr reference_iterator& operator-- (void) noexcept
    {
      m_is_end = false;
      return *this;
    }
  
    constexpr reference_iterator operator-- (int) noexcept
    {
      reference_iterator save = *this;
      --*this;
      return save;
    }
  
    constexpr reference_iterator& operator+= (difference_type n) noexcept
    {
      if (n > 0)
        return ++*this;
      else if (n < 0)
        return --*this;
      return *this;
    }
  
    [[nodiscard]]
    constexpr reference_iterator operator+ (difference_type n) const noexcept
    {
      return reference_iterator (*this) += n;
    }
  
    constexpr reference_iterator& operator-= (difference_type n) noexcept
    {
      return operator+= (-n);
    }
  
    [[nodiscard]]
    constexpr reference_iterator operator- (difference_type n) const noexcept
    {
      return operator+ (-n);
    }
  
    [[nodiscard]]
    constexpr difference_type
    operator- (const reference_iterator& other) const noexcept
    {
      return m_is_end - other.m_is_end;
    }
  
    [[nodiscard]]
    constexpr reference operator[] (difference_type n) const
    {
      return *(operator+ (n));
    }
  
    //
    // The below functions will be wrong if other does not refer to the same 
    // object. However, this should be fine because it works the same way
    // for std container iterators.
    //
  
    [[nodiscard]]
    constexpr bool operator< (const reference_iterator& other) const noexcept
    {
      return ! m_is_end && other.m_is_end;
    }
  
    [[nodiscard]]
    constexpr bool operator> (const reference_iterator& other) const noexcept
    {
      return m_is_end && ! other.m_is_end;
    }
  
    [[nodiscard]]
    constexpr bool operator<= (const reference_iterator& other) const noexcept
    {
      return m_is_end ? other.m_is_end : true;
    }
  
    [[nodiscard]]
    constexpr bool operator>= (const reference_iterator& other) const noexcept
    {
      return m_is_end ? true : ! other.m_is_end;
    }
  
    [[nodiscard]]
    constexpr bool operator== (const reference_iterator& other) const noexcept
    {
      return (*m_ptr == *other.m_ptr) && (m_is_end == other.m_is_end);
    }
  
    [[nodiscard]]
    constexpr bool operator!= (const reference_iterator& other) const noexcept
    {
      return ! operator== (other);
    }
  
    [[nodiscard]]
    constexpr reference operator* (void) const
    {
      return *get_pointer ();
    }
  
    [[nodiscard]]
    constexpr pointer operator-> (void) const
    {
      return get_pointer ();
    }

  private:
    
    [[nodiscard]]
    constexpr bool is_end (void) const noexcept 
    {
      return m_is_end;
    }
  
    [[nodiscard]]
    constexpr pointer get_pointer (void) const noexcept
    {
      return m_ptr;
    }
  
    pointer m_ptr    = nullptr;
    bool    m_is_end = true;
  
  };
  
  template <typename Value> [[nodiscard]]
  constexpr reference_iterator<Value>
  operator+ (typename reference_iterator<Value>::difference_type n,
             const reference_iterator<Value>& it) noexcept
  {
    return it + n;
  }

//  template <typename Value>
//  using value_iterator = value_iterator_impl<Value *>;
  
  template <typename Value>
  using const_reference_iterator = reference_iterator<const Value>;
  
  template <typename Value>
  using reverse_reference_iterator
    = std::reverse_iterator<reference_iterator<Value>>;
  
  template <typename Value>
  using const_reverse_reference_iterator
    = std::reverse_iterator<const_reference_iterator<Value>>;
  
  template <typename Value>
  using ref_iter = reference_iterator<Value>;
  
  template <typename Value>
  using ref_citer = const_reference_iterator<Value>;
  
  template <typename Value>
  using ref_riter = reverse_reference_iterator<Value>;
  
  template <typename Value>
  using ref_criter = const_reverse_reference_iterator<Value>;
  
  template <typename Value>
  class value_iterator
  {
  public:
    
    using difference_type   = int8_t;
    using value_type        = std::remove_cv_t<Value>;
    using pointer           = Value *;
    using reference         = Value&;
    using iterator_category = std::random_access_iterator_tag;
    
    struct dereference_exception : std::exception
    {
      dereference_exception (void) = default;
      
      explicit dereference_exception (const char *str)
        : m_str (str)
      { }
      
      [[nodiscard]]
      const char* what (void) const noexcept override
      {
        return m_str;
      }
    private:
      const char* m_str = "Tried to dereference an incremented iterator.";
    };
    
    friend value_iterator<std::add_const_t<Value>>;
    friend value_iterator<std::add_volatile_t<Value>>;
    friend value_iterator<std::add_cv_t<Value>>;
    
    value_iterator            (void)                      = default;
    value_iterator            (const value_iterator&)     = default;
    value_iterator            (value_iterator&&) noexcept = default;
    value_iterator& operator= (const value_iterator&)     = default;
    value_iterator& operator= (value_iterator&&) noexcept = default;
    ~value_iterator           (void)                      = default;
    
    template <typename ...Args,
              typename = std::enable_if_t<
                std::is_constructible<value_type, Args...>::value>>
    constexpr explicit value_iterator (Args&&... args)
      : m_value (std::forward<Args> (args)...),
        m_is_end (false)
    { }
    
    // convert from value_iterator to const_value_iterator
    template<typename NonConst,
             typename = std::enable_if_t<
               std::is_same<std::remove_const_t<Value>, NonConst>::value>>
    constexpr value_iterator (const value_iterator<NonConst>& it) noexcept (
    std::is_nothrow_copy_constructible<value_type>::value)
      : m_value  (it.m_value),
        m_is_end (it.m_is_end)
    { }
    
    // move from value_iterator to const_value_iterator
    template<typename NonConst,
             typename = std::enable_if_t<
               std::is_same<std::remove_const_t<Value>, NonConst>::value>>
    constexpr value_iterator (value_iterator<NonConst>&& it) noexcept (
    std::is_nothrow_move_constructible<value_type>::value)
      : m_value  (std::move (it.m_value)),
        m_is_end (it.m_is_end)
    { }
    
    constexpr value_iterator& operator++ (void) noexcept
    {
      m_is_end = true;
      return *this;
    }
    
    constexpr value_iterator operator++ (int) noexcept
    {
      value_iterator save = *this;
      ++*this;
      return save;
    }
    
    constexpr value_iterator& operator-- (void) noexcept
    {
      m_is_end = false;
      return *this;
    }
    
    constexpr value_iterator operator-- (int) noexcept
    {
      value_iterator save = *this;
      --*this;
      return save;
    }
    
    constexpr value_iterator& operator+= (difference_type n) noexcept
    {
      m_is_end = additive_result (n);
      return *this;
    }
    
    [[nodiscard]]
    constexpr value_iterator operator+ (difference_type n) const noexcept
    {
      return value_iterator(*this) += n;
    }
    
    constexpr value_iterator& operator-= (difference_type n) noexcept
    {
      return operator+= (-n);
    }
    
    [[nodiscard]]
    constexpr value_iterator
    operator- (difference_type n) const noexcept
    {
      return operator+ (-n);
    }
    
    [[nodiscard]]
    constexpr difference_type
    operator- (const value_iterator& other) const noexcept
    {
      return m_is_end - other.m_is_end;
    }
    
    [[nodiscard]]
    constexpr reference operator[] (difference_type n) const
    {
      if (additive_result (n))
        throw dereference_exception ();
      return m_value;
    }
    
    //
    // The below functions will be wrong if other does not refer to the same 
    // object. However, this should be fine because it works the same way
    // for std container iterators.
    //
    
    [[nodiscard]]
    constexpr bool operator< (const value_iterator& other) const noexcept
    {
      return ! m_is_end && other.m_is_end;
    }
    
    [[nodiscard]]
    constexpr bool operator> (const value_iterator& other) const noexcept
    {
      return m_is_end && ! other.m_is_end;
    }
    
    [[nodiscard]]
    constexpr bool operator<= (const value_iterator& other) const noexcept
    {
      return m_is_end ? other.m_is_end : true;
    }
    
    [[nodiscard]]
    constexpr bool operator>= (const value_iterator& other) const noexcept
    {
      return m_is_end ? true : ! other.m_is_end;
    }
    
    [[nodiscard]]
    constexpr bool operator== (const value_iterator& other) const noexcept
    {
      return (m_value == other.m_value) && (m_is_end == other.m_is_end);
    }
    
    [[nodiscard]]
    constexpr bool operator!= (const value_iterator& other) const noexcept
    {
      return ! operator== (other);
    }
    
    [[nodiscard]]
    constexpr reference operator* (void) const
    {
      if (m_is_end)
        throw dereference_exception ();
      return m_value;
    }
    
    [[nodiscard]]
    constexpr pointer operator-> (void) const
    {
      return m_is_end ? nullptr : &m_value;
    }
  
  private:
    
    [[nodiscard]]
    constexpr bool additive_result (difference_type n) const noexcept
    {
      return n == 0 ? m_is_end : n > 0;
    }
    
    mutable value_type m_value  = value_type ();
    bool               m_is_end = true;
    
  };
  
  template <typename Value> [[nodiscard]]
  constexpr value_iterator<Value>
  operator+ (typename value_iterator<Value>::difference_type n,
             const value_iterator<Value>& it) noexcept
  {
    return it + n;
  }
  
  template <typename Value>
  using const_value_iterator = value_iterator<const Value>;
  
  template <typename Value>
  using reverse_value_iterator
  = std::reverse_iterator<value_iterator<Value>>;
  
  template <typename Value>
  using const_reverse_value_iterator
  = std::reverse_iterator<const_value_iterator<Value>>;
  
  template <typename Value>
  using value_iter   = value_iterator<Value>;
  
  template <typename Value>
  using value_citer  = const_value_iterator<Value>;
  
  template <typename Value>
  using value_riter  = reverse_value_iterator<Value>;
  
  template <typename Value>
  using value_criter = const_reverse_value_iterator<Value>;
  
  template <typename T> [[nodiscard]]
  value_iter<T> value_begin (T&& val)
  {
    return value_iter<T> (std::forward<T> (val));
  }

  template <typename T> [[nodiscard]]
  value_iter<T> value_end (T&& val)
  {
    return ++value_begin (std::forward<T> (val));
  }

  template <class... Functors>
  struct overload : Functors...
  {
    using Functors::operator()...;
  };

  template <class... Functors> 
  overload (Functors...) -> overload<Functors...>;
  
  template <typename ...Its>
  class variant_iterator
  {
    static_assert (std::conjunction<is_iterator<Its>...>::value,
                   "All template arguments must be iterators.");
    
    template <typename It>
    using diff_t = typename std::iterator_traits<It>::difference_type;
    
    template <typename It>
    using value_t = typename std::iterator_traits<It>::value_type;
    
    template <typename It>
    using pointer_t = typename std::iterator_traits<It>::pointer;
    
    template <typename It>
    using reference_t = typename std::iterator_traits<It>::reference;
    
    template <typename It>
    using category_t = typename std::iterator_traits<It>::iterator_category;
  
  public:
    
    template <std::size_t I = 0>
    using iterator_type = select_t<I, Its...>;
    
    static_assert (all_same<value_t<Its>...>::value,
                   "value types must be equal.");
    
    static_assert (all_same<reference_t<Its>...>::value,
                   "reference types must be equal.");
    
    static_assert (all_same<pointer_t<Its>...>::value,
                   "pointer types must be equal.");
    
    using difference_type   = std::common_type_t<diff_t<Its>...>;
    using value_type        = value_t<iterator_type<>>;
    using pointer           = pointer_t<iterator_type<>>;
    using const_pointer     = const value_type *;
    using reference         = reference_t<iterator_type<>>;
    using const_reference   = const value_type&;
    using iterator_category = std::common_type_t<category_t<Its>...>;
    
    struct type_exception : std::exception
    {
      type_exception (void) = default;
      
      // explicit type_exception (const char *str)
      //   : m_str (str)
      // { }
      
      [[nodiscard]]
      const char* what (void) const noexcept override
      {
        return m_str;
      }
    private:
      const char* m_str = "Iterator types are not the same.";
    };
  
  private:
    
    struct deduced_tag;
    
    template <template<typename...> class BinaryOperator,
                                    typename Return = deduced_tag>
    struct binary_visitor : BinaryOperator<>
    {
      template <typename It>
      constexpr Return operator() (It&& lhs, It&& rhs) const
      {
        return Return (BinaryOperator<>::operator() (std::forward<It> (lhs),
                                                     std::forward<It> (rhs)));
      }
      
      template <typename T, typename U,
                typename = std::enable_if_t<! std::is_same<T, U>::value>>
      constexpr Return operator() (T&&, U&&) { throw type_exception (); }
    };
    
    template <template<typename...> class BinaryOperator>
    struct binary_visitor<BinaryOperator, deduced_tag> : BinaryOperator<>
    {
      using BinaryOperator<>::operator();
      using return_type = std::invoke_result_t<BinaryOperator<>,
                                               iterator_type<>,
                                               iterator_type<>>;
      
      static_assert (all_same<std::invoke_result_t<BinaryOperator<void>,
                                                   Its, Its>...>::value,
                     "Binary operator return types must be uniform.");
      
      template <typename T, typename U,
                typename = std::enable_if_t<! std::is_same<T, U>::value>>
      constexpr return_type operator() (T&&, U&&) { throw type_exception (); }
    };
  
  public:
    
    variant_iterator            (void)                        = default;
    variant_iterator            (const variant_iterator&)     = default;
    variant_iterator            (variant_iterator&&) noexcept = default;
    variant_iterator& operator= (const variant_iterator&)     = default;
    variant_iterator& operator= (variant_iterator&&) noexcept = default;
    ~variant_iterator           (void)                        = default;
    
    // value copying initializer
    template <typename It,
              typename = std::enable_if_t<is_element<It, Its...>::value>>
    constexpr explicit variant_iterator (const It& it)
      : m_variant (it)
    { }
    
    template <typename It,
              typename = std::enable_if_t<is_element<It, Its...>::value>>
    constexpr explicit variant_iterator (It&& it)
      : m_variant (std::forward<It> (it))
    { }
    
    constexpr variant_iterator& operator++ (void) noexcept
    {
      std::visit ([] (auto&& it) -> void { ++it; }, m_variant);
      return *this;
    }
    
    constexpr variant_iterator operator++ (int) noexcept
    {
      return std::visit ([] (auto&& it) -> variant_iterator
                         { return variant_iterator (it++); }, m_variant);
    }
    
    constexpr variant_iterator& operator-- (void) noexcept
    {
      std::visit ([] (auto&& it) -> void { --it; }, m_variant);
      return *this;
    }
    
    constexpr variant_iterator operator-- (int) noexcept
    {
      return std::visit ([] (auto&& it) -> variant_iterator
                         { return variant_iterator (it--); }, m_variant);
    }
    
    constexpr variant_iterator& operator+= (difference_type n) noexcept
    {
      std::visit ([&n] (auto&& it) { it += n; }, m_variant);
      return *this;
    }
    
    [[nodiscard]]
    constexpr variant_iterator operator+ (difference_type n) const noexcept
    {
      return std::visit ([&n] (auto&& it) -> variant_iterator
                         { return variant_iterator (it + n); }, m_variant);
    }
    
    [[nodiscard]]
    friend constexpr variant_iterator operator+ (difference_type n,
                                                 const variant_iterator& var) noexcept
    {
      return std::visit ([&n] (auto&& it) -> variant_iterator
                         { return variant_iterator<Its...> (n + it); },
                         var.m_variant);
    }
    
    constexpr variant_iterator& operator-= (difference_type n) noexcept
    {
      std::visit ([&n] (auto&& it) -> void { it -= n; }, m_variant);
      return *this;
    }
    
    [[nodiscard]]
    constexpr variant_iterator operator- (difference_type n) const noexcept
    {
      return std::visit ([&n] (auto&& it) -> variant_iterator
                         { return variant_iterator (it - n); }, m_variant);
    }
    
    [[nodiscard]]
    constexpr difference_type operator- (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::minus, difference_type> { },
                         m_variant, other.m_variant);
    }
    
    [[nodiscard]]
    constexpr reference operator[] (difference_type n) const
    {
      return std::visit ([&n] (auto&& it) -> reference
                         { return it[n]; },  m_variant);
    }
    
    [[nodiscard]]
    constexpr bool operator< (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::less> { },
                         m_variant, other.m_variant);
    }
    
    [[nodiscard]]
    constexpr bool operator> (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::greater> { },
                         m_variant, other.m_variant);
    }
    
    [[nodiscard]]
    constexpr bool operator<= (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::less_equal> { },
                         m_variant, other.m_variant);
    }
    
    [[nodiscard]]
    constexpr bool operator>= (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::greater_equal> { },
                         m_variant, other.m_variant);
    }
    
    [[nodiscard]]
    constexpr bool operator== (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::equal_to> { },
                         m_variant, other.m_variant);
    }
    
    [[nodiscard]]
    constexpr bool operator!= (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::not_equal_to> { },
                         m_variant, other.m_variant);
    }
    
    [[nodiscard]]
    constexpr reference operator* (void) const
    {
      return std::visit ([] (auto&& it) -> reference
                         { return it.operator* (); }, m_variant);
    }
    
    [[nodiscard]]
    constexpr pointer operator-> (void) const
    {
      return std::visit ([] (auto&& it) -> pointer
                         { return it.operator-> (); }, m_variant);
    }
  
  private:
    
    std::variant<Its...> m_variant;
    
  };

  // abstract
  class ir_component
  {
  public:

    using link_cache_vect = std::vector<ir_basic_block *>;
    using link_cache_iter = link_cache_vect::iterator;
    using link_cache_citer = link_cache_vect::const_iterator;

    constexpr ir_component (void) noexcept = default;

    virtual ~ir_component (void) noexcept = 0;

    using link_iter  
      = variant_iterator<link_cache_iter, value_iter<ir_basic_block *>>;
    
    using link_citer 
      = variant_iterator<link_cache_citer, value_citer<ir_basic_block *>>;
  
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

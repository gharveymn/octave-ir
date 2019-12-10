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
    using value_type        = Value;
    using pointer           = value_type *;
    using reference         = value_type&;
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

    value_iterator
    operator++ (int) noexcept
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

    value_iterator
    operator-- (int) noexcept
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

  template <class... Ts>
  struct overload : Ts...
  {
    using Ts::operator()...;
  };

  template <class... Ts> overload (Ts...) ->overload<Ts...>;

  template <typename It1, typename It2>
  class variant_iterator
  {

    using diff_type1  = typename It1::difference_type;
    using value_type1 = typename It1::value_type;
    using pointer1    = typename It1::pointer;
    using reference1  = typename It1::reference;
    using iter_cat1   = typename It1::iterator_category;

    using diff_type2  = typename It2::difference_type;
    using value_type2 = typename It2::value_type;
    using pointer2    = typename It2::pointer;
    using reference2  = typename It2::reference;
    using iter_cat2   = typename It2::iterator_category;

  public:

    static_assert (std::is_same<value_type1, value_type2>::value,
                   "value types must be equal.");

    using difference_type   = std::common_type_t<diff_type1, diff_type2>;
    using value_type        = value_type1;
    using pointer           = pointer1;
    using reference         = reference1;
    using iterator_category = std::common_type_t<iter_cat1, iter_cat2>;

    using type1 = It1;
    using type2 = It2;

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
    struct binary_visitor : BinaryOperator<It1>, BinaryOperator<It2>
    {
      using BinaryOperator<It1>::operator();
      using BinaryOperator<It2>::operator();

      using return_type = std::invoke_result_t<BinaryOperator<It1>, It1, It1>;
      
      static_assert (std::is_same<return_type, 
                  std::invoke_result_t<BinaryOperator<It2>, It2, It2>>::value,
                     "Binary operator return types must be uniform.");
      return_type operator() (It1, It2) { throw type_exception (); }
      return_type operator() (It2, It1) { throw type_exception (); }
    };

  public:

    // DefaultConstructible
    constexpr variant_iterator (void) noexcept = default;

    explicit variant_iterator (const It1& it1)
      : m_variant (it1)
    { }

    explicit variant_iterator (It1&& it1)
      : m_variant (std::move (it1))
    { }

    explicit variant_iterator (const It2& it2)
      : m_variant (it2)
    { }

    explicit variant_iterator (It2&& it2)
      : m_variant (std::move (it2))
    { }

    variant_iterator (const variant_iterator& other)
      : m_variant (other.m_variant)
    { }

    variant_iterator (variant_iterator&& other) noexcept
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

    variant_iterator operator+ (difference_type n) const noexcept
    {
      return std::visit ([&n] (auto&& it) // -> variant_iterator
                         { return variant_iterator (it + n); }, m_variant);
    }

    friend variant_iterator operator+ (difference_type n,
                                       const variant_iterator& var)
    {
      return std::visit ([&n] (auto&& it) // -> variant_iterator
                         { return variant_iterator<It1, It2> (n + it); }, var.m_variant);
    }

    variant_iterator& operator-= (difference_type n) noexcept
    {
      std::visit ([&n] (auto&& it) { it -= n; }, m_variant);
      return *this;
    }

    variant_iterator operator- (difference_type n) const noexcept
    {
      return std::visit ([&n] (auto&& it) // -> variant_iterator
                         { return variant_iterator (it - n); }, m_variant);
    }

  public:

    difference_type operator- (const variant_iterator& other) const
    {
      using diff_type = difference_type;
      return std::visit (overload { 
          [] (It1 lhs, It1 rhs) -> diff_type { return lhs - rhs; },
          [] (It2 lhs, It2 rhs) -> diff_type { return lhs - rhs; },
          [] (auto lhs, auto rhs) -> diff_type { throw type_exception (); } },
        m_variant, other.m_variant);
    }

    reference operator[] (difference_type n) const noexcept
    {
      return std::visit ([&n] (auto&& it) -> reference
                         { return it[n]; }, m_variant);
    }

    bool operator< (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::less> { },
                         m_variant, other.m_variant);
    }

    bool operator> (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::greater> { },
                         m_variant, other.m_variant);
    }

    bool operator<= (const variant_iterator& other) const
    {
      return std::visit (binary_visitor<std::less_equal> { },
                         m_variant, other.m_variant);
    }

    bool operator>= (const variant_iterator& other) const
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

    reference operator* (void) const noexcept
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
    noexcept (std::is_nothrow_swappable<std::variant<It1, It2>>::value)
    {
      using std::swap;
      swap (this->m_variant, other.m_variant);
    }

  private:

    std::variant<It1, It2> m_variant;

  };

  template <typename It1, typename It2>
  inline std::enable_if_t<std::is_swappable<std::variant<It1, It2>>::value>
  swap(variant_iterator<It1, It2>& lhs, variant_iterator<It1, It2>& rhs)
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
    
    template <typename T>
    class ptr_iterator;
    
    // basically just an iterator-compatible wrapper for a pointer
    // to avoid UB of incrementing a non-associated pointer.
    template <typename T>
    class ptr_iterator<T *>
    {
    public:
      
      using difference_type   = std::int64_t;
      using value_type        = T *;
      using pointer           = value_type *;
      using reference         = value_type&;
      using iterator_category = std::random_access_iterator_tag;

      constexpr explicit ptr_iterator (value_type ptr)
        : m_ptr (ptr)
      { }
      
//      constexpr ptr_iterator (const ptr_iterator& other)
//        : m_ptr (other.m_ptr)
//      { }
//
//      constexpr ptr_iterator (ptr_iterator&& other) noexcept
//        : m_ptr (other.m_ptr)
//      { }
//
//      constexpr ptr_iterator& operator= (const ptr_iterator& other)
//      { 
//        m_ptr = other.m_ptr;
//        return *this;
//      }

//      constexpr ptr_iterator& operator= (ptr_iterator&& other) noexcept
//      {
//        m_ptr = other.m_ptr;
//        return *this;
//      }

      ptr_iterator& operator++ (void) noexcept 
      {
        ++m_inc;
        return *this;
      }

      ptr_iterator
      operator++ (int) noexcept
      {
        ptr_iterator save = *this;
        ++*this;
        return save;
      }

      ptr_iterator& operator-- (void) noexcept
      {
        --m_inc;
        return *this;
      }

      ptr_iterator
      operator-- (int) noexcept
      {
        ptr_iterator save = *this;
        --*this;
        return save;
      }

      ptr_iterator& operator+= (difference_type n) noexcept 
      {
        m_inc += n;
        return *this;
      }

      constexpr ptr_iterator
      operator+ (difference_type n) const noexcept
      {
        return { m_ptr, m_inc + n };
      }

      friend constexpr ptr_iterator
      operator+ (difference_type n, ptr_iterator it) noexcept
      {
        return { it.m_ptr, it.m_inc + n };
      }

      constexpr ptr_iterator& operator= (difference_type n) noexcept
      {
        m_inc -= n;
        return *this;
      }

      constexpr ptr_iterator
      operator- (difference_type n) const noexcept
      {
        return { m_ptr, m_inc - n };
      }

      difference_type operator- (const ptr_iterator& other) const noexcept
      {
        return m_inc - other.m_inc;
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
      
      constexpr bool operator< (const ptr_iterator& other) const noexcept
      {
        return m_inc < other.m_inc;
      }

      constexpr bool operator> (const ptr_iterator& other) const noexcept
      {
        return m_inc > other.m_inc;
      }

      constexpr bool operator<= (const ptr_iterator& other) const noexcept
      {
        return m_inc <= other.m_inc;
      }

      constexpr bool operator>= (const ptr_iterator& other) const noexcept
      {
        return m_inc >= other.m_inc;
      }

      constexpr bool operator== (const ptr_iterator& other) const noexcept
      {
        return m_inc == other.m_inc;
      }

      constexpr bool operator!= (const ptr_iterator& other) const noexcept
      {
        return ! operator== (other);
      }

      constexpr reference operator* (void) noexcept
      {
        return is_incremented ? nullptr : m_ptr;
      }

      constexpr pointer operator-> (void) noexcept
      {
        return is_incremented ? pointer (nullptr) : &m_ptr;
      }
      
      void swap (ptr_iterator& other)
      {
        using std::swap;
        swap (m_ptr, other.m_ptr);
        swap (m_inc, other.m_inc);
      }
      
    private:
      
//      constexpr ptr_iterator (value_type ptr, int inc)
//        : m_ptr (ptr),
//          m_inc (inc)
//      { }

      [[nodiscard]]
      constexpr bool is_incremented (void) const noexcept 
      {
        return m_inc != 0;
      }
      
      value_type   m_ptr = nullptr;
      std::int32_t m_inc = 0;
      
    };
    
    template <typename T>
    inline void 
    swap (ptr_iterator<T *>& it1, ptr_iterator<T *>& it2)
    {
      it1.swap (it2);
    }

    static_assert (
      std::is_trivially_copy_constructible<ptr_iterator<void *>>::value, "");
    
    static_assert (
      std::is_trivially_move_constructible<ptr_iterator<void *>>::value, "");
    
    static_assert (
      std::is_trivially_copy_assignable<ptr_iterator<void *>>::value, "");
    
    static_assert (
      std::is_trivially_move_assignable<ptr_iterator<void *>>::value, "");
    
    static_assert (
      std::is_trivially_destructible<ptr_iterator<void *>>::value, "");

    template <typename It, typename E = void>
    class union_iterator;

    // restrictions make sure T is a pointer, the value held by It is the same
    // as T, and It is a random access iterator. A concept would be good here.
    template <typename It>
    class union_iterator<It,
                         std::enable_if_t<
                            std::conjunction<
                              std::is_pointer<typename It::value_type>
                            >::value
                          >
                        >
    {
      
    public:

      using iter_type         = It;
      using difference_type   = typename iter_type::difference_type;
      using value_type        = typename iter_type::value_type;
      using pointer           = typename iter_type::pointer;
      using reference         = typename iter_type::reference;
      using iterator_category = typename iter_type::iterator_category;
      
    private:

      template <typename Iter>
      using enable_if_random_access_iter = std::enable_if_t<std::is_base_of<
        std::random_access_iterator_tag,
        typename Iter::iterator_category>::value>;

      template <typename Iter>
      using enable_if_bidirect_iter = std::enable_if_t<std::is_base_of<
        std::bidirectional_iterator_tag,
        typename Iter::iterator_category>::value>;

      template <typename Iter>
      using enable_if_forward_iter = std::enable_if_t<std::is_base_of<
        std::forward_iterator_tag,
        typename Iter::iterator_category>::value>;

      template <typename Iter>
      using enable_if_input_iter = std::enable_if_t<std::is_base_of<
        std::input_iterator_tag,
        typename Iter::iterator_category>::value>;      

      struct is_swappable 
        : std::conjunction<std::is_swappable<iter_type>, 
                           std::is_swappable<value_type>>
      { };

      // DefaultConstructible
      constexpr union_iterator (void) noexcept
        : m_value (nullptr),
          m_type (tag::value)
      { }

      explicit union_iterator (const iter_type& it) noexcept
        : m_iter (it),
          m_type (tag::iterator)
      { }
  
      explicit union_iterator (iter_type&& it) noexcept
        : m_iter (std::move (it)),
          m_type (tag::iterator)
      { }

      explicit constexpr union_iterator (const value_type& val) noexcept
        : m_value (val),
          m_type (tag::value)
      { }
  
      explicit constexpr union_iterator (value_type&& val) noexcept
        : m_value (std::move (val)),
          m_type (tag::value)
      { }

      explicit constexpr union_iterator (std::nullptr_t) noexcept
        : m_value (nullptr),
          m_type (tag::value)
      { }
      
      // CopyConstructible
      union_iterator (const union_iterator& other)
        : m_type (other.m_type)
      {
        if (other.is_iterator ())
          this->m_iter = other.m_iter;
        else
          this->m_value = other.m_value;
      }
  
      union_iterator (union_iterator&& other) noexcept
        : m_type (other.m_type)
      {
        if (other.is_iterator ())
          this->m_iter = std::move (other.m_iter);
        else
          this->m_value = std::move (other.m_value);
      }
      
      // CopyAssignable
      // ref-qualified to prevent assignment to rvalues
      union_iterator& operator= (const union_iterator& o) &
      {
        if (&o != this)
          {
            this->m_type = o.m_type;
            if (o.is_iterator ())
              this->m_iter = o.m_iter;
            else
              this->m_value = o.m_value;
          }
        return *this;
      }

      // MoveAssignable
      // ref-qualified to prevent assignment to rvalues
      union_iterator& operator= (union_iterator&& o) & noexcept
      {
        this->m_type = o.m_type;
        if (o.is_iterator ())
          this->m_iter = std::move (o.m_iter);
        else
          this->m_value = std::move (o.m_value);
        return *this;
      }

      // Destructible
      ~union_iterator (void) noexcept
      {
        if (is_iterator ())
          m_iter.~iter_type ();
        else
          m_value.~value_type ();
      }

      union_iterator& operator++ (void) noexcept
      {
        if (is_iterator ())
          ++m_iter;
        else
          ++m_value;
        return *this;
      }

//      template <enable_if_input_iter<iter_type>* = nullptr>
      union_iterator operator++ (int) noexcept
      {
        union_iterator save = *this;
        ++*this;
        return save;
      }

//      template <enable_if_bidirect_iter<iter_type>* = nullptr>
      union_iterator& operator-- (void) noexcept
      {
        if (is_iterator ())
          --m_iter;
        else
          --m_value;
        return *this;
      }
      
//      template <enable_if_bidirect_iter<iter_type>* = nullptr>
      union_iterator operator-- (int) noexcept
      {
        union_iterator save = *this;
        --*this;
        return save;
      }

//      template <enable_if_random_access_iter<iter_type>* = nullptr>
      union_iterator& operator+= (difference_type n) noexcept
      {
        if (is_iterator ())
          m_iter += n;
        else
          m_value += n;
        return *this;
      }

//      template <enable_if_random_access_iter<iter_type>* = nullptr>
      union_iterator operator+ (difference_type n) const noexcept
      {
        return is_iterator () ? union_iterator (m_iter + n)
                              : union_iterator (m_value + n);
      }

//      template <enable_if_random_access_iter<iter_type>* = nullptr>
      friend union_iterator operator+ (difference_type n, 
                                       const union_iterator& o)
      {
        return o + n;
      }

//      template <enable_if_random_access_iter<iter_type>* = nullptr>
      union_iterator& operator-= (difference_type n) noexcept
      {
        if (is_iterator ())
          m_iter -= n;
        else
          m_value -= n;
        return *this;
      }

//      template <enable_if_random_access_iter<iter_type>* = nullptr>
      union_iterator operator- (difference_type n) const noexcept
      {
        return is_iterator () ? union_iterator (m_iter  - n)
                              : union_iterator (m_value - n);
      }

//      template <enable_if_random_access_iter<iter_type>* = nullptr>
      difference_type operator- (const union_iterator& other) const noexcept
      {
        return is_iterator () ? this->m_iter  - other.m_iter
                              : this->m_value - other.m_value;
      }

//      template <enable_if_random_access_iter<iter_type>* = nullptr>
      reference operator[] (difference_type n) const noexcept
      {
        return is_iterator () ? m_iter[n] : m_value[n];
      }

//      template <enable_if_random_access_iter<iter_type>* = nullptr>
      bool operator< (const union_iterator& other) const noexcept
      {
        return is_iterator () ? this->m_iter  < other.m_iter
                              : this->m_value < other.m_value;
      }

//      template <enable_if_random_access_iter<iter_type>* = nullptr>
      bool operator> (const union_iterator& other) const noexcept
      {
        return is_iterator () ? this->m_iter  > other.m_iter
                              : this->m_value > other.m_value;
      }

//      template <enable_if_random_access_iter<iter_type>* = nullptr>
      bool operator<= (const union_iterator& other) const noexcept
      {
        return is_iterator () ? this->m_iter  <= other.m_iter
                              : this->m_value <= other.m_value;
      }

//      template <enable_if_random_access_iter<iter_type>* = nullptr>
      bool operator>= (const union_iterator& other) const noexcept
      {
        return is_iterator () ? this->m_iter  >= other.m_iter
                              : this->m_value >= other.m_value;
      }

      // EqualityComparable
      constexpr bool operator== (const union_iterator& other) const noexcept
      {
        return type_equal (other) ? (is_iterator () ? iterator_equal (other) 
                                                : value_equal (other))
                              : false;
      }

//      template <enable_if_input_iter<iter_type>* = nullptr>
      constexpr bool operator!= (const union_iterator& other) const noexcept
      {
        return ! operator== (other);
      }

      reference operator* (void) noexcept
      {
        return is_iterator () ? *m_iter : m_value;
      }

      constexpr reference operator* (void) const noexcept
      {
        return is_iterator () ? *m_iter : m_value;
      }

//      template <enable_if_input_iter<iter_type>* = nullptr>
      constexpr pointer operator-> (void) const noexcept
      {
        return is_iterator () ? m_iter.operator-> () : &m_value;
      }
      
      void swap (union_iterator& other) 
        noexcept (std::is_nothrow_swappable<iter_type>::value 
                  && std::is_nothrow_swappable<value_type>::value)
      {
        using std::swap;
        if (is_iterator ())
          swap (this->m_iter, other.m_iter);
        else
          swap (this->m_value, other.m_value);
      }

      // Swappable
      friend void swap (union_iterator& it1, union_iterator& it2)
      {
        it1.swap (it2);
      }

    private:

      constexpr bool type_equal (const union_iterator& o) const noexcept
      {
        return this->m_type != o.m_type;
      }

      bool iterator_equal (const union_iterator& o) const noexcept
      {
        return this->m_iter == o.m_iter;
      }

      constexpr bool value_equal (const union_iterator& o) const noexcept
      {
        return this->m_value == o.m_value;
      }

      constexpr bool is_iterator (void) const noexcept
      {
        return m_type == tag::iterator;
      }

      constexpr bool is_value (void) const noexcept
      {
        return m_type == tag::value;
      }

      union
      {
        iter_type m_iter;
        value_type m_value;
      };

      const enum class tag
      {
        iterator,
        value
      } m_type;
    };

    template <typename It>
    inline std::enable_if_t<union_iterator<It>::is_swappable::value> 
    swap (union_iterator<It>& it1, union_iterator<It>& it2) noexcept (
      noexcept (it1.swap (it2)))
    {
      it1.swap (it2);
    }

    using link_iter  = union_iterator<link_cache_iter>;
    using link_citer = union_iterator<link_cache_citer>;
  
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

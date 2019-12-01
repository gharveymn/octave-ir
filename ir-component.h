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

#include <vector>

namespace octave
{

  class ir_basic_block;
  class ir_function;

  // abstract
  class ir_component
  {
  public:

    using link_cache_vec = std::vector<ir_basic_block *>;
    using link_cache_iter = link_cache_vec::iterator;
    using link_cache_citer = link_cache_vec::const_iterator;

    constexpr ir_component (void) noexcept = default;

    virtual ~ir_component (void) noexcept = 0;

    template <typename It, typename E = void>
    class union_iterator;

    // restrictions make sure T is a pointer, the value held by It is the same
    // as T, and It is a random access iterator. A concept would be good here.
    template <typename It>
    class union_iterator<It, 
                          enable_if_t<
                            conjunction<
                              std::is_pointer<typename It::value_type>,
                              std::is_same<typename It::iterator_category, 
                                           std::random_access_iterator_tag>
                            >::value
                          >
                        >
    {

      template <typename Iter>
      using enable_if_random_access_iter = enable_if_t<std::is_base_of<
        std::random_access_iterator_tag,
        typename Iter::iterator_category>::value>;

      template <typename Iter>
      using enable_if_bidirect_iter = enable_if_t<std::is_base_of<
        std::bidirectional_iterator_tag,
        typename Iter::iterator_category>::value>;

      template <typename Iter>
      using enable_if_forward_iter = enable_if_t<std::is_base_of<
        std::forward_iterator_tag,
        typename Iter::iterator_category>::value>;

      template <typename Iter>
      using enable_if_input_iter = enable_if_t<std::is_base_of<
        std::input_iterator_tag,
        typename Iter::iterator_category>::value>;
      
    public:
      using iter_type         = It;
      using difference_type   = typename iter_type::difference_type;
      using value_type        = typename iter_type::value_type;
      using pointer           = typename iter_type::pointer;
      using reference         = typename iter_type::reference;
      using iterator_category = typename iter_type::iterator_category;
      
    public:

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
      union_iterator (const union_iterator& o)
        : m_type (o.m_type)
      {
        if (o.is_iterator ())
          this->m_iter = o.m_iter;
        else
          this->m_value = o.m_value;
      }
  
      union_iterator (union_iterator&& o) noexcept
        : m_type (o.m_type)
      {
        if (o.is_iterator ())
          this->m_iter = std::move (o.m_iter);
        else
          this->m_value = std::move (o.m_value);
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

      template <enable_if_input_iter<iter_type>* = nullptr>
      union_iterator operator++ (int) noexcept
      {
        union_iterator save = *this;
        ++*this;
        return save;
      }

      template <enable_if_bidirect_iter<iter_type>* = nullptr>
      union_iterator& operator-- (void) noexcept
      {
        if (is_iterator ())
          --m_iter;
        else
          --m_value;
        return *this;
      }
      
      template <enable_if_bidirect_iter<iter_type>* = nullptr>
      union_iterator operator-- (int) noexcept
      {
        union_iterator save = *this;
        --*this;
        return save;
      }

      template <enable_if_random_access_iter<iter_type>* = nullptr>
      union_iterator& operator+= (difference_type n) noexcept
      {
        if (is_iterator ())
          m_iter += n;
        else
          m_value += n;
        return *this;
      }

      template <enable_if_random_access_iter<iter_type>* = nullptr>
      union_iterator operator+ (difference_type n) const noexcept
      {
        return is_iterator () ? union_iterator (m_iter + n)
                              : union_iterator (m_value + n);
      }

      template <enable_if_random_access_iter<iter_type>* = nullptr>
      friend union_iterator operator+ (difference_type n, 
                                       const union_iterator& o)
      {
        return o + n;
      }

      template <enable_if_random_access_iter<iter_type>* = nullptr>
      union_iterator& operator-= (difference_type n) noexcept
      {
        if (is_iterator ())
          m_iter -= n;
        else
          m_value -= n;
        return *this;
      }

      template <enable_if_random_access_iter<iter_type>* = nullptr>
      union_iterator operator- (difference_type n) const noexcept
      {
        return is_iterator () ? union_iterator (m_iter  - n)
                              : union_iterator (m_value - n);
      }

      template <enable_if_random_access_iter<iter_type>* = nullptr>
      difference_type operator- (const union_iterator& o) const noexcept
      {
        return is_iterator () ? this->m_iter  - o.m_iter
                              : this->m_value - o.m_value;
      }

      template <enable_if_random_access_iter<iter_type>* = nullptr>
      reference operator[] (difference_type n) const noexcept
      {
        return is_iterator () ? m_iter[n] : m_value[n];
      }

      template <enable_if_random_access_iter<iter_type>* = nullptr>
      bool operator< (const union_iterator& o) const noexcept
      {
        return is_iterator () ? this->m_iter  < o.m_iter
                              : this->m_value < o.m_value;
      }

      template <enable_if_random_access_iter<iter_type>* = nullptr>
      bool operator> (const union_iterator& o) const noexcept
      {
        return is_iterator () ? this->m_iter  > o.m_iter
                              : this->m_value > o.m_value;
      }

      template <enable_if_random_access_iter<iter_type>* = nullptr>
      bool operator<= (const union_iterator& o) const noexcept
      {
        return is_iterator () ? this->m_iter  <= o.m_iter
                              : this->m_value <= o.m_value;
      }

      template <enable_if_random_access_iter<iter_type>* = nullptr>
      bool operator>= (const union_iterator& o) const noexcept
      {
        return is_iterator () ? this->m_iter  >= o.m_iter
                              : this->m_value >= o.m_value;
      }

      // EqualityComparable
      constexpr bool operator== (const union_iterator& o) const noexcept
      {
        return type_equal (o) ? (is_iterator () ? iterator_equal (o) 
                                                : value_equal (o))
                              : false;
      }

      template <enable_if_input_iter<iter_type>* = nullptr>
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

      template <enable_if_input_iter<iter_type>* = nullptr>
      constexpr pointer operator-> (void) const noexcept
      {
        return is_iterator () ? m_iter.operator-> () : &m_value;
      }

      void swap (union_iterator& o)
      {
        if (is_iterator ())
          std::swap (this->m_iter, o.m_iter);
        else
          std::swap (this->m_value, o.m_value);
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

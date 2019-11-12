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
  class ir_module;

  // abstract
  class ir_component
  {
  public:

    using link_cache_vec = std::vector<ir_basic_block *>;
    using link_cache_iter = link_cache_vec::iterator;
    using link_cache_citer = link_cache_vec::const_iterator;

    ir_component (ir_module& mod)
      : m_module (mod)
    { }

    virtual ~ir_component (void) noexcept = 0;

    constexpr ir_module& get_module (void) const noexcept
    {
      return m_module;
    }

    template <typename It, typename E = void>
    class union_iterator;

    template <typename It>
    class union_iterator<It, enable_if_t<std::is_pointer<typename It::value_type>::value>>
    {
      using iter = It;
    public:

      using difference_type   = typename iter::difference_type;
      using value_type        = typename iter::value_type;
      using pointer           = typename iter::pointer;
      using reference         = typename iter::reference;
      using iterator_category = typename iter::iterator_category;

      explicit union_iterator (iter it) noexcept
        : m_iter (it),
          m_type (tag::iterator)
      { }

      explicit constexpr union_iterator (value_type val) noexcept
        : m_value (std::move (val)),
          m_type (tag::value)
      { }

      explicit constexpr union_iterator (std::nullptr_t) noexcept
        : m_value (nullptr),
          m_type (tag::value)
      { }

      union_iterator (const union_iterator& o)
        : m_type (o.m_type)
      {
        if (o.is_iterator ())
          this->m_iter = o.m_iter;
        else
          this->m_value = o.m_value;
      }


      union_iterator& operator= (const union_iterator& o)
      {
        this->m_type = o.m_type;
        if (o.is_iterator ())
          this->m_iter = o.m_iter;
        else
          this->m_value = o.m_value;
      }

      union_iterator (union_iterator&& o) noexcept = default;
      union_iterator& operator= (union_iterator&&) noexcept = default;

      ~union_iterator (void) noexcept
      {
        if (is_iterator ())
          m_iter.~iter ();
      }

      union_iterator& operator++ (void) noexcept
      {
        if (is_iterator ())
          ++m_iter;
        else
          ++m_value;
        return *this;
      }

      union_iterator
      operator++ (int) noexcept
      {
        union_iterator save = *this;
        ++*this;
        return save;
      }

      union_iterator& operator-- (void) noexcept
      {
        if (is_iterator ())
          --m_iter;
        else
          --m_value;
        return *this;
      }

      union_iterator
      operator-- (int) noexcept
      {
        union_iterator save = *this;
        --*this;
        return save;
      }

      union_iterator& operator+= (difference_type n) noexcept
      {
        if (is_iterator ())
          m_iter += n;
        else
          m_value += n;
        return *this;
      }

      union_iterator
      operator+ (difference_type n) const noexcept
      {
        return is_iterator () ? union_iterator (m_iter + n)
                              : union_iterator (m_value + n);
      }

      friend union_iterator
      operator+ (difference_type n,
                                      const union_iterator& o)
      {
        return o + n;
      }

      union_iterator& operator-= (difference_type n) noexcept
      {
        if (is_iterator ())
          m_iter -= n;
        else
          m_value -= n;
        return *this;
      }

      union_iterator
      operator- (difference_type n) const noexcept
      {
        return is_iterator () ? union_iterator (m_iter - n)
                              : union_iterator (m_value - n);
      }

      difference_type operator- (const union_iterator& o) const noexcept
      {
        return is_iterator () ? this->m_iter - o.m_iter
                              : this->m_value - o.m_value;
      }

      reference operator[] (difference_type n) const noexcept
      {
        return is_iterator () ? m_iter[n] : m_value[n];
      }

      bool operator< (const union_iterator& o) const noexcept
      {
        return is_iterator () ? this->m_iter < o.m_iter
                              : this->m_value < o.m_value;
      }

      bool operator> (const union_iterator& o) const noexcept
      {
        return is_iterator () ? this->m_iter > o.m_iter
                              : this->m_value > o.m_value;
      }

      bool operator<= (const union_iterator& o) const noexcept
      {
        return is_iterator () ? this->m_iter <= o.m_iter
                              : this->m_value <= o.m_value;
      }

      bool operator>= (const union_iterator& o) const noexcept
      {
        return is_iterator () ? this->m_iter >= o.m_iter
                              : this->m_value >= o.m_value;
      }

      constexpr bool
      operator== (const union_iterator& o) const noexcept
      {
        return type_equal (o)
               ? (is_iterator () ? iterator_equal (o) : value_equal (o))
               : false;
      }

      constexpr bool
      operator!= (const union_iterator& other) const noexcept
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

      friend void swap (union_iterator& it1, union_iterator& it2)
      {
        it1.swap (it2);
      }

    private:

      constexpr bool
      type_equal (const union_iterator& o) const noexcept
      {
        return this->m_type != o.m_type;
      }

      bool
      iterator_equal (const union_iterator& o) const noexcept
      {
        return this->m_iter == o.m_iter;
      }

      constexpr bool
      value_equal (const union_iterator& o) const noexcept
      {
        return this->m_value == o.m_value;
      }

      constexpr bool
      is_iterator (void) const noexcept
      {
        return m_type == tag::iterator;
      }

      constexpr bool
      is_value (void) const noexcept
      {
        return m_type == tag::value;
      }

      union
      {
        iter m_iter;
        value_type m_value;
      };

      const enum class tag
      {
        iterator,
        value
      } m_type;
    };

    using link_iter = union_iterator<link_cache_iter>;
    using link_citer = union_iterator<link_cache_citer>;

    static_assert (std::is_same<link_cache_citer::pointer, ir_basic_block * const *>::value, "");
    static_assert (std::is_same<link_cache_vec::const_pointer, ir_basic_block * const *>::value, "");

    virtual link_iter leaf_begin  (void) = 0;
    virtual link_iter leaf_end    (void) = 0;
    virtual ir_basic_block * get_entry_block (void) = 0;

  private:
    // TODO the module doesn't need to propogate through all ir_components
    //  optimize using virtuals at some point
    ir_module& m_module;

  };

}

#endif
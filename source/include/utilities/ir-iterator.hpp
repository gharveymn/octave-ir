/** ir-iterator.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_ITERATOR_HPP
#define OCTAVE_IR_IR_ITERATOR_HPP

namespace gch
{


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
  set_inserter (Container&) -> set_inserter<Container>;

  template <typename Container>
  class set_emplacer
  {
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
      : m_set { c }
    { }

    template <typename Tuple,
              std::enable_if_t<! std::is_same_v<set_emplacer, std::decay_t<Tuple>>> * = nullptr>
    constexpr
    set_emplacer&
    operator= (Tuple&& t)
    {
      std::apply ([this](auto&&... e) { m_set.emplace (std::forward<decltype (e)> (e)...); },
                  std::forward<Tuple> (t));
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
    nonnull_ptr<Container> m_set;
  };

  template <typename Container>
  set_emplacer (Container&) -> set_emplacer<Container>;

}

#endif // OCTAVE_IR_IR_ITERATOR_HPP

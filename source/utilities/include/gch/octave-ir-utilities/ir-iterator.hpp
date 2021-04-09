/** ir-iterator.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_UTILITIES_IR_ITERATOR_HPP
#define OCTAVE_IR_UTILITIES_IR_ITERATOR_HPP

#include "gch/octave-ir-utilities/ir-common.hpp"
#include "gch/octave-ir-utilities/ir-functional.hpp"

#include <gch/nonnull_ptr.hpp>

#include <cassert>
#include <iterator>
#include <optional>

#if defined (__cpp_lib_concepts) && __cpp_lib_concepts >= 202002L
#  if ! defined (GCH_LIB_CONCEPTS) && ! defined (GCH_DISABLE_CONCEPTS)
#    define GCH_LIB_CONCEPTS
#  endif
#endif

namespace gch
{

  template <typename Iterator>
  struct is_input_iterator
    : std::bool_constant<std::is_base_of_v<
        std::input_iterator_tag,
        typename std::iterator_traits<Iterator>::iterator_category>>
  { };

  template <typename Iterator>
  inline constexpr bool is_input_iterator_v = is_input_iterator<Iterator>::value;

  template <typename Iterator>
  struct is_output_iterator
    : std::bool_constant<std::is_base_of_v<
        std::output_iterator_tag ,
        typename std::iterator_traits<Iterator>::iterator_category>>
  { };

  template <typename Iterator>
  inline constexpr bool is_output_iterator_v = is_output_iterator<Iterator>::value;

  template <typename Iterator>
  struct is_forward_iterator
    : std::bool_constant<std::is_base_of_v<
        std::forward_iterator_tag ,
        typename std::iterator_traits<Iterator>::iterator_category>>
  { };

  template <typename Iterator>
  inline constexpr bool is_forward_iterator_v = is_forward_iterator<Iterator>::value;

  template <typename Iterator>
  struct is_bidirectional_iterator
    : std::bool_constant<std::is_base_of_v<
        std::bidirectional_iterator_tag ,
        typename std::iterator_traits<Iterator>::iterator_category>>
  { };

  template <typename Iterator>
  inline constexpr bool is_bidirectional_iterator_v = is_bidirectional_iterator<Iterator>::value;

  template <typename Iterator>
  struct is_random_access_iterator
    : std::bool_constant<std::is_base_of_v<
        std::random_access_iterator_tag ,
        typename std::iterator_traits<Iterator>::iterator_category>>
  { };

  template <typename Iterator>
  inline constexpr bool is_random_access_iterator_v = is_random_access_iterator<Iterator>::value;

  template <typename Value>
  class basic_contiguous_iterator
  {
  public:
    using difference_type   = typename std::iterator_traits<Value *>::difference_type;
    using value_type        = typename std::iterator_traits<Value *>::value_type;
    using pointer           = typename std::iterator_traits<Value *>::pointer;
    using reference         = typename std::iterator_traits<Value *>::reference;
    using iterator_category = typename std::iterator_traits<Value *>::iterator_category;
#ifdef GCH_LIB_CONCEPTS
    using iterator_concept  = std::contiguous_iterator_tag;
#endif

//  basic_contiguous_iterator            (void)                                 = impl;
    basic_contiguous_iterator            (const basic_contiguous_iterator&)     = default;
    basic_contiguous_iterator            (basic_contiguous_iterator&&) noexcept = default;
    basic_contiguous_iterator& operator= (const basic_contiguous_iterator&)     = default;
    basic_contiguous_iterator& operator= (basic_contiguous_iterator&&) noexcept = default;
    ~basic_contiguous_iterator           (void)                                 = default;

#ifdef NDEBUG
    basic_contiguous_iterator (void) = default;
#else
    constexpr
    basic_contiguous_iterator (void) noexcept
      : m_ptr ()
    { }
#endif

    constexpr explicit
    basic_contiguous_iterator (Value& v) noexcept
      : m_ptr (std::addressof (v))
    { }

    template <typename U,
              std::enable_if_t<std::is_convertible_v<U *, pointer>> * = nullptr>
    constexpr GCH_IMPLICIT_CONVERSION
    basic_contiguous_iterator (const basic_contiguous_iterator<U>& other) noexcept
      : m_ptr (other.base ())
    { }

    constexpr
    basic_contiguous_iterator&
    operator++ (void) noexcept
    {
      ++m_ptr;
      return *this;
    }

    constexpr
    basic_contiguous_iterator
    operator++ (int) noexcept
    {
      return basic_contiguous_iterator (m_ptr++);
    }

    constexpr
    basic_contiguous_iterator&
    operator-- (void) noexcept
    {
      --m_ptr;
      return *this;
    }

    constexpr
    basic_contiguous_iterator
    operator-- (int) noexcept
    {
      return basic_contiguous_iterator (m_ptr--);
    }

    constexpr
    basic_contiguous_iterator&
    operator+= (difference_type n) noexcept
    {
      m_ptr += n;
      return *this;
    }

    constexpr
    basic_contiguous_iterator
    operator+ (difference_type n) const noexcept
    {
      return basic_contiguous_iterator (m_ptr + n);
    }

    constexpr
    basic_contiguous_iterator&
    operator-= (difference_type n) noexcept
    {
      m_ptr -= n;
      return *this;
    }

    constexpr
    basic_contiguous_iterator
    operator- (difference_type n) const noexcept
    {
      return basic_contiguous_iterator (m_ptr - n);
    }

    constexpr
    reference
    operator* (void) const noexcept
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

  private:
    pointer m_ptr;
  };

  template <typename ValueLHS, typename ValueRHS>
  constexpr
  bool
  operator== (const basic_contiguous_iterator<ValueLHS>& lhs,
              const basic_contiguous_iterator<ValueRHS>& rhs) noexcept
  {
    return lhs.base () == rhs.base ();
  }

  template <typename Value>
  constexpr
  bool
  operator== (const basic_contiguous_iterator<Value>& lhs,
              const basic_contiguous_iterator<Value>& rhs) noexcept
  {
    return lhs.base () == rhs.base ();
  }

  template <typename ValueLHS, typename ValueRHS>
  constexpr
  bool
  operator!= (const basic_contiguous_iterator<ValueLHS>& lhs,
              const basic_contiguous_iterator<ValueRHS>& rhs) noexcept
  {
    return lhs.base () != rhs.base ();
  }

  template <typename Value>
  constexpr
  bool
  operator!= (const basic_contiguous_iterator<Value>& lhs,
              const basic_contiguous_iterator<Value>& rhs) noexcept
  {
    return lhs.base () != rhs.base ();
  }

  template <typename ValueLHS, typename ValueRHS>
  constexpr
  bool
  operator< (const basic_contiguous_iterator<ValueLHS>& lhs,
             const basic_contiguous_iterator<ValueRHS>& rhs) noexcept
  {
    return lhs.base () < rhs.base ();
  }

  template <typename Value>
  constexpr
  bool
  operator< (const basic_contiguous_iterator<Value>& lhs,
             const basic_contiguous_iterator<Value>& rhs) noexcept
  {
    return lhs.base () < rhs.base ();
  }

  template <typename ValueLHS, typename ValueRHS>
  constexpr
  bool
  operator> (const basic_contiguous_iterator<ValueLHS>& lhs,
             const basic_contiguous_iterator<ValueRHS>& rhs) noexcept
  {
    return lhs.base () > rhs.base ();
  }

  template <typename Value>
  constexpr
  bool
  operator> (const basic_contiguous_iterator<Value>& lhs,
             const basic_contiguous_iterator<Value>& rhs) noexcept
  {
    return lhs.base () > rhs.base ();
  }

  template <typename ValueLHS, typename ValueRHS>
  constexpr
  bool
  operator<= (const basic_contiguous_iterator<ValueLHS>& lhs,
              const basic_contiguous_iterator<ValueRHS>& rhs) noexcept
  {
    return lhs.base () <= rhs.base ();
  }

  template <typename Value>
  constexpr
  bool
  operator<= (const basic_contiguous_iterator<Value>& lhs,
              const basic_contiguous_iterator<Value>& rhs) noexcept
  {
    return lhs.base () <= rhs.base ();
  }

  template <typename ValueLHS, typename ValueRHS>
  constexpr
  bool
  operator>= (const basic_contiguous_iterator<ValueLHS>& lhs,
              const basic_contiguous_iterator<ValueRHS>& rhs) noexcept
  {
    return lhs.base () >= rhs.base ();
  }

  template <typename Value>
  constexpr
  bool
  operator>= (const basic_contiguous_iterator<Value>& lhs,
              const basic_contiguous_iterator<Value>& rhs) noexcept
  {
    return lhs.base () >= rhs.base ();
  }

  template <typename ValueLHS, typename ValueRHS>
  constexpr
  auto
  operator- (const basic_contiguous_iterator<ValueLHS>& lhs,
             const basic_contiguous_iterator<ValueRHS>& rhs) noexcept
    -> decltype (lhs.base () - rhs.base ())
  {
    return lhs.base () - rhs.base ();
  }

  template <typename Value>
  constexpr
  auto
  operator- (const basic_contiguous_iterator<Value>& lhs,
             const basic_contiguous_iterator<Value>& rhs) noexcept
    -> decltype (lhs.base () - rhs.base ())
  {
    return lhs.base () - rhs.base ();
  }

  template <typename Value>
  constexpr
  basic_contiguous_iterator<Value>
  operator+ (typename basic_contiguous_iterator<Value>::difference_type n,
             const basic_contiguous_iterator<Value>& it) noexcept
  {
    return it + n;
  }

  template <typename Iterator>
  auto
  as_contiguous_iterator (Iterator it)
    -> basic_contiguous_iterator<std::remove_reference_t<decltype (*it)>>
  {
    return basic_contiguous_iterator<std::remove_reference_t<decltype (*it)>> (*it);
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

  template <typename Iterator, typename Projection>
  class projection_iterator
  {
  public:
    using iterator_type   = Iterator;
    using projection_type = Projection;

    using iterator_dereference_type = decltype (*std::declval<iterator_type> ());
    using projection_result_type = std::invoke_result_t<projection_type, iterator_dereference_type>;

    using difference_type   = typename std::iterator_traits<iterator_type>::difference_type;
    using value_type        = remove_cvref_t<projection_result_type>;
    using pointer           = void;
    using reference         = projection_result_type;
    using iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;

    projection_iterator            (void)                           = default;
    projection_iterator            (const projection_iterator&)     = default;
    projection_iterator            (projection_iterator&&) noexcept = default;
    projection_iterator& operator= (const projection_iterator&)     = default;
    projection_iterator& operator= (projection_iterator&&) noexcept = default;
    ~projection_iterator           (void)                           = default;

    template <typename Iter, typename Proj>
    constexpr
    projection_iterator (Iter&& iter, Proj&& proj)
      : m_iterator   (std::forward<Iter> (iter)),
        m_projection (std::in_place, std::forward<Projection> (proj))
    { }

    template <typename I,
              std::enable_if_t<std::is_convertible_v<I, iterator_type>> * = nullptr>
    constexpr GCH_IMPLICIT_CONVERSION
    projection_iterator (const projection_iterator<I, projection_type>& other)
      : m_iterator   (other.get_iterator ()),
        m_projection (std::in_place, other.get_projection ())
    { }

    constexpr
    projection_iterator&
    operator++ (void)
    {
      ++m_iterator;
      return *this;
    }

    constexpr
    projection_iterator
    operator++ (int)
    {
      return projection_iterator (m_iterator++, *m_projection);
    }

    constexpr
    projection_iterator&
    operator-- (void)
    {
      --m_iterator;
      return *this;
    }

    constexpr
    projection_iterator
    operator-- (int)
    {
      return projection_iterator (m_iterator--, *m_projection);
    }

    constexpr
    projection_iterator&
    operator+= (difference_type n)
    {
      m_iterator += n;
      return *this;
    }

    constexpr
    projection_iterator
    operator+ (difference_type n) const
    {
      return projection_iterator (m_iterator + n, *m_projection);
    }

    constexpr
    projection_iterator&
    operator-= (difference_type n)
    {
      m_iterator -= n;
      return *this;
    }

    constexpr
    projection_iterator
    operator- (difference_type n) const
    {
      return projection_iterator (m_iterator - n, *m_projection);
    }

    constexpr
    reference
    operator* (void) const
      noexcept (std::is_nothrow_invocable_v<projection_type, iterator_dereference_type>)
    {
      return gch::invoke (*m_projection, *m_iterator);
    }

    constexpr
    reference
    operator[] (difference_type n) const
      noexcept (std::is_nothrow_invocable_v<projection_type, iterator_dereference_type>)
    {
      return *(*this + n);
    }

    constexpr
    iterator_type
    get_iterator () const
    {
      return m_iterator;
    }

    constexpr
    projection_type
    get_projection (void) const
    {
      return *m_projection;
    }

  private:
    iterator_type                  m_iterator;
    std::optional<projection_type> m_projection;
  };

  template <typename IteratorLHS, typename ProjectionLHS,
            typename IteratorRHS, typename ProjectionRHS>
  constexpr
  bool
  operator== (const projection_iterator<IteratorLHS, ProjectionLHS>& lhs,
              const projection_iterator<IteratorRHS, ProjectionRHS>& rhs)
  {
    return lhs.get_iterator () == rhs.get_iterator ();
  }

  template <typename IteratorLHS, typename ProjectionLHS,
            typename IteratorRHS, typename ProjectionRHS>
  constexpr
  bool
  operator!= (const projection_iterator<IteratorLHS, ProjectionLHS>& lhs,
              const projection_iterator<IteratorRHS, ProjectionRHS>& rhs)
  {
    return lhs.get_iterator () != rhs.get_iterator ();
  }

  template <typename IteratorLHS, typename ProjectionLHS,
            typename IteratorRHS, typename ProjectionRHS>
  constexpr
  bool
  operator< (const projection_iterator<IteratorLHS, ProjectionLHS>& lhs,
             const projection_iterator<IteratorRHS, ProjectionRHS>& rhs)
  {
    return lhs.get_iterator () < rhs.get_iterator ();
  }

  template <typename IteratorLHS, typename ProjectionLHS,
            typename IteratorRHS, typename ProjectionRHS>
  constexpr
  bool
  operator> (const projection_iterator<IteratorLHS, ProjectionLHS>& lhs,
             const projection_iterator<IteratorRHS, ProjectionRHS>& rhs)
  {
    return lhs.get_iterator () > rhs.get_iterator ();
  }

  template <typename IteratorLHS, typename ProjectionLHS,
            typename IteratorRHS, typename ProjectionRHS>
  constexpr
  bool
  operator<= (const projection_iterator<IteratorLHS, ProjectionLHS>& lhs,
              const projection_iterator<IteratorRHS, ProjectionRHS>& rhs)
  {
    return lhs.get_iterator () <= rhs.get_iterator ();
  }

  template <typename IteratorLHS, typename ProjectionLHS,
            typename IteratorRHS, typename ProjectionRHS>
  constexpr
  bool
  operator>= (const projection_iterator<IteratorLHS, ProjectionLHS>& lhs,
              const projection_iterator<IteratorRHS, ProjectionRHS>& rhs)
  {
    return lhs.get_iterator () >= rhs.get_iterator ();
  }

  template <typename IteratorLHS, typename ProjectionLHS,
            typename IteratorRHS, typename ProjectionRHS>
  constexpr
  auto
  operator- (const projection_iterator<IteratorLHS, ProjectionLHS>& lhs,
             const projection_iterator<IteratorRHS, ProjectionRHS>& rhs)
    -> decltype (lhs.get_iterator () - rhs.get_iterator ())
  {
    return lhs.get_iterator () - rhs.get_iterator ();
  }

  template <typename Iterator, typename Projection>
  constexpr
  projection_iterator<Iterator, Projection>
  operator+ (typename projection_iterator<Iterator, Projection>::difference_type n,
             const projection_iterator<Iterator, Projection>& it)
  {
    return it + n;
  }

  template <typename Iterator, typename Projection>
  projection_iterator (Iterator, Projection) -> projection_iterator<Iterator, Projection>;

  class index_generator
  {
  public:
    using difference_type   = std::ptrdiff_t;
    using value_type        = void;
    using pointer           = void;
    using reference         = void;
    using iterator_category = std::random_access_iterator_tag;

    index_generator            (void)                       = default;
    index_generator            (const index_generator&)     = default;
    index_generator            (index_generator&&) noexcept = default;
    index_generator& operator= (const index_generator&)     = default;
    index_generator& operator= (index_generator&&) noexcept = default;
    ~index_generator           (void)                       = default;

    constexpr explicit
    index_generator (difference_type index)
      : m_index (index)
    { }

    template <typename T, std::enable_if_t<std::is_unsigned_v<T>> * = nullptr>
    constexpr explicit
    index_generator (T index)
      : m_index (static_cast<difference_type> (index))
    {
      assert (index <= static_cast<std::size_t> ((std::numeric_limits<difference_type>::max) ()));
    }

    constexpr
    index_generator&
    operator++ (void) noexcept
    {
      ++m_index;
      return *this;
    }

    constexpr
    index_generator
    operator++ (int) noexcept
    {
      return index_generator (m_index++);
    }

    constexpr
    index_generator&
    operator-- (void)
    {
      --m_index;
      return *this;
    }

    constexpr
    index_generator
    operator-- (int)
    {
      return index_generator (m_index--);
    }

    constexpr
    index_generator&
    operator+= (difference_type n)
    {
      m_index += n;
      return *this;
    }

    constexpr
    index_generator
    operator+ (difference_type n) const
    {
      return index_generator (m_index + n);
    }

    constexpr
    index_generator&
    operator-= (difference_type n)
    {
      m_index -= n;
      return *this;
    }

    constexpr
    index_generator
    operator- (difference_type n) const
    {
      return index_generator (m_index - n);
    }

    constexpr
    const index_generator&
    operator* (void) const noexcept
    {
      return *this;
    }

    constexpr
    difference_type
    get_index (void) const noexcept
    {
      return m_index;
    }

  private:
    difference_type m_index;
  };

  constexpr
  bool
  operator== (const index_generator& lhs, const index_generator& rhs)
  {
    return lhs.get_index () == rhs.get_index ();
  }

  constexpr
  bool
  operator!= (const index_generator& lhs, const index_generator& rhs)
  {
    return lhs.get_index () != rhs.get_index ();
  }

  constexpr
  bool
  operator< (const index_generator& lhs, const index_generator& rhs)
  {
    return lhs.get_index () < rhs.get_index ();
  }

  constexpr
  bool
  operator> (const index_generator& lhs, const index_generator& rhs)
  {
    return lhs.get_index () > rhs.get_index ();
  }

  constexpr
  bool
  operator<= (const index_generator& lhs, const index_generator& rhs)
  {
    return lhs.get_index () <= rhs.get_index ();
  }

  constexpr
  bool
  operator>= (const index_generator& lhs, const index_generator& rhs)
  {
    return lhs.get_index () >= rhs.get_index ();
  }

  constexpr
  auto
  operator- (const index_generator& lhs, const index_generator& rhs)
    -> decltype (lhs.get_index () - rhs.get_index ())
  {
    return lhs.get_index () - rhs.get_index ();
  }

  constexpr
  index_generator
  operator+ (index_generator::difference_type n, const index_generator& it)
  {
    return it + n;
  }

  template <typename Integer, typename Functor>
  projection_iterator<index_generator, Functor>
  make_generation_iterator (Integer&& index, Functor&& functor)
  {
    return { index_generator (std::forward<Integer> (index)), std::forward<Functor> (functor) };
  }

}

#endif // OCTAVE_IR_UTILITIES_IR_ITERATOR_HPP

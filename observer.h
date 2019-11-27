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

//! standalone
#if ! defined (octave_ir_observable_h)
#define octave_ir_observable_h 1

#include "octave-config.h"
#include <utility>
#include <list>

namespace octave
{
  template <typename Child, typename Parent = void>
  class observer;

  template <typename Child, typename Parent = observer<Child>>
  class observee;

  template <typename Child>
  class child_ptr
  {
  public:

    friend observer<Child>;
    friend observee<Child>;

    using element_type   = observee<Child>;
    using self_type      = child_ptr<Child>;
    using reference_type = element_type&;
    using pointer_type   = element_type *;

    explicit constexpr child_ptr (pointer_type ptr) noexcept
      : m_ptr (ptr)
    { }

    child_ptr (const child_ptr&)                      = delete;
    child_ptr (child_ptr&& other) noexcept            = delete;
    child_ptr& operator= (const child_ptr&)           = delete;
    child_ptr& operator= (child_ptr&& other) noexcept = delete;

    ~child_ptr (void) noexcept;

//  /* implicitly convertible to pointer_type */
//  operator pointer_type (void) const noexcept
//  {
//    return m_ptr;
//  }
//
//  operator reference_type (void) const
//  {
//    return *m_ptr;
//  }

    constexpr pointer_type get (void) const noexcept
    {
      return m_ptr;
    }

    constexpr reference_type operator* (void) const
    {
      return *get ();
    }

    constexpr pointer_type operator-> (void) const
    {
      return get ();
    }

  private:

    // access available only to self_type observer<T>, and
    // observee<T>
    void reset (pointer_type ptr = pointer_type ()) noexcept
    {
      m_ptr = ptr;
    }

    pointer_type m_ptr = nullptr;

  };

// the SFINAE here prevents usage with classes which don't inherit observee.
  template <typename Child>
  class observer<Child>
  {
  public:

//  static_assert (std::is_base_of<observee<Child>, Child>::value,
//                 "observee<Child> was not inherited by Child");

    using child_type      = Child;
    using self_type       = observer<child_type>;
    using child_base_type = observee<child_type>;

    friend child_base_type;

  private:
    using element_type    = child_ptr<child_type>;
    using child_list      = std::list<element_type>;
    using internal_iter   = typename child_list::iterator;
    using internal_citer  = typename child_list::const_iterator;
    using internal_riter  = typename child_list::reverse_iterator;
    using internal_criter = typename child_list::const_reverse_iterator;
    using internal_ref    = typename child_list::reference;
    using internal_cref   = typename child_list::const_reference;

  public:

    // pretend like child_ptr doesn't exist
    struct const_iterator
    {
      using self_type         = const_iterator;

      using difference_type   = typename internal_citer::difference_type;
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type        = child_type;
      using pointer           = child_type *;
      using const_pointer     = const child_type *;
      using reference         = child_type&;
      using const_reference   = const child_type&;

      const_iterator (internal_citer cit)
        : m_citer (cit)
      { }

      const_iterator (internal_iter it)
        : const_iterator (internal_citer (it))
      { }

      constexpr reference operator* (void) const noexcept
      {
        return static_cast<reference> (m_citer->operator* ());
      }

      constexpr pointer operator-> (void) const noexcept
      {
        return static_cast<pointer> (m_citer->operator-> ());
      }

      self_type& operator++ (void) noexcept
      {
        ++m_citer;
        return *this;
      }

      self_type operator++ (int) noexcept
      {
        self_type tmp = *this;
        ++m_citer;
        return tmp;
      }

      self_type& operator-- (void) noexcept
      {
        --m_citer;
        return *this;
      }

      self_type operator-- (int) noexcept
      {
        self_type tmp = *this;
        --m_citer;
        return tmp;
      }

      friend bool operator== (const self_type& x, const self_type& y) noexcept
      {
        return x.m_citer == y.m_citer;
      }

      friend bool operator!= (const self_type& x, const self_type& y) noexcept
      {
        return x.m_citer != y.m_citer;
      }

      friend class observer;

    private:
      internal_citer m_citer;
    };

    using iter   = const_iterator;
    using citer  = const_iterator;
    using riter  = std::reverse_iterator<iter>;
    using criter = std::reverse_iterator<citer>;
    using ref    = typename const_iterator::reference;
    using cref   = typename const_iterator::const_reference;

    observer (void) = default;

    observer (const observer&) = delete;

    observer (observer&& other) noexcept
      : m_children (std::move (other.m_children))
    { }

    observer& operator= (const observer&) = delete;
    observer& operator= (observer&& other) noexcept
    {
      m_children = std::move (other.m_children);
      return *this;
    }

    std::size_t num_children (void) const noexcept
    {
      return m_children.size ();
    }

    void transfer (observer&& src, citer pos)
    {
      for (internal_ref c_ptr : src.m_children)
        c_ptr->replace_observer (this);
      return m_children.splice (pos.m_citer, src.m_children);
    }

    void transfer (observer& src, citer pos)
    {
      return transfer (std::move (src), pos);
    }

    template <typename T>
    void transfer (T&& src)
    {
      return transfer (std::forward<T> (src), m_children.cend ());
    }

    iter   begin   (void)       noexcept { return m_children.begin (); }
    citer  begin   (void) const noexcept { return m_children.begin (); }
    citer  cbegin  (void) const noexcept { return m_children.cbegin (); }

    iter   end     (void)       noexcept { return m_children.end (); }
    citer  end     (void) const noexcept { return m_children.end (); }
    citer  cend    (void) const noexcept { return m_children.cend (); }

    riter  rbegin  (void)       noexcept { return m_children.rbegin (); }
    criter rbegin  (void) const noexcept { return m_children.rbegin (); }
    criter crbegin (void) const noexcept { return m_children.crbegin (); }

    riter  rend    (void)       noexcept { return m_children.rend (); }
    criter rend    (void) const noexcept { return m_children.rend (); }
    criter crend   (void) const noexcept { return m_children.crend (); }

    ref    front   (void)                { return *begin (); }
    cref   front   (void) const          { return *begin (); }

    ref    back    (void)                { return *--end (); }
    cref   back    (void) const          { return *--end (); }

    void   reverse (void)       noexcept { m_children.reverse (); }

    bool has_children (void) const noexcept
    {
      return ! m_children.empty ();
    }

    template <typename ...Args>
    child_type create (Args... args)
    {
      return { *this, std::forward<Args> (args)... };
    }

  private:

    internal_iter track_child (child_base_type *ptr)
    {
      return m_children.emplace (m_children.end (), ptr);
    }

    internal_iter untrack_child (internal_citer cit)
    {
      return m_children.erase (cit);
    }

    child_list m_children;

  };

  template <typename Child, typename Parent>
  class observer : public observer<Child>
  {
  public:

    using child_type      = Child;
    using parent_type     = Parent;
    using base_type       = observer<Child>;
    using self_type       = observer<Child, Parent>;

    using child_base_type = observee<child_type, parent_type>;

    friend child_base_type;

  private:
    using element_type    = child_ptr<child_type>;
    using child_list      = std::list<element_type>;
    using internal_iter   = typename child_list::iterator;
    using internal_citer  = typename child_list::const_iterator;
    using internal_riter  = typename child_list::reverse_iterator;
    using internal_criter = typename child_list::const_reverse_iterator;
    using internal_ref    = typename child_list::reference;
    using internal_cref   = typename child_list::const_reference;

  public:

    // pretend like child_ptr doesn't exist
    struct const_iterator
    {
      using self_type         = const_iterator;

      using difference_type   = typename internal_citer::difference_type;
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type        = child_type;
      using pointer           = child_type *;
      using const_pointer     = const child_type *;
      using reference         = child_type&;
      using const_reference   = const child_type&;

      const_iterator (internal_citer cit)
        : m_citer (cit)
      { }

      const_iterator (internal_iter it)
        : const_iterator (internal_citer (it))
      { }

      constexpr reference operator* (void) const noexcept
      {
        return static_cast<reference> (m_citer->operator* ());
      }

      constexpr pointer operator-> (void) const noexcept
      {
        return static_cast<pointer> (m_citer->operator-> ());
      }

      self_type& operator++ (void) noexcept
      {
        ++m_citer;
        return *this;
      }

      self_type operator++ (int) noexcept
      {
        self_type tmp = *this;
        ++m_citer;
        return tmp;
      }

      self_type& operator-- (void) noexcept
      {
        --m_citer;
        return *this;
      }

      self_type operator-- (int) noexcept
      {
        self_type tmp = *this;
        --m_citer;
        return tmp;
      }

      friend bool operator== (const self_type& x, const self_type& y) noexcept
      {
        return x.m_citer == y.m_citer;
      }

      friend bool operator!= (const self_type& x, const self_type& y) noexcept
      {
        return x.m_citer != y.m_citer;
      }

    private:
      internal_citer m_citer;
    };

    using iter   = const_iterator;
    using citer  = const_iterator;
    using riter  = std::reverse_iterator<iter>;
    using criter = std::reverse_iterator<citer>;
    using ref    = typename const_iterator::reference;
    using cref   = typename const_iterator::const_reference;

    explicit observer (parent_type *parent)
      : m_parent (parent)
    { }

    observer (const observer&) = delete;

    observer (observer&& other) noexcept
      : base_type (std::move (other)),
        m_parent (other.m_parent)
    { }

    observer& operator= (const observer&) = delete;

    observer& operator= (observer&& other) noexcept
    {
      base_type::operator= (std::move (other));
      m_parent = other.m_parent;
      return *this;
    }

    constexpr bool has_parent (void) const noexcept
    {
      return m_parent != nullptr;
    }

    constexpr parent_type *get_parent (void) const noexcept
    {
      return m_parent;
    };

  private:

    parent_type *m_parent = nullptr;

  };

//template <template <typename...> class Pack, typename ...Children,
//          typename Parent>
//class observer<Pack<Children...>, Parent>
//{
//  template <typename Child>
//  using sub_type = observer<Child, Parent>;
//
//private:
//
//  template <std::size_t I, typename T, typename Head, typename ...Tail>
//  struct type_offset : type_offset<I + 1, T, Tail...>
//  { };
//
//  template <std::size_t I, typename T, typename ...Tail>
//  struct type_offset<I, T, T, Tail...> : std::integral_constant<std::size_t, I>
//  { };
//
//  template <typename T>
//  using type_index = type_offset<0, T, observer<Children, Parent>...>;
//
//public:
//
//  using tuple_type = std::tuple<observer<Children, Parent>...>;
//
//  template <typename T>
//  typename sub_type<T>::iter begin (void) noexcept
//  {
//    return get<T> ().begin ();
//  }
//
//  template <typename T>
//  typename sub_type<T>::citer begin (void) const noexcept
//  {
//    return get<T> ().begin ();
//  }
//
//  template <typename T>
//  typename sub_type<T>::citer cbegin (void) const noexcept
//  {
//    return get<T> ().cbegin ();
//  }
//
//  template <typename T>
//  typename sub_type<T>::iter end (void) noexcept
//  {
//    return get<T> ().end ();
//  }
//
//  template <typename T>
//  typename sub_type<T>::citer end (void) const noexcept
//  {
//    return get<T> ().end ();
//  }
//
//  template <typename T>
//  typename sub_type<T>::citer cend (void) const noexcept
//  {
//    return get<T> ().cend ();
//  }
//
//  template <typename T>
//  sub_type<T>& get (void) noexcept
//  {
//    return std::get<type_index<T>> (m_subs);
//  }
//
//  template <typename T>
//  const sub_type<T>& get (void) const noexcept
//  {
//    return std::get<type_index<T>> (m_subs);
//  }
//
//private:
//
//  std::tuple<observer<Children, Parent>...> m_subs;
//
//};

  template <typename Child>
  class observee<Child>
  {
  public:

    using derived_type    = Child;
    using self_type       = observee<Child>;
    using observer_type   = observer<Child>;
    using self_iter_type  = typename std::list<child_ptr<Child>>::iterator;

    observee (void) noexcept = default;

    explicit observee (observer_type& parent)
      : m_observer (&parent),
        m_self_iter (m_observer->track_child (this))
    { }

    observee (const observee& other)
      : m_observer (other.m_observer),
        m_self_iter (has_observer () ? m_observer->track_child (this)
                                     : self_iter_type { })
    { }

    observee (observee&& other) noexcept
      : m_observer (other.m_observer),
        m_self_iter (other.m_self_iter)
    {
      if (has_observer ())
        {
          m_self_iter->reset (this);
          other.replace_observer ();
        }
    }

    observee& operator= (const observee& other)
    {
      if (&other != this)
        {
          if (has_observer ())
            m_observer->untrack_child (m_self_iter);

          m_observer = other.m_observer;

          if (has_observer ())
            m_self_iter = m_observer->track_child (this);
        }
      return *this;
    }

    observee& operator= (observee&& other) noexcept
    {
      if (this != &other)
        {
          if (has_observer ())
            m_observer->untrack_child (m_self_iter);

          m_observer = other.m_observer;
          m_self_iter = other.m_self_iter;

          if (has_observer ())
            {
              m_self_iter->reset (this);
              other.replace_observer ();
            }
        }
      return *this;
    }

    ~observee (void) noexcept
    {
      if (has_observer ())
        m_observer->untrack_child (m_self_iter);
    }

    constexpr bool has_observer (void) const noexcept
    {
      return m_observer != nullptr;
    }

    //! only use after using has_parent
    constexpr observer_type *get_observer (void) const noexcept
    {
      return m_observer;
    }

    friend void observer<Child>::transfer (observer<Child>&& src,
                                           typename observer<Child>::citer pos);

    friend child_ptr<Child>::~child_ptr (void) noexcept;

  private:

    void replace_observer (observer_type *ptr = nullptr) noexcept
    {
      m_observer = ptr;
    }

    observer_type *m_observer = nullptr;
    self_iter_type m_self_iter = self_iter_type { };

  };

  template <typename Child, typename Parent>
  class observee : public observee<Child>
  {

  public:

    using derived_type       = Child;
    using parent_type        = Parent;
    using base_type          = observee<Child>;
    using self_type          = observee<Child, Parent>;
    using observer_type      = observer<Child, Parent>;
    using observer_base_type = typename observer_type::base_type;

    observee (void) noexcept = default;

    explicit observee (observer_type& observer)
      : base_type (observer)
    { }

    observee (const observee& other)
      : base_type (other)
    { }

    observee (observee&& other) noexcept
      : base_type (std::move (other))
    { }

    observee& operator= (const observee& other)
    {
      base_type::operator= (other);
      return *this;
    }

    observee& operator= (observee&& other) noexcept
    {
      base_type::operator= (std::move (other));
      return *this;
    }

    constexpr bool has_parent (void) const noexcept
    {
      return this->has_observer () && this->get_observer ()->has_parent ();
    }

    constexpr parent_type *get_parent (void) const noexcept
    {
      return this->has_observer () ? this->get_observer()->get_parent ()
                                   : nullptr;
    }
  };

  template <typename T>
  child_ptr<T>::~child_ptr (void) noexcept
  {
    if (m_ptr != nullptr)
      m_ptr->replace_observer ();
  }
}

#endif

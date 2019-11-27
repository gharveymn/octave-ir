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
#if ! defined (octave_tracker_h)
#define octave_tracker_h 1

#include "octave-config.h"
#include <utility>
#include <list>

namespace octave
{
  template <typename Child, typename Parent = void>
  class tracker;

  template <typename Child>
  class reporter_base;

  template <typename Child, typename Parent>
  class intrusive_reporter;

  template <typename Parent, typename Tracker = tracker<Parent>>
  class reporter;

  template <typename Child>
  class child_ptr
  {
  public:

    friend tracker<Child>;
    friend reporter_base<Child>;

    using element_type   = reporter_base<Child>;
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

    ~child_ptr (void) noexcept
    {
      if (m_ptr != nullptr)
        m_ptr->replace_tracker ();
    }

    constexpr pointer_type get (void) const noexcept
    {
      return m_ptr;
    }

    constexpr reference_type operator* (void) const
    {
      return *m_ptr;
    }

    constexpr pointer_type operator-> (void) const
    {
      return m_ptr;
    }

    friend bool operator== (child_ptr<Child> c, pointer_type p) noexcept
    {
      return c.m_ptr == p;
    }

    friend bool operator== (pointer_type p, child_ptr<Child> c) noexcept
    {
      return p == c.m_ptr;
    }

    friend bool operator!= (child_ptr<Child> c, pointer_type p) noexcept
    {
      return c.m_ptr != p;
    }

    friend bool operator!= (pointer_type p, child_ptr<Child> c) noexcept
    {
      return p != c.m_ptr;
    }

  private:

    // access available only to self_type tracker<T>, and
    // reporter<T>
    void reset (pointer_type ptr = pointer_type ()) noexcept
    {
      m_ptr = ptr;
    }

    pointer_type m_ptr = nullptr;

  };

  template <typename Child>
  class tracker<Child>
  {
  public:

    using child_type      = Child;
    using self_type       = tracker<child_type>;
    using child_base_type = reporter_base<child_type>;

    friend reporter_base<child_type>;

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
        const self_type tmp = *this;
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
        const self_type tmp = *this;
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

      friend class tracker;

    private:
      internal_citer m_citer;
    };

    using iter   = const_iterator;
    using citer  = const_iterator;
    using riter  = std::reverse_iterator<iter>;
    using criter = std::reverse_iterator<citer>;
    using ref    = typename const_iterator::reference;
    using cref   = typename const_iterator::const_reference;

    tracker (void) = default;

    tracker (const tracker&) = delete;

    tracker (tracker&& other) noexcept
      : m_children (std::move (other.m_children))
    { }

    tracker& operator= (const tracker&) = delete;
    tracker& operator= (tracker&& other) noexcept
    {
      m_children = std::move (other.m_children);
      return *this;
    }

    std::size_t num_children (void) const noexcept
    {
      return m_children.size ();
    }

    void transfer_from (tracker&& src, citer pos)
    {
      for (internal_ref c_ptr : src.m_children)
        c_ptr->replace_tracker (this);
      return m_children.splice (pos.m_citer, src.m_children);
    }

    void transfer_from (tracker& src, citer pos)
    {
      return transfer_from (std::move (src), pos);
    }

    template <typename T>
    void transfer_from (T&& src)
    {
      return transfer_from (std::forward<T> (src), m_children.cend ());
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

    internal_iter track (child_base_type *ptr)
    {
      return m_children.emplace (m_children.end (), ptr);
    }

    internal_iter untrack (internal_citer cit)
    {
      return m_children.erase (cit);
    }

    child_list m_children;

  };

  template <typename Child, typename Parent>
  class tracker : public tracker<Child>
  {
  public:

    using child_type      = Child;
    using parent_type     = Parent;
    using base_type       = tracker<Child>;
    using self_type       = tracker<Child, Parent>;
    using child_base_type = intrusive_reporter<child_type, parent_type>;

    friend child_base_type;

    explicit tracker (parent_type *parent)
      : m_parent (parent)
    { }

    tracker (const tracker&) = delete;

    tracker (tracker&& other) noexcept
      : base_type (std::move (other)),
        m_parent (other.m_parent)
    { }

    tracker& operator= (const tracker&) = delete;

    tracker& operator= (tracker&& other) noexcept
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

  template <typename Child>
  class reporter_base
  {
  public:

    using child_type   = Child;
    using self_type      = reporter_base<Child>;
    using tracker_type   = tracker<Child>;
    using ext_citer_type = typename tracker<Child>::citer;
    using self_iter_type = typename std::list<child_ptr<Child>>::iterator;

    reporter_base (void) noexcept = default;

    explicit reporter_base (tracker_type& tkr)
      : m_tracker (&tkr),
        m_self_iter (m_tracker->track (this))
    { }

    reporter_base (const reporter_base& other)
      : m_tracker (other.m_tracker),
        m_self_iter (has_tracker () ? m_tracker->track (this)
                                     : self_iter_type { })
    { }

    reporter_base (reporter_base&& other) noexcept
      : m_tracker (other.m_tracker),
        m_self_iter (other.m_self_iter)
    {
      if (has_tracker ())
        {
          m_self_iter->reset (this);
          other.replace_tracker ();
        }
    }

    reporter_base& operator= (const reporter_base& other)
    {
      if (this != &other)
        {
          if (has_tracker ())
            m_tracker->untrack (m_self_iter);

          m_tracker = other.m_tracker;

          if (has_tracker ())
            m_self_iter = m_tracker->track (this);
        }
      return *this;
    }

    reporter_base& operator= (reporter_base&& other) noexcept
    {
      if (this != &other)
        {
          if (has_tracker ())
            m_tracker->untrack (m_self_iter);

          m_tracker = other.m_tracker;
          m_self_iter = other.m_self_iter;

          if (has_tracker ())
            {
              m_self_iter->reset (this);
              other.replace_tracker ();
            }
        }
      return *this;
    }

    ~reporter_base (void) noexcept
    {
      if (has_tracker ())
        m_tracker->untrack (m_self_iter);
    }

    constexpr bool has_tracker (void) const noexcept
    {
      return m_tracker != nullptr;
    }

    //! only use after using has_parent
    constexpr tracker_type *get_base_tracker (void) const noexcept
    {
      return m_tracker;
    }

    std::size_t get_position (void) const noexcept
    {
      if (! has_tracker ())
        return 0;
      return std::distance (m_tracker->m_children.begin (), m_self_iter);
    }

    friend void tracker<Child>::transfer_from (tracker<Child>&& src,
                                                ext_citer_type pos);

    friend child_ptr<Child>::~child_ptr<Child> (void) noexcept;

  private:

    void replace_tracker (tracker_type *ptr = nullptr) noexcept
    {
      m_tracker = ptr;
    }

    tracker_type *m_tracker = nullptr;
    self_iter_type m_self_iter = self_iter_type { };

  };

  template <typename Child, typename Parent>
  class intrusive_reporter : public reporter_base<Child>
  {

  public:

    using child_type        = Child;
    using parent_type       = Parent;
    using base_type         = reporter_base<Child>;
    using self_type         = intrusive_reporter<Child, Parent>;
    using tracker_type      = tracker<Child, Parent>;
    using tracker_base_type = typename tracker_type::base_type;

    intrusive_reporter (void) noexcept = default;

    explicit intrusive_reporter (tracker_type& tkr)
      : base_type (tkr)
    { }

    intrusive_reporter (const intrusive_reporter& other)
      : base_type (other)
    { }

    intrusive_reporter (intrusive_reporter&& other) noexcept
      : base_type (std::move (other))
    { }

    intrusive_reporter& operator= (const intrusive_reporter& other)
    {
      base_type::operator= (other);
      return *this;
    }

    intrusive_reporter& operator= (intrusive_reporter&& other) noexcept
    {
      base_type::operator= (std::move (other));
      return *this;
    }

    constexpr tracker_type *get_tracker (void) const noexcept
    {
      return static_cast<tracker_type *> (this->get_base_tracker ());
    }

    constexpr bool has_parent (void) const noexcept
    {
      return this->has_tracker () && this->get_tracker ()->has_parent ();
    }

    constexpr parent_type *get_parent (void) const noexcept
    {
      return this->has_tracker () ? this->get_tracker()->get_parent ()
                                   : nullptr;
    }
  };

  //! non-intrusive; for use as a class member
  template <typename Child, typename Parent>
  class reporter : public intrusive_reporter<reporter<Child, Parent>, Parent>
  {
  public:
    using child_type   = Child;
    using parent_type  = Parent;
    using base_type    = intrusive_reporter<reporter, Parent>;
    using tracker_type = typename base_type::tracker_type;


    explicit reporter (child_type *child, tracker_type& tkr)
      : base_type (tkr),
        m_child (child)
    { }

    reporter (const reporter& other)
      : base_type (other),
        m_child (other.m_child)
    { }

    reporter (reporter&& other) noexcept
    : base_type (std::move (other)),
      m_child (other.m_child)
    { }

    reporter& operator= (const reporter& other)
    {
      base_type::operator= (other);
      m_child = other.m_child;
      return *this;
    }

    reporter& operator= (reporter&& other) noexcept
    {
      base_type::operator= (std::move (other));
      m_child = other.m_child;
      return *this;
    }

    constexpr const child_type& operator* (void) const noexcept
    {
      return *m_child;
    }

    constexpr child_type * operator-> (void) const noexcept
    {
      return m_child;
    }

  private:
    child_type *m_child;

  };

}

#endif

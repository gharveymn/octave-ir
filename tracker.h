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
#include <plf_list.h>
#include <utility>
#include <list>

namespace octave
{
  template <typename Reporter>
  class tracker_base;

  template <typename Reporter, typename Parent = tracker_base<Reporter>>
  class tracker;

  template <typename Reporter>
  class reporter_base;

  template <typename Derived, typename Parent = tracker<Derived>>
  class intrusive_reporter;

  template <typename Child, typename Parent = tracker<Child>>
  class reporter;

  template <typename Reporter>
  class reporter_ptr
  {
  public:

    using reporter_type  = Reporter;
    using self_type      = reporter_ptr<reporter_type>;
    using reference_type = reporter_type&;
    using pointer_type   = reporter_type *;

    friend tracker_base<Reporter>;
    friend reporter_base<Reporter>;

    explicit constexpr reporter_ptr (pointer_type ptr) noexcept
      : m_ptr (ptr)
    { }

    reporter_ptr (const reporter_ptr&)                      = delete;

    // no moves allowed because iterators in
    reporter_ptr (reporter_ptr&& other) noexcept            = delete;
    reporter_ptr& operator= (const reporter_ptr&)           = delete;
    reporter_ptr& operator= (reporter_ptr&& other) noexcept = delete;

    ~reporter_ptr (void) noexcept
    {
      if (m_ptr != nullptr)
        m_ptr->replace_tracker ();
    }

    constexpr pointer_type get (void) const noexcept
    {
      return m_ptr;
    }

    constexpr reference_type operator* (void) const noexcept
    {
      return *m_ptr;
    }

    constexpr pointer_type operator-> (void) const noexcept
    {
      return m_ptr;
    }

    friend bool operator== (reporter_ptr<Reporter> c, pointer_type p) noexcept
    {
      return c.m_ptr == p;
    }

    friend bool operator== (pointer_type p, reporter_ptr<Reporter> c) noexcept
    {
      return p == c.m_ptr;
    }

    friend bool operator!= (reporter_ptr<Reporter> c, pointer_type p) noexcept
    {
      return c.m_ptr != p;
    }

    friend bool operator!= (pointer_type p, reporter_ptr<Reporter> c) noexcept
    {
      return p != c.m_ptr;
    }

  private:

    // access available only to self_type tracker_base<T>, and
    // reporter<T>
    void reset (pointer_type ptr = pointer_type ()) noexcept
    {
      m_ptr = ptr;
    }

    pointer_type m_ptr = nullptr;

  };

  template <typename Reporter>
  class tracker_base
  {
  public:


    using reporter_type = Reporter;
    using self_type     = tracker_base<reporter_type>;

    friend reporter_base<reporter_type>;

  private:

    using element_type    = reporter_ptr<reporter_type>;
    using reporter_list   = plf::list<element_type>;
    using internal_iter   = typename reporter_list::iterator;
    using internal_citer  = typename reporter_list::const_iterator;
    using internal_riter  = typename reporter_list::reverse_iterator;
    using internal_criter = typename reporter_list::const_reverse_iterator;
    using internal_ref    = typename reporter_list::reference;
    using internal_cref   = typename reporter_list::const_reference;

  public:

    // pretend like reporter_ptr doesn't exist
    struct const_iterator
    {
      using self_type         = const_iterator;

      using difference_type   = typename internal_citer::difference_type;
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type        = reporter_type;
      using pointer           = reporter_type *;
      using const_pointer     = const reporter_type *;
      using reference         = reporter_type&;
      using const_reference   = const reporter_type&;

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

      friend class tracker_base;

    private:
      internal_citer m_citer;
    };

    using iter   = const_iterator;
    using citer  = const_iterator;
    using riter  = std::reverse_iterator<iter>;
    using criter = std::reverse_iterator<citer>;
    using ref    = typename const_iterator::reference;
    using cref   = typename const_iterator::const_reference;

    tracker_base (void) = default;

    tracker_base (const tracker_base&) = delete;

    tracker_base (tracker_base&& other) noexcept
      : m_reporters (std::move (other.m_reporters))
    { }

    tracker_base& operator= (const tracker_base&) = delete;

    tracker_base& operator= (tracker_base&& other) noexcept
    {
      m_reporters = std::move (other.m_reporters);
      return *this;
    }

    std::size_t num_children (void) const noexcept
    {
      return m_reporters.size ();
    }

    void transfer_from (tracker_base&& src, citer pos)
    {
      for (internal_ref cptr : src.m_reporters)
        {
          cptr->replace (this, m_reporters.emplace (pos.m_citer, cptr.get ()));
          cptr.reset ();
        }
      src.m_reporters.clear ();
    }

    void transfer_from (tracker_base& src, citer pos)
    {
      return transfer_from (std::move (src), pos);
    }

    template <typename T>
    void transfer_from (T&& src)
    {
      return transfer_from (std::forward<T> (src), m_reporters.cend ());
    }

    iter   begin   (void)       noexcept { return m_reporters.begin (); }
    citer  begin   (void) const noexcept { return m_reporters.begin (); }
    citer  cbegin  (void) const noexcept { return m_reporters.cbegin (); }

    iter   end     (void)       noexcept { return m_reporters.end (); }
    citer  end     (void) const noexcept { return m_reporters.end (); }
    citer  cend    (void) const noexcept { return m_reporters.cend (); }

    riter  rbegin  (void)       noexcept { return m_reporters.rbegin (); }
    criter rbegin  (void) const noexcept { return m_reporters.rbegin (); }
    criter crbegin (void) const noexcept { return m_reporters.crbegin (); }

    riter  rend    (void)       noexcept { return m_reporters.rend (); }
    criter rend    (void) const noexcept { return m_reporters.rend (); }
    criter crend   (void) const noexcept { return m_reporters.crend (); }

    ref    front   (void)                { return *begin (); }
    cref   front   (void) const          { return *begin (); }

    ref    back    (void)                { return *--end (); }
    cref   back    (void) const          { return *--end (); }

    void   reverse (void)       noexcept { m_reporters.reverse (); }

    bool has_children (void) const noexcept
    {
      return ! m_reporters.empty ();
    }

    template <typename ...Args>
    reporter_type create (Args... args)
    {
      return { *this, std::forward<Args> (args)... };
    }

  private:

    internal_iter track (reporter_type *ptr)
    {
      return m_reporters.emplace (m_reporters.end (), ptr);
    }

    internal_iter untrack (internal_citer cit)
    {
      return m_reporters.erase (cit);
    }

    reporter_list m_reporters;

  };

  template <typename Reporter, typename Parent>
  class tracker : public tracker_base<Reporter>
  {
  public:

    using reporter_type   = Reporter;
    using parent_type     = Parent;
    using base_type       = tracker_base<reporter_type>;
    using self_type       = tracker<reporter_type, parent_type>;
    using child_base_type = intrusive_reporter<reporter_type, parent_type>;

    friend intrusive_reporter<reporter_type, parent_type>;

    explicit tracker (parent_type *parent)
      : m_parent (parent)
    { }

    tracker (const tracker&)            = delete;
    tracker (tracker&&) noexcept        = delete;

    tracker (parent_type *new_parent, const tracker& other)
    : base_type (other),
      m_parent (new_parent)
    { }

    tracker (parent_type *new_parent, tracker&& other) noexcept
    : base_type (std::move (other)),
      m_parent (new_parent)
    { }

    tracker& operator= (const tracker& other)
    {
      base_type::operator= (other);
      return *this;
    }

    tracker& operator= (tracker&& other) noexcept
    {
      base_type::operator= (std::move (other));
      return *this;
    }

    constexpr parent_type& operator* (void) const noexcept
    {
      return *m_parent;
    }

    constexpr parent_type * operator-> (void) const noexcept
    {
      return m_parent;
    }

    constexpr bool has_parent (void) const noexcept
    {
      return m_parent != nullptr;
    }

    constexpr parent_type * get_parent (void) const noexcept
    {
      return m_parent;
    }

  private:

    parent_type *m_parent = nullptr;

  };

  template <typename Reporter>
  class tracker<Reporter> : public tracker_base<Reporter>
  {
  public:

    using reporter_type   = Reporter;
    using parent_type     = tracker_base<reporter_type>;
    using base_type       = tracker_base<reporter_type>;
    using self_type       = tracker<reporter_type, parent_type>;
    using child_base_type = intrusive_reporter<reporter_type, parent_type>;

    friend intrusive_reporter<reporter_type, parent_type>;

    tracker (void) = default;

    tracker (const tracker& other)
      : base_type (other)
    { }

    tracker (tracker&& other) noexcept
      : base_type (std::move (other))
    { }

    tracker& operator= (const tracker& other)
    {
      base_type::operator= (other);
      return *this;
    }

    tracker& operator= (tracker&& other) noexcept
    {
      base_type::operator= (std::move (other));
      return *this;
    }

    constexpr bool has_parent (void) const noexcept
    {
      return true;
    }

    constexpr parent_type * get_parent (void) const noexcept
    {
      return static_cast<parent_type *> (this);
    }

  };

  template <typename Reporter>
  class reporter_base
  {
  public:

    using reporter_type  = Reporter;
    using self_type      = reporter_base<reporter_type>;
    using tracker_type   = tracker_base<reporter_type>;
    using self_ptr_type  = reporter_ptr<reporter_type>;
    using ext_citer_type = typename tracker_base<reporter_type>::citer;
    using self_iter_type = typename tracker_type::internal_iter;

    friend void
    tracker_base<Reporter>::transfer_from (tracker_base<Reporter>&& src,
                                           ext_citer_type pos);

    friend reporter_ptr<Reporter>::~reporter_ptr<Reporter> (void) noexcept;

    reporter_base (void) noexcept = default;

    explicit reporter_base (tracker_type& tkr)
      : m_tracker (&tkr),
        m_self_iter (m_tracker->track (downcast (this)))
    { }

    reporter_base (const reporter_base& other)
      : m_tracker (other.m_tracker),
        m_self_iter (has_tracker () ? m_tracker->track (downcast (this))
                                     : self_iter_type { })
    { }

    reporter_base (reporter_base&& other) noexcept
      : m_tracker (other.m_tracker),
        m_self_iter (other.m_self_iter)
    {
      if (has_tracker ())
        {
          m_self_iter->reset (downcast (this));
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
            m_self_iter = m_tracker->track (downcast (this));
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
              m_self_iter->reset (downcast (this));
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
      return std::distance (m_tracker->m_reporters.begin (), m_self_iter);
    }

  private:

    void replace_tracker (tracker_type *ptr = nullptr) noexcept
    {
      m_tracker = ptr;
    }

    void replace_iter (self_iter_type it = self_iter_type { }) noexcept
    {
      m_self_iter = it;
    }

    void replace (tracker_type *ptr = nullptr,
                  self_iter_type it = self_iter_type { }) noexcept
    {
      m_tracker   = ptr;
      m_self_iter = it;
    }

    static constexpr const reporter_type * downcast (const self_type *r)
    {
      return static_cast<const reporter_type *> (r);
    }

    static constexpr reporter_type * downcast (self_type *r)
    {
      return static_cast<reporter_type *> (r);
    }

    tracker_type *m_tracker = nullptr;
    self_iter_type m_self_iter = self_iter_type { };

  };

  template <typename Derived, typename Parent>
  class intrusive_reporter : public reporter_base<Derived>
  {

  public:

    using derived_type      = Derived;
    using parent_type       = Parent;
    using base_type         = reporter_base<derived_type>;
    using self_type         = intrusive_reporter<derived_type, parent_type>;
    using tracker_type      = tracker<derived_type, parent_type>;
    using tracker_base_type = typename tracker_type::base_type;

    static_assert (std::is_same<tracker_base_type,
                                tracker_base<derived_type>>::value,
                   "tracker base has unexpected type");

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
    using self_type    = reporter<child_type, parent_type>;
    using base_type    = intrusive_reporter<self_type, parent_type>;
    using tracker_type = typename base_type::tracker_type;

    static_assert (std::is_same<tracker_type,
                                tracker<self_type, parent_type>>::value,
                   "tracker has unexpected type");

    explicit reporter (child_type *child, tracker_type& tkr)
      : base_type (tkr),
        m_child (child)
    { }

    reporter (const reporter&)            = delete;
    reporter (reporter&&) noexcept        = delete;

    reporter (child_type *new_child, const reporter& other)
      : base_type (other),
        m_child (new_child)
    { }

    reporter (child_type *new_child, reporter&& other) noexcept
    : base_type (std::move (other)),
      m_child (new_child)
    { }

    reporter& operator= (const reporter& other)
    {
      base_type::operator= (other);
      return *this;
    }

    reporter& operator= (reporter&& other) noexcept
    {
      base_type::operator= (std::move (other));
      return *this;
    }

    constexpr child_type& operator* (void) const noexcept
    {
      return *m_child;
    }

    constexpr child_type * operator-> (void) const noexcept
    {
      return m_child;
    }

  private:

    child_type *m_child = nullptr;

  };

}

#endif

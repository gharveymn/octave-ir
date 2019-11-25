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
  template <typename T>
  class observable_parent;
  
  template <typename T, typename Parent = observable_parent<T>>
  class observable_child;
  
  template <typename T>
  class child_ptr
  {
  public:
    
    using child_type = T;
    
    explicit constexpr child_ptr (child_type *ptr) noexcept
      : m_ptr (ptr)
    { }
    
    child_ptr (const child_ptr&) = delete;
    child_ptr (child_ptr&& other) noexcept
      : m_ptr (other.m_ptr)
    {
      other.reset ();
    }
    
    child_ptr& operator= (const child_ptr&)           = delete;
    child_ptr& operator= (child_ptr&& other) noexcept = delete;
    
    ~child_ptr (void) noexcept;
    
    constexpr child_type * get (void) const noexcept
    {
      return m_ptr;
    }
    
    void set (child_type *ptr) noexcept
    {
      m_ptr = ptr;
    }
    
    void reset (void) noexcept
    {
      m_ptr = nullptr;
    }
    
    void supplant_parent (observable_parent<child_type> *new_parent)
    {
      if (m_ptr != nullptr)
        m_ptr->supplant_parent (new_parent);
      reset ();
    }
  
  private:
    child_type *m_ptr = nullptr;
  };
  
  template <typename T>
  class observable_parent
  {
  public:
    
    static_assert (std::is_base_of<observable_child<T>, T>::value, "");
    
    using child_type = T;
  
  public:
    using element_type = child_ptr<T>;
    using child_list = std::list<element_type>;
    using iter = typename child_list::iterator;
    using citer = typename child_list::const_iterator;
    
    iter track_child (child_type* ptr)
    {
      return m_children.emplace (m_children.end (), ptr);
    }
    
    void untrack_child (citer cit)
    {
      m_children.erase (cit);
    }
    
    std::size_t num_children (void) const noexcept
    {
      return m_children.size ();
    }
    
    void transfer (observable_parent *to)
    {
      for (element_type& e : m_children)
        e.supplant_parent (to);
      m_children.clear ();
    }
  
  private:
    child_list m_children;
    
  };
  
  
  template <typename T>
  class observable_child<T>
  {
  public:
//  static_assert (std::is_base_of<observable_child<T>, T>::value, "");
    
    using self_type = T;
    using parent_type = observable_parent<T>;
    using self_iter_type = typename std::list<child_ptr<T>>::iterator;
    
    explicit observable_child (parent_type *parent) noexcept
      : m_parent (parent),
        m_self_iter (m_parent
                     ? m_parent->track_child (static_cast<self_type *> (this))
                     : self_iter_type ())
    { }
    
    observable_child (const observable_child& other)
      : observable_child (other.m_parent)
    { }
    
    observable_child (observable_child&& other) noexcept
    {
      operator= (std::move (other));
    }
    
    observable_child& operator= (const observable_child& other)
    {
      m_parent = other.m_parent;
      m_self_iter = m_parent
                    ? m_parent->track_child (static_cast<self_type *> (this))
                    : self_iter_type ();
      return *this;
    }
    
    observable_child& operator= (observable_child&& other) noexcept
    {
      if (has_parent ())
        m_parent->untrack_child (m_self_iter);
      
      m_parent = other.m_parent;
      m_self_iter = other.m_self_iter;
      
      if (has_parent ())
        {
          m_self_iter->set (static_cast<self_type *> (this));
          other.reset ();
        }
      return *this;
    }
    
    ~observable_child (void) noexcept
    {
      if (m_parent != nullptr)
        m_parent->untrack_child (m_self_iter);
    }
    
    constexpr bool has_parent (void) const noexcept
    {
      return m_parent != nullptr;
    }
    
    constexpr parent_type * get_parent (void) const noexcept
    {
      return m_parent;
    }
    
    void reset (void) noexcept
    {
      m_parent = nullptr;
    }
    
    void
    supplant_parent (parent_type *new_parent)
    {
      m_parent = new_parent;
      if (new_parent != nullptr)
        m_self_iter = new_parent->track_child (static_cast<self_type *> (this));
    }
  
  private:
    parent_type *m_parent = nullptr;
    self_iter_type m_self_iter;
    
  };
  
  template <typename T, typename Parent>
  class observable_child : public observable_child<T>
  {
    using base_type = observable_child<T>;
  public:
//  static_assert (std::is_base_of<observable_parent<T>, Parent>::value, "");
    using parent_type = Parent;
    
    explicit observable_child (parent_type *parent) noexcept
      : base_type (parent)
    { }
    
    observable_child (const observable_child& other)
      : base_type (other)
    { }
    
    observable_child (observable_child&& other) noexcept
      : base_type (std::move (other))
    { }
    
    observable_child& operator= (const observable_child& other)
    {
      base_type::operator= (other);
      return *this;
    }
    
    observable_child& operator= (observable_child&& other) noexcept
    {
      base_type::operator= (std::move (other));
      return *this;
    }
    
    constexpr parent_type * get_parent (void) const noexcept
    {
      return static_cast<parent_type> (base_type::get_parent ());
    }
    
  };
  
  template <typename T>
  child_ptr<T>::~child_ptr (void) noexcept
  {
    if (m_ptr != nullptr)
      m_ptr->reset ();
  }
}

#endif

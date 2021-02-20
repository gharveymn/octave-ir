/** ir-link-set.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_LINK_SET_HPP
#define OCTAVE_IR_IR_LINK_SET_HPP

// #include "ir-common-util.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/small_vector.hpp>

namespace gch
{

  template <typename T>
  class ir_link_set
    // : public init_counter
  {
  public:
    using container_type         = small_vector<nonnull_ptr<T>, 1>;
    using value_type             = nonnull_ptr<T>;
    using comparison_type        = nonnull_cptr<T>;

    using size_type              = typename container_type::size_type;
    using difference_type        = typename container_type::difference_type;

    using remote_reference       = T&;
    using const_remote_reference = const T&;

    using iterator               = typename container_type::const_iterator;
    using const_iterator         = typename container_type::const_iterator;
    using reverse_iterator       = typename container_type::const_reverse_iterator;
    using const_reverse_iterator = typename container_type::const_reverse_iterator;

    using value_t = value_type;
    using size_ty = size_type;
    using diff_ty = difference_type;

    using iter    = iterator;
    using citer   = const_iterator;
    using riter   = reverse_iterator;
    using criter  = const_reverse_iterator;

    struct insert_return_type
    {
      iterator position;
      bool     inserted;
    };

    class view_type
    {
    public:
      using iterator        = ir_link_set::const_iterator;
      using difference_type = ir_link_set::difference_type;
      using value_type      = ir_link_set::value_type;

      view_type            (void)                 = default;
      view_type            (const view_type&)     = default;
      view_type            (view_type&&) noexcept = default;
      view_type& operator= (const view_type&)     = default;
      view_type& operator= (view_type&&) noexcept = default;
      ~view_type           (void)                 = default;

      view_type (iterator first, iterator last)
        : m_first (first),
          m_last  (last)
      { }

      [[nodiscard]]
      iterator
      begin (void) const noexcept
      {
        return m_first;
      }

      [[nodiscard]]
      iterator
      end (void) const noexcept
      {
        return m_last;
      }

      [[nodiscard]]
      value_type
      front (void) const noexcept
      {
        return *begin ();
      }

      [[nodiscard]]
      value_type
      back (void) const noexcept
      {
        return *(--end ());
      }

      [[nodiscard]]
      std::size_t
      size (void) const noexcept
      {
        return std::distance (m_first, m_last);
      }

      [[nodiscard]]
      bool
      empty (void) const noexcept
      {
        return m_first == m_last;
      }

      [[nodiscard]]
      operator bool (void) const noexcept
      {
        return ! empty ();
      }

      [[nodiscard]]
      view_type
      next (difference_type count = 1) const &
      {
        view_type ret = *this;
        ret.advance (count);
        return ret;
      }

      [[nodiscard]]
      view_type
      next (difference_type count = 1) &&
      {
        advance (count);
        return *this;
      }

      [[nodiscard]]
      view_type
      prev (difference_type count = 1) const
      {
        return next (-count);
      }

      view_type&
      advance (difference_type count)
      {
        std::advance (m_first, count);
        return *this;
      }

    private:
      iter m_first;
      iter m_last;
    };

    ir_link_set            (void)                   = default;
    ir_link_set            (const ir_link_set&)     = default;
    ir_link_set            (ir_link_set&&) noexcept = default;
    ir_link_set& operator= (const ir_link_set&)     = default;
    ir_link_set& operator= (ir_link_set&&) noexcept = default;
    ~ir_link_set           (void)                   = default;

    template <typename InputIt,
              std::enable_if_t<std::is_same_v<
                value_type,
                typename std::iterator_traits<InputIt>::value_type>> * = nullptr>
    ir_link_set (InputIt first, InputIt last)
    {
      insert (first, last);
    }

    ir_link_set (std::initializer_list<value_type> init)
    {
      insert (init.begin (), init.end ());
    }

    [[nodiscard]]
    const_iterator
    begin (void) const noexcept
    {
     return m_data.begin ();
    }

    [[nodiscard]]
    const_iterator
    end (void) const noexcept
    {
     return m_data.end ();
    }

    [[nodiscard]]
    const_reverse_iterator
    rbegin (void) const noexcept
    {
     return m_data.rbegin ();
    }

    [[nodiscard]]
    const_reverse_iterator
    rend (void) const noexcept
    {
     return m_data.rend ();
    }

    [[nodiscard]]
    value_type
    front (void) const noexcept
    {
      return *m_data.begin ();
    }

    [[nodiscard]]
    value_type
    back (void) const noexcept
    {
      return *m_data.rbegin ();
    }

    [[nodiscard]]
    size_type
    size (void) const noexcept
    {
     return m_data.size ();
    }

    [[nodiscard]]
    bool
    empty (void) const noexcept
    {
      return m_data.empty ();
    }

    [[nodiscard]]
    size_type
    max_size (void) const noexcept
    {
      return m_data.max_size ();
    }

    void
    clear (void) noexcept
    {
      m_data.clear ();
    }

    insert_return_type
    insert (value_type remote_ptr)
    {
      citer found = lower_bound (remote_ptr);
      if (found != end () && *found == remote_ptr)
        return { found, false };
      return { m_data.insert (found, remote_ptr), true };
    }

    template <typename InputIt,
              std::enable_if_t<std::is_same_v<
                value_type,
                typename std::iterator_traits<InputIt>::value_type>> * = nullptr>
    void
    insert (InputIt first, InputIt last)
    {
      if (first == last)
        return;

      container_type sorted { first, last };
      std::sort (sorted.begin (), sorted.end ());
      sorted.erase (std::unique (sorted.begin (), sorted.end ()), sorted.end ());

      presorted_union (std::move (sorted));
    }

    void
    insert (std::initializer_list<value_type> ilist)
    {
      insert (ilist.begin (), ilist.end ());
    }

    insert_return_type
    emplace (value_type remote_ptr)
    {
      return insert (remote_ptr);
    }

    insert_return_type
    emplace (remote_reference remote)
    {
      return insert (nonnull_ptr { remote });
    }

    const_iterator
    erase (const_iterator pos)
    {
      return m_data.erase (pos);
    }

    const_iterator
    erase (const_iterator first, const_iterator last)
    {
      return m_data.erase (first, last);
    }

    bool
    erase (comparison_type remote_cptr)
    {
      if (citer found = find (remote_cptr) ; found != end ())
      {
        erase (found);
        return true;
      }
      return false;
    }

    bool
    erase (const_remote_reference remote)
    {
      return erase (nonnull_ptr { remote });
    }

    void
    swap (ir_link_set& other) noexcept
    {
      using std::swap;
      swap (m_data, other.m_data);
    }

    ir_link_set&
    merge (const ir_link_set& other)
    {
      presorted_union (other.m_data);
      return *this;
    }

    const_iterator
    find (nonnull_cptr<T> remote_cptr) const
    {
      return std::find (begin (), end (), remote_cptr);
    }

    const_iterator
    find (const_remote_reference remote) const
    {
      return find (nonnull_ptr { remote });
    }

    bool
    contains (nonnull_cptr<T> remote_cptr) const
    {
      return find (remote_cptr) != end ();
    }

    bool
    contains (const_remote_reference remote) const
    {
      return contains (nonnull_ptr { remote });
    }

    view_type
    equal_range (nonnull_cptr<T> remote_cptr) const
    {
      citer l = lower_bound (remote_cptr);
      return { l, *l == remote_cptr ? std::next (l) : l };
    }

    view_type
    equal_range (const_remote_reference remote) const
    {
      return equal_range (nonnull_ptr { remote });
    }

    const_iterator
    lower_bound (nonnull_cptr<T> remote_cptr) const
    {
      return std::lower_bound (begin (), end (), remote_cptr);
    }

    const_iterator
    lower_bound (const_remote_reference remote) const
    {
      return lower_bound (nonnull_ptr { remote });
    }

    const_iterator
    upper_bound (nonnull_cptr<T> remote_cptr) const
    {
      return std::upper_bound (begin (), end (), remote_cptr);
    }

    const_iterator
    upper_bound (const_remote_reference remote) const
    {
      return upper_bound (nonnull_ptr { remote });
    }

    friend inline
    bool
    operator== (const ir_link_set& lhs, const ir_link_set& rhs) noexcept
    {
      return lhs.m_data == rhs.m_data;
    }

    friend inline
    bool
    operator< (const ir_link_set& lhs, const ir_link_set& rhs)
    {
      return ! (lhs == rhs);
    }

  private:
    void
    presorted_union (const container_type& sorted_input)
    {
      if (m_data.empty ())
        m_data = sorted_input;
      else if (! sorted_input.empty ())
        nontrivial_union (sorted_input);
    }

    void
    presorted_union (container_type&& sorted_input)
    {
      if (m_data.empty ())
        m_data = std::move (sorted_input);
      else if (! sorted_input.empty ())
        nontrivial_union (sorted_input);
    }

    void
    nontrivial_union (const container_type& sorted_input)
    {
      container_type result;
      result.reserve (m_data.size () + sorted_input.size ());
      std::set_union (m_data.begin (), m_data.end (),
                      sorted_input.begin (), sorted_input.end (),
                      std::back_inserter (result));
      m_data = std::move (result);
    }

    container_type m_data;
  };

  template <typename T>
  inline
  bool
  operator!= (const ir_link_set<T>& lhs, const ir_link_set<T>& rhs) noexcept
  {
    return ! (lhs == rhs);
  }

  template <typename T>
  inline
  bool
  operator>= (const ir_link_set<T>& lhs, const ir_link_set<T>& rhs)
  {
    return ! (lhs < rhs);
  }

  template <typename T>
  inline
  bool
  operator> (const ir_link_set<T>& lhs, const ir_link_set<T>& rhs)
  {
    return rhs < lhs;
  }

  template <typename T>
  inline
  bool
  operator<= (const ir_link_set<T>& lhs, const ir_link_set<T>& rhs)
  {
    return rhs >= lhs;
  }

  template <typename T>
  inline
  void
  swap (ir_link_set<T>& lhs, ir_link_set<T>& rhs) noexcept
  {
    lhs.swap (rhs);
  }

  template <typename T, typename Pred>
  inline
  typename ir_link_set<T>::size_type
  erase_if (ir_link_set<T>& s, Pred pred)
  {
    typename ir_link_set<T>::size_type old_size = s.size ();
    for (typename ir_link_set<T>::const_iterator it = s.begin (); it != s.end ();)
    {
      if (pred (*it))
        it = s.erase (it);
      else
        ++it;
    }
    return old_size - s.size ();
  }

  template <typename T>
  inline
  ir_link_set<T>&
  operator|= (ir_link_set<T>& lhs, const ir_link_set<T>& rhs)
  {
    return lhs.merge (rhs);
  }

  template <typename T>
  [[nodiscard]] inline
  ir_link_set<T>&&
  operator| (ir_link_set<T>&& lhs, const ir_link_set<T>& rhs)
  {
    return std::move (lhs |= rhs);
  }

  template <typename T>
  [[nodiscard]] inline
  ir_link_set<T>&&
  operator| (const ir_link_set<T>& lhs, ir_link_set<T>&& rhs)
  {
    return std::move (rhs) | lhs;
  }

  template <typename T>
  [[nodiscard]] inline
  ir_link_set<T>&&
  operator| (ir_link_set<T>&& lhs, ir_link_set<T>&& rhs)
  {
    return std::move (lhs) | rhs;
  }

  template <typename T>
  [[nodiscard]] inline
  ir_link_set<T>
  operator| (const ir_link_set<T>& lhs, const ir_link_set<T>& rhs)
  {
    return ir_link_set (lhs) | rhs;
  }

  template <typename InputIt>
  ir_link_set (InputIt, InputIt)
    -> ir_link_set<typename std::pointer_traits<
                     typename std::iterator_traits<InputIt>::value_type>::element_type>;

}

#endif // OCTAVE_IR_IR_LINK_SET_HPP

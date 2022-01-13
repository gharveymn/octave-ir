/** ir-allocator-util.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_UTILITIES_IR_ALLOCATOR_UTIL_HPP
#define OCTAVE_IR_UTILITIES_IR_ALLOCATOR_UTIL_HPP

#include <gch/nonnull_ptr.hpp>

namespace gch
{
  template <typename Pair, typename Parent>
  class strongly_linked_map_allocator;

  template <typename Child, typename Parent>
  class strongly_linked_allocator
  {
  public:
    template <typename Key>
    using rebind_for_map = strongly_linked_map_allocator<std::pair<const Key, Child>, Parent>;

    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap            = std::false_type;
    using is_always_equal                        = std::false_type;

    using value_type = Child;

    strongly_linked_allocator            (void)                                 = delete;
    strongly_linked_allocator            (const strongly_linked_allocator&)     = default;
    strongly_linked_allocator            (strongly_linked_allocator&&) noexcept = default;
    strongly_linked_allocator& operator= (const strongly_linked_allocator&)     = default;
    strongly_linked_allocator& operator= (strongly_linked_allocator&&) noexcept = default;
    ~strongly_linked_allocator           (void)                                 = default;

    template <typename U>
    strongly_linked_allocator (const strongly_linked_allocator<U, Parent>& other) noexcept
      : m_parent (other.get_parent ())
    { }

    explicit
    strongly_linked_allocator (ir_def_timeline& parent) noexcept
      : m_parent (parent)
    { }

    template <typename U, typename ...Args>
    void
    construct (U *p, Args&&... args)
    {
      void *vp = const_cast<void *> (static_cast<const volatile void *> (p));
      ::new (vp) value_type (get_parent (), std::forward<Args> (args)...);
    }

    [[nodiscard]]
    value_type *
    allocate (std::size_t n)
    {
      return std::allocator<value_type> { }.allocate (n);
    }

    void
    deallocate (value_type *p, std::size_t n)
    {
      return std::allocator<value_type> { }.deallocate (p, n);
    }

    [[nodiscard]]
    constexpr
    Parent&
    get_parent (void) const noexcept
    {
      return *m_parent;
    }

  private:
    nonnull_ptr<Parent> m_parent;
  };

  template <typename Child, typename Parent>
  bool
  operator== (const strongly_linked_allocator<Child, Parent>& lhs,
              const strongly_linked_allocator<Child, Parent>& rhs)
  {
    return &lhs.get_parent () == &rhs.get_parent ();
  }

  template <typename Child, typename Parent>
  bool
  operator!= (const strongly_linked_allocator<Child, Parent>& lhs,
              const strongly_linked_allocator<Child, Parent>& rhs)
  {
    return ! (lhs == rhs);
  }

  template <typename Pair, typename Parent>
  class strongly_linked_map_allocator
  {
  public:
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap            = std::false_type;
    using is_always_equal                        = std::false_type;

    using value_type = Pair;

    strongly_linked_map_allocator            (void)                                     = delete;
    strongly_linked_map_allocator            (const strongly_linked_map_allocator&)     = default;
    strongly_linked_map_allocator            (strongly_linked_map_allocator&&) noexcept = default;
    strongly_linked_map_allocator& operator= (const strongly_linked_map_allocator&)     = default;
    strongly_linked_map_allocator& operator= (strongly_linked_map_allocator&&) noexcept = default;
    ~strongly_linked_map_allocator           (void)                                     = default;

    template <typename U>
    strongly_linked_map_allocator (const strongly_linked_map_allocator<U, Parent>& other) noexcept
      : m_parent (other.get_parent ())
    { }

    explicit
    strongly_linked_map_allocator (ir_def_timeline& parent) noexcept
      : m_parent (parent)
    { }

    template <typename U, typename KeyTuple, typename ChildTuple>
    void
    construct (U *p, std::piecewise_construct_t, KeyTuple&& key_tuple, ChildTuple&& child_tuple)
    {
      void *vp = const_cast<void *> (static_cast<const volatile void *> (p));
      ::new (vp) U (std::piecewise_construct,
                    std::forward<KeyTuple> (key_tuple),
                    std::tuple_cat (std::forward_as_tuple (get_parent ()),
                                    std::forward<ChildTuple> (child_tuple)));
    }

    template <typename U>
    void
    construct (U *p)
    {
      return construct (p,
                        std::piecewise_construct,
                        std::tuple<> { },
                        std::tuple<> { });
    }

    template <typename U, typename LHS, typename RHS>
    void
    construct (U *p, LHS&& lhs, RHS&& rhs)
    {
      return construct (p,
                        std::piecewise_construct,
                        std::forward_as_tuple (lhs),
                        std::forward_as_tuple (rhs));
    }

    template <typename U, typename LHS, typename RHS>
    void
    construct (U *p, const std::pair<LHS, RHS>& other)
    {
      return construct (p,
                        std::piecewise_construct,
                        std::forward_as_tuple (std::get<0> (other)),
                        std::forward_as_tuple (std::get<1> (other)));
    }

    template <typename U, typename LHS, typename RHS>
    void
    construct (U *p, std::pair<LHS, RHS>&& other)
    {
      return construct (p,
                        std::piecewise_construct,
                        std::forward_as_tuple (std::get<0> (std::move (other))),
                        std::forward_as_tuple (std::get<1> (std::move (other))));
    }

    [[nodiscard]]
    value_type *
    allocate (std::size_t n)
    {
      return std::allocator<value_type> { }.allocate (n);
    }

    void
    deallocate (value_type *p, std::size_t n)
    {
      return std::allocator<value_type> { }.deallocate (p, n);
    }

    [[nodiscard]]
    constexpr
    Parent&
    get_parent (void) const noexcept
    {
      return *m_parent;
    }

  private:
    nonnull_ptr<Parent> m_parent;
  };

  template <typename Child, typename Parent>
  bool
  operator== (const strongly_linked_map_allocator<Child, Parent>& lhs,
              const strongly_linked_map_allocator<Child, Parent>& rhs)
  {
    return &lhs.get_parent () == &rhs.get_parent ();
  }

  template <typename Child, typename Parent>
  bool
  operator!= (const strongly_linked_map_allocator<Child, Parent>& lhs,
              const strongly_linked_map_allocator<Child, Parent>& rhs)
  {
    return ! (lhs == rhs);
  }

}

#endif // OCTAVE_IR_UTILITIES_IR_ALLOCATOR_UTIL_HPP

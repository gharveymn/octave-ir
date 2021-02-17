/** ir-allocator-util.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_ALLOCATOR_UTIL_HPP
#define OCTAVE_IR_IR_ALLOCATOR_UTIL_HPP

#include <gch/nonnull_ptr.hpp>

namespace gch
{

  template <typename Child, typename Parent>
  class strongly_linked_allocator
  {
  public:
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
      ::new (vp) Child (get_parent (), std::forward<Args> (args)...);
    }

    [[nodiscard]]
    Child *
    allocate (std::size_t n)
    {
      return std::allocator<Child> { }.allocate (n);
    }

    void
    deallocate (Child* p, std::size_t n)
    {
      return std::allocator<Child> { }.deallocate (p, n);
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

}

#endif // OCTAVE_IR_IR_ALLOCATOR_UTIL_HPP

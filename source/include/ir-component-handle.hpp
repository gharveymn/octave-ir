/** ir-component-handle.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_COMPONENT_HANDLE_HPP
#define OCTAVE_IR_IR_COMPONENT_HANDLE_HPP

#if defined (__cpp_concepts) && __cpp_concepts >= 201907L
#  ifndef GCH_CONCEPTS
#    define GCH_CONCEPTS
#  endif
#  if defined(__has_include) && __has_include(<concepts>)
#    include <concepts>
#    if defined (__cpp_lib_concepts) && __cpp_lib_concepts >= 202002L
#      if ! defined (GCH_LIB_CONCEPTS) && ! defined (GCH_DISABLE_CONCEPTS)
#        define GCH_LIB_CONCEPTS
#      endif
#    endif
#  endif
#endif

#include "ir-optional-util.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>

#include <cassert>
#include <functional>
#include <iterator>
#include <memory>
#include <vector>

namespace gch
{
  class ir_component;
  class ir_component_handle;

  using ir_component_storage = std::unique_ptr<ir_component>;

  namespace detail
  {

    template <typename Storage, typename Component>
    class ir_component_handle_base
    {
    public:
      using storage_type = Storage;
      using storage_ptr  = storage_type *;
      using storage_ref  = storage_type&;

      using component_type = Component;
      using component_ptr  = component_type *;
      using component_ref  = component_type&;

      using observer = optional_ref<component_type>;

      ir_component_handle_base            (void)                                = default;
      ir_component_handle_base            (const ir_component_handle_base&)     = default;
      ir_component_handle_base            (ir_component_handle_base&&) noexcept = default;
      ir_component_handle_base& operator= (const ir_component_handle_base&)     = default;
      ir_component_handle_base& operator= (ir_component_handle_base&&) noexcept = default;
      ~ir_component_handle_base           (void)                                = default;

      constexpr explicit
      ir_component_handle_base (storage_ptr ptr)
        : m_storage_ptr (ptr)
      { }

      [[nodiscard]] constexpr
      storage_ptr
      get_storage_ptr (void) const noexcept
      {
        return m_storage_ptr;
      }

      [[nodiscard]] constexpr
      storage_ref
      get_storage (void) const noexcept
      {
        return *get_storage_ptr ();
      }

      [[nodiscard]] constexpr
      bool
      has_component (void) const noexcept
      {
        return bool (get_storage ());
      }

      [[nodiscard]] constexpr
      component_ptr
      get_component_pointer (void) const noexcept
      {
        return get_storage ().get ();
      }

      [[nodiscard]] constexpr
      observer
      maybe_get_component (void) const noexcept
      {
        return get_component_pointer ();
      }

      [[nodiscard]] constexpr
      component_ref
      get_component (void) const noexcept
      {
        return *get_storage ();
      }

      [[nodiscard]] constexpr
      explicit
      operator bool (void) const noexcept
      {
        return has_component ();
      }

      constexpr
      void
      swap (ir_component_handle_base& other) noexcept
      {
        using std::swap;
        swap (m_storage_ptr, other.m_storage_ptr);
      }

    protected:
      template <typename Integer>
      void
      advance (Integer count)
      {
        std::advance (m_storage_ptr, count);
      }

    private:
      storage_ptr m_storage_ptr;
    };

    template <typename Storage, typename Component>
    constexpr
    void
    swap (ir_component_handle_base<Storage, Component>& lhs,
          ir_component_handle_base<Storage, Component>& rhs) noexcept
    {
      lhs.swap (rhs);
    }

  }

  // mutable ir_component_storage -> mutable ir_component
  //
  // ir_component_storage  ir_component
  // mutable               mutable       T  [ir_component_handle]
  // mutable               const         F  <not allowed>
  // const                 mutable       T  [ir_component_ptr]
  // const                 const         T  [ir_component_const_ptr]

  // ir_component_handles are not nullable
  class ir_component_handle
    : private detail::ir_component_handle_base<ir_component_storage, ir_component>
  {
    using base = detail::ir_component_handle_base<ir_component_storage, ir_component>;

  public:
    ir_component_handle            (void)                           = delete;
    ir_component_handle            (const ir_component_handle&)     = default;
    ir_component_handle            (ir_component_handle&&) noexcept = default;
    ir_component_handle& operator= (const ir_component_handle&)     = default;
    ir_component_handle& operator= (ir_component_handle&&) noexcept = default;
    ~ir_component_handle           (void)                           = default;

    constexpr explicit
    ir_component_handle (storage_ref ref)
      : base (&ref)
    { }

    using base::get_storage;
    using base::get_storage_ptr;
    using base::swap;

    [[nodiscard]] constexpr
    storage_ref
    operator* (void) const
    {
      return get_storage ();
    }

    [[nodiscard]] constexpr
    storage_ptr
    operator-> (void) const noexcept
    {
      return get_storage_ptr ();
    }

    constexpr
    void
    swap (ir_component_handle& other)
    {
      base::swap (other);
    }
  };

  constexpr
  void
  swap (ir_component_handle& lhs, ir_component_handle& rhs) noexcept
  {
    lhs.swap (rhs);
  }

  class ir_component_mover
  {
  public:
    ir_component_mover            (void)                          = delete;
    ir_component_mover            (const ir_component_mover&)     = default;
    ir_component_mover            (ir_component_mover&&) noexcept = default;
    ir_component_mover& operator= (const ir_component_mover&)     = default;
    ir_component_mover& operator= (ir_component_mover&&) noexcept = default;
    ~ir_component_mover           (void)                          = default;

    constexpr explicit
    ir_component_mover (ir_component_storage& storage)
      : m_handle (storage)
    { }

    ir_component_mover (const ir_component_storage&& storage) = delete;

    [[nodiscard]] constexpr
    operator ir_component_storage&& (void) const
    {
      return std::move (m_handle.get_storage ());
    }

    [[nodiscard]] constexpr
    ir_component_handle
    get_handle (void) const noexcept
    {
      return m_handle;
    }

    constexpr
    void
    swap (ir_component_mover& other)
    {
      using std::swap;
      swap (m_handle, other.m_handle);
    }

  private:
    ir_component_handle m_handle;
  };

  constexpr
  void
  swap (ir_component_mover& lhs, ir_component_mover& rhs) noexcept
  {
    lhs.swap (rhs);
  }

  constexpr
  ir_component_mover
  make_mover (ir_component_handle handle)
  {
    return ir_component_mover { handle.get_storage () };
  }

  constexpr
  ir_component_mover
  make_mover (ir_component_storage& storage)
  {
    return ir_component_mover { storage };
  }

  constexpr
  ir_component_mover
  make_mover (const ir_component_storage&& storage) = delete;

  // a component view is basically a handle which references a const component
  template <typename Component>
  class ir_component_pointer
    : public detail::ir_component_handle_base<const ir_component_storage, Component>
  {
    using base = detail::ir_component_handle_base<const ir_component_storage, Component>;

  public:
    using storage_ptr       = typename base::storage_ptr;
    using component_type    = Component;

    using difference_type   = typename std::iterator_traits<storage_ptr>::difference_type;
    using value_type        = std::remove_cv_t<component_type>;
    using pointer           = component_type *;
    using reference         = component_type&;
    using iterator_category = typename std::iterator_traits<storage_ptr>::iterator_category;
#ifdef GCH_LIB_CONCEPTS
    using iterator_concept  = std::contiguous_iterator_tag;
#endif

    ir_component_pointer            (void)                            = default;
    ir_component_pointer            (const ir_component_pointer&)     = default;
    ir_component_pointer            (ir_component_pointer&&) noexcept = default;
    ir_component_pointer& operator= (const ir_component_pointer&)     = default;
    ir_component_pointer& operator= (ir_component_pointer&&) noexcept = default;
    ~ir_component_pointer           (void)                            = default;

    constexpr explicit
    ir_component_pointer (storage_ptr ptr)
      : base (ptr)
    { }

    constexpr /* implicit */
    ir_component_pointer (const ir_component_handle& handle)
      : ir_component_pointer (handle.get_storage_ptr ())
    { }

    template <typename U = Component, std::enable_if_t<std::is_const_v<U>> * = nullptr>
    constexpr /* implicit */
    ir_component_pointer (const ir_component_pointer<std::remove_const_t<U>>& other) noexcept
      : base (other.get_storage_ptr ())
    { }

    constexpr
    ir_component_pointer&
    operator+= (difference_type n) noexcept
    {
      base::advance (n);
      return *this;
    }

    constexpr
    ir_component_pointer&
    operator-= (difference_type n) noexcept
    {
      return *this += -n;
    }

    constexpr
    ir_component_pointer&
    operator++ (void) noexcept
    {
      return *this += 1;
    }

    constexpr
    ir_component_pointer
    operator++ (int) noexcept
    {
      ir_component_pointer ret { *this };
      base::advance (1);
      return ret;
    }

    constexpr
    ir_component_pointer&
    operator-- (void) noexcept
    {
      return *this += -1;
    }

    constexpr
    ir_component_pointer
    operator-- (int) noexcept
    {
      ir_component_pointer ret { *this };
      base::advance (-1);
      return ret;
    }

    constexpr
    ir_component_pointer
    operator+ (difference_type n) const noexcept
    {
      return ir_component_pointer (base::get_storage_ptr () + n);
    }

    constexpr
    ir_component_pointer
    operator- (difference_type n) const noexcept
    {
      return *this + -n;
    }

    [[nodiscard]] constexpr
    reference
    operator* (void) const
    {
      return *base::get_storage ();
    }

    [[nodiscard]] constexpr
    pointer
    operator-> (void) const noexcept
    {
      return base::get_component_pointer ();
    }

    constexpr
    reference
    operator[] (difference_type n) const noexcept
    {
      return *(*this + n);
    }

  };

  template <typename Component>
  constexpr
  ir_component_pointer<Component>
  operator+ (typename ir_component_pointer<Component>::difference_type n,
             const ir_component_pointer<Component>& it) noexcept
  {
    return it + n;
  }

  using ir_component_ptr  = ir_component_pointer<ir_component>;
  using ir_component_cptr = ir_component_pointer<const ir_component>;

  constexpr
  auto
  operator- (const ir_component_cptr& lhs, const ir_component_cptr& rhs) noexcept
  -> decltype (lhs.get_storage_ptr () - rhs.get_storage_ptr ())
  {
    return lhs.get_storage_ptr () - rhs.get_storage_ptr ();
  }

  template <typename T>
  constexpr
  optional_ref<T>
  maybe_get_as (ir_component_ptr comp)
  {
    assert (comp && "handle should be viewing a component");
    return maybe_cast<T> (comp.get_component_pointer ());
  }

  template <typename T>
  constexpr
  optional_ref<const T>
  maybe_get_as (ir_component_cptr comp)
  {
    assert (comp && "handle should be viewing a component");
    return maybe_cast<T> (comp.get_component_pointer ());
  }

  template <typename T>
  constexpr
  T&
  get_as (ir_component_ptr comp)
  {
    assert (comp && "handle should be viewing a component");
    return static_cast<T&> (*comp);
  }

  template <typename T>
  constexpr
  const T&
  get_as (ir_component_cptr comp)
  {
    assert (comp && "handle should be viewing a component");
    return static_cast<const T&> (*comp);
  }

  constexpr
  bool
  operator== (const ir_component_cptr& lhs, const ir_component_cptr& rhs)
  {
    return lhs.get_storage_ptr () == rhs.get_storage_ptr ();
  }

  constexpr
  bool
  operator!= (const ir_component_cptr& lhs, const ir_component_cptr& rhs)
  {
    return ! (lhs == rhs);
  }

  constexpr
  bool
  operator< (const ir_component_cptr& lhs, const ir_component_cptr& rhs)
  {
    return std::less<ir_component_cptr::storage_ptr> { } (lhs.get_storage_ptr (),
                                                          rhs.get_storage_ptr ());
  }

  constexpr
  bool
  operator<= (const ir_component_cptr& lhs, const ir_component_cptr& rhs)
  {
    return ! (rhs < lhs);
  }

  constexpr
  bool
  operator> (const ir_component_cptr& lhs, const ir_component_cptr& rhs)
  {
    return rhs < lhs;
  }

  constexpr
  bool
  operator>= (const ir_component_cptr& lhs, const ir_component_cptr& rhs)
  {
    return ! (lhs < rhs);
  }

  constexpr
  bool
  operator== (const ir_component_cptr& lhs, const ir_component_cptr::observer& rhs)
  {
    return lhs.maybe_get_component () == rhs;
  }

  constexpr
  bool
  operator== (const ir_component_cptr::observer& lhs, const ir_component_cptr& rhs)
  {
    return lhs == rhs.maybe_get_component ();
  }

  constexpr
  bool
  operator!= (const ir_component_cptr& lhs, const ir_component_cptr::observer& rhs)
  {
    return ! (lhs == rhs);
  }

  constexpr
  bool
  operator!= (const ir_component_cptr::observer& lhs, const ir_component_cptr& rhs)
  {
    return ! (lhs == rhs);
  }

  constexpr
  bool
  operator< (const ir_component_cptr& lhs, const ir_component_cptr::observer& rhs)
  {
    return std::less<ir_component_cptr::observer> { } (lhs.maybe_get_component (), rhs);
  }

  constexpr
  bool
  operator< (const ir_component_cptr::observer& lhs, const ir_component_cptr& rhs)
  {
    return std::less<ir_component_cptr::observer> { } (lhs, rhs.maybe_get_component ());
  }

  constexpr
  bool
  operator<= (const ir_component_cptr& lhs, const ir_component_cptr::observer& rhs)
  {
    return ! (rhs < lhs);
  }

  constexpr
  bool
  operator<= (const ir_component_cptr::observer& lhs, const ir_component_cptr& rhs)
  {
    return ! (rhs < lhs);
  }

  constexpr
  bool
  operator> (const ir_component_cptr& lhs, const ir_component_cptr::observer& rhs)
  {
    return rhs < lhs;
  }

  constexpr
  bool
  operator> (const ir_component_cptr::observer& lhs, const ir_component_cptr& rhs)
  {
    return rhs < lhs;
  }

  constexpr
  bool
  operator>= (const ir_component_cptr& lhs, const ir_component_cptr::observer& rhs)
  {
    return ! (lhs < rhs);
  }

  constexpr
  bool
  operator>= (const ir_component_cptr::observer& lhs, const ir_component_cptr& rhs)
  {
    return ! (lhs < rhs);
  }

  template <typename T>
  bool
  holds_type (const ir_component_cptr& comp)
  {
    return dynamic_cast<const T*> (comp.get_component_pointer ()) != nullptr;
  }

#ifdef GCH_CONCEPTS
  static_assert (std::contiguous_iterator<ir_component_ptr>);
#endif

  [[nodiscard]]
  inline
  ir_component_ptr
  make_ptr (const std::vector<ir_component_storage>::iterator it)
  {
    return ir_component_ptr { it.operator-> () };
  }

  [[nodiscard]]
  inline
  ir_component_cptr
  make_ptr (const std::vector<ir_component_storage>::const_iterator cit)
  {
    return ir_component_cptr { cit.operator-> () };
  }

}

namespace std
{

  template <>
  struct hash<gch::ir_component_handle>
  {
    std::size_t
    operator() (const gch::ir_component_handle& comp) const noexcept
    {
      return std::hash<const gch::ir_component_storage *> { } (comp.get_storage_ptr ());
    }
  };

  template <>
  struct hash<gch::ir_component_ptr>
  {
    std::size_t
    operator() (const gch::ir_component_ptr& comp) const noexcept
    {
      return std::hash<const gch::ir_component_storage *> { } (comp.get_storage_ptr ());
    }
  };

  template <>
  struct hash<gch::ir_component_cptr>
  {
    std::size_t
    operator() (const gch::ir_component_cptr& comp) const noexcept
    {
      return std::hash<const gch::ir_component_storage *> { } (comp.get_storage_ptr ());
    }
  };

}

#endif // OCTAVE_IR_IR_COMPONENT_HANDLE_HPP

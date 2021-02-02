/** ir-component-handle.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_COMPONENT_HANDLE_HPP
#define OCTAVE_IR_IR_COMPONENT_HANDLE_HPP

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>

#include <cassert>
#include <functional>
#include <memory>

namespace gch
{
  class ir_component;
  class ir_component_handle;

  using ir_component_storage = std::unique_ptr<ir_component>;

  template <typename T, typename... Args>
  inline
  ir_component_storage
  make_ir_component (Args&&... args)
  {
    return std::make_unique<T> (std::forward<Args> (args)...);
  }

  namespace detail
  {

    template <typename Component>
    class ir_component_handle_base
    {
      using storage_type = ir_component_storage;

    public:
      using value_type   = std::remove_cv_t<Component>;
      using element_type = Component;
      using pointer      = element_type *;
      using reference    = element_type&;
      using observer     = optional_ref<element_type>;

      ir_component_handle_base            (void)                                = delete;
      ir_component_handle_base            (const ir_component_handle_base&)     = default;
      ir_component_handle_base            (ir_component_handle_base&&) noexcept = default;
      ir_component_handle_base& operator= (const ir_component_handle_base&)     = default;
      ir_component_handle_base& operator= (ir_component_handle_base&&) noexcept = default;
      ~ir_component_handle_base           (void)                                = default;

      constexpr /* implicit */
      ir_component_handle_base (const storage_type& storage_ref)
        : m_storage_view (storage_ref)
      { }

      [[nodiscard]] constexpr
      bool
      has_component (void) const noexcept
      {
        return bool (get_storage ());
      }

      [[nodiscard]] constexpr
      const storage_type&
      get_storage (void) const noexcept
      {
        return *m_storage_view;
      }

      [[nodiscard]] constexpr
      pointer
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
      reference
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

      [[nodiscard]] constexpr
      reference
      operator* (void) const
      {
        return get_component ();
      }

      [[nodiscard]] constexpr
      pointer
      operator-> (void) const noexcept
      {
        return get_component_pointer ();
      }

      constexpr
      void
      swap (ir_component_handle_base& other) noexcept
      {
        m_storage_view.swap (other.m_storage_view);
      }

    private:
      nonnull_ptr<const storage_type> m_storage_view;
    };

  }

  class ir_component_handle
    : public detail::ir_component_handle_base<ir_component>
  {
    using base = detail::ir_component_handle_base<ir_component>;

  public:
    ir_component_handle            (void)                           = delete;
    ir_component_handle            (const ir_component_handle&)     = default;
    ir_component_handle            (ir_component_handle&&) noexcept = default;
    ir_component_handle& operator= (const ir_component_handle&)     = default;
    ir_component_handle& operator= (ir_component_handle&&) noexcept = default;
    ~ir_component_handle           (void)                           = default;

    using base::ir_component_handle_base;

    static_assert (std::is_same_v<reference, ir_component&>);
  };

  constexpr
  void
  swap (ir_component_handle& lhs, ir_component_handle& rhs) noexcept
  {
    lhs.swap (rhs);
  }

  template <typename T>
  constexpr
  optional_ref<T>
  maybe_get_as (ir_component_handle comp)
  {
    assert (comp && "handle should be viewing a component");
    return { dynamic_cast<T *> (comp.get_component_pointer ()) };
  }

  template <typename T>
  constexpr
  T&
  get_as (ir_component_handle comp)
  {
    assert (comp && "handle should be viewing a component");
    return static_cast<T&> (*comp);
  }

  // a component view is basically a handle which references a const component
  class ir_component_view
    : public detail::ir_component_handle_base<const ir_component>
  {
    using base = detail::ir_component_handle_base<const ir_component>;

  public:
    ir_component_view            (void)                         = delete;
    ir_component_view            (const ir_component_view&)     = default;
    ir_component_view            (ir_component_view&&) noexcept = default;
    ir_component_view& operator= (const ir_component_view&)     = default;
    ir_component_view& operator= (ir_component_view&&) noexcept = default;
    ~ir_component_view           (void)                         = default;

    using base::ir_component_handle_base;

    constexpr /* implicit */
    ir_component_view (const ir_component_handle& other)
      : base (other.get_storage ())
    { }

    static_assert (std::is_same_v<reference, const ir_component&>);
  };

  constexpr
  bool
  operator== (const ir_component_view& lhs, const ir_component_view& rhs)
  {
    return lhs.maybe_get_component () == rhs.maybe_get_component ();
  }

  constexpr
  bool
  operator!= (const ir_component_view& lhs, const ir_component_view& rhs)
  {
    return ! (lhs == rhs);
  }

  constexpr
  bool
  operator< (const ir_component_view& lhs, const ir_component_view& rhs)
  {
    return std::less<ir_component_view::observer> { } (lhs.maybe_get_component (),
                                                         rhs.maybe_get_component ());
  }

  constexpr
  bool
  operator<= (const ir_component_view& lhs, const ir_component_view& rhs)
  {
    return ! (rhs < lhs);
  }

  constexpr
  bool
  operator> (const ir_component_view& lhs, const ir_component_view& rhs)
  {
    return rhs < lhs;
  }

  constexpr
  bool
  operator>= (const ir_component_view& lhs, const ir_component_view& rhs)
  {
    return ! (lhs < rhs);
  }

  constexpr
  bool
  operator== (const ir_component_view& lhs, nullopt_t) noexcept
  {
    return ! lhs;
  }

  constexpr
  bool
  operator== (nullopt_t, const ir_component_view& rhs) noexcept
  {
    return ! rhs;
  }

  constexpr
  bool
  operator!= (const ir_component_view& lhs, nullopt_t) noexcept
  {
    return lhs.has_component ();
  }

  constexpr
  bool
  operator!= (nullopt_t, const ir_component_view& rhs) noexcept
  {
    return rhs.has_component ();
  }

  constexpr
  bool
  operator< (const ir_component_view& lhs, nullopt_t)
  {
    return std::less<ir_component_view::observer> { } (lhs.maybe_get_component (), nullopt);
  }

  constexpr
  bool
  operator< (nullopt_t, const ir_component_view& rhs)
  {
    return std::less<ir_component_view::observer> { } (nullopt, rhs.maybe_get_component ());
  }

  constexpr
  bool
  operator<= (const ir_component_view& lhs, nullopt_t)
  {
    return ! (nullptr < lhs);
  }

  constexpr
  bool
  operator<= (nullopt_t, const ir_component_view& rhs)
  {
    return ! (rhs < nullptr);
  }

  constexpr
  bool
  operator> (const ir_component_view& lhs, nullopt_t)
  {
    return nullptr < lhs;
  }

  constexpr
  bool
  operator> (nullopt_t, const ir_component_view& rhs)
  {
    return rhs < nullptr;
  }

  constexpr
  bool
  operator>= (const ir_component_view& lhs, nullopt_t)
  {
    return ! (lhs < nullptr);
  }

  constexpr
  bool
  operator>= (nullopt_t, const ir_component_view& rhs)
  {
    return ! (nullptr < rhs);
  }

  constexpr
  bool
  operator== (const ir_component_view& lhs, const ir_component_view::observer& rhs)
  {
    return lhs.maybe_get_component () == rhs;
  }

  constexpr
  bool
  operator== (const ir_component_view::observer& lhs, const ir_component_view& rhs)
  {
    return lhs == rhs.maybe_get_component ();
  }

  constexpr
  bool
  operator!= (const ir_component_view& lhs, const ir_component_view::observer& rhs)
  {
    return ! (lhs == rhs);
  }

  constexpr
  bool
  operator!= (const ir_component_view::observer& lhs, const ir_component_view& rhs)
  {
    return ! (lhs == rhs);
  }

  constexpr
  bool
  operator< (const ir_component_view& lhs, const ir_component_view::observer& rhs)
  {
    return std::less<ir_component_view::observer> { } (lhs.maybe_get_component (), rhs);
  }

  constexpr
  bool
  operator< (const ir_component_view::observer& lhs, const ir_component_view& rhs)
  {
    return std::less<ir_component_view::observer> { } (lhs, rhs.maybe_get_component ());
  }

  constexpr
  bool
  operator<= (const ir_component_view& lhs, const ir_component_view::observer& rhs)
  {
    return ! (rhs < lhs);
  }

  constexpr
  bool
  operator<= (const ir_component_view::observer& lhs, const ir_component_view& rhs)
  {
    return ! (rhs < lhs);
  }

  constexpr
  bool
  operator> (const ir_component_view& lhs, const ir_component_view::observer& rhs)
  {
    return rhs < lhs;
  }

  constexpr
  bool
  operator> (const ir_component_view::observer& lhs, const ir_component_view& rhs)
  {
    return rhs < lhs;
  }

  constexpr
  bool
  operator>= (const ir_component_view& lhs, const ir_component_view::observer& rhs)
  {
    return ! (lhs < rhs);
  }

  constexpr
  bool
  operator>= (const ir_component_view::observer& lhs, const ir_component_view& rhs)
  {
    return ! (lhs < rhs);
  }

  template <typename T>
  bool
  contains (const ir_component_view& comp)
  {
    return dynamic_cast<const T*> (comp.get_component_pointer ()) != nullptr;
  }

  constexpr
  void
  swap (ir_component_view& lhs, ir_component_view& rhs) noexcept
  {
    lhs.swap (rhs);
  }

  template <typename T>
  constexpr
  optional_ref<const T>
  maybe_get_as (ir_component_view comp)
  {
    assert (comp && "handle should be viewing a component");
    return { dynamic_cast<T *> (comp.get_component_pointer ()) };
  }

  template <typename T>
  constexpr
  const T&
  get_as (ir_component_view comp)
  {
    assert (comp && "handle should be viewing a component");
    return static_cast<const T&> (*comp);
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
      return std::hash<gch::ir_component_storage> { } (comp.get_storage ());
    }
  };

}

#endif // OCTAVE_IR_IR_COMPONENT_HANDLE_HPP

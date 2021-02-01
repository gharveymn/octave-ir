/** ir-component-handle.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_COMPONENT_HANDLE_HPP
#define OCTAVE_IR_IR_COMPONENT_HANDLE_HPP

#include <gch/optional_ref.hpp>

#include <functional>
#include <memory>

namespace gch
{
  class ir_component;
  class ir_component_handle;

  template <typename T, typename... Args>
  inline
  ir_component_handle
  make_ir_component (Args&&... args);

  class ir_component_handle
  {
    using storage_type = std::unique_ptr<ir_component>;

  public:
    friend std::hash<ir_component_handle>;

    using pointer        = storage_type::pointer;
    using element_type   = storage_type::element_type;
    using deleter_type   = storage_type::deleter_type;

    using reference       = element_type&;
    using const_reference = const element_type&;
    using observer        = decltype (make_optional_ref (std::declval<reference> ()));
    using const_observer  = decltype (make_optional_ref (std::declval<const_reference> ()));

    ir_component_handle            (void)                           = default;
    ir_component_handle            (const ir_component_handle&)     = delete;
    ir_component_handle            (ir_component_handle&&) noexcept = default;
    ir_component_handle& operator= (const ir_component_handle&)     = delete;
    ir_component_handle& operator= (ir_component_handle&&) noexcept = default;
    ~ir_component_handle           (void)                           = default;

  private:
    template <typename U, typename E>
    /* implicit */
    ir_component_handle (std::unique_ptr<U, E>&& other) noexcept
      : m_storage (std::move (other))
    { }

  public:
    ir_component_handle&
    operator= (std::nullptr_t) noexcept
    {
      m_storage = nullptr;
      return *this;
    }

    observer
    release (void) noexcept
    {
      return m_storage.release ();
    }

    void
    reset (void) noexcept
    {
      m_storage.reset ();
    }

    void
    swap (ir_component_handle& other) noexcept
    {
      using std::swap;
      swap (m_storage, other.m_storage);
    }

    [[nodiscard]]
    observer
    get (void) const noexcept
    {
      return m_storage.get ();
    }

    [[nodiscard]]
    deleter_type&
    get_deleter (void) noexcept
    {
      return m_storage.get_deleter ();
    }

    [[nodiscard]]
    const deleter_type&
    get_deleter (void) const noexcept
    {
      return m_storage.get_deleter ();
    }

    explicit
    operator bool (void) const noexcept
    {
      return bool (m_storage);
    }

    reference
    operator* (void) const
    {
      return *m_storage;
    }

    pointer
    operator-> (void) const noexcept
    {
      return m_storage.operator-> ();
    }

    pointer
    get_pointer (void) const noexcept
    {
      return operator-> ();
    }

    template <typename T, typename ...Args>
    friend inline
    ir_component_handle
    make_ir_component (Args&&... args)
    {
      return std::make_unique<T> (std::forward<Args> (args)...);
    }

  private:
    storage_type m_storage;
  };

  bool
  operator== (const ir_component_handle& lhs, const ir_component_handle& rhs)
  {
    return lhs.get () == rhs.get ();
  }

  bool
  operator!= (const ir_component_handle& lhs, const ir_component_handle& rhs)
  {
    return ! (lhs == rhs);
  }

  bool
  operator< (const ir_component_handle& lhs, const ir_component_handle& rhs)
  {
    return std::less<ir_component_handle::observer> { } (lhs.get (), rhs.get ());
  }

  bool
  operator<= (const ir_component_handle& lhs, const ir_component_handle& rhs)
  {
    return ! (rhs < lhs);
  }

  bool
  operator> (const ir_component_handle& lhs, const ir_component_handle& rhs)
  {
    return rhs < lhs;
  }

  bool
  operator>= (const ir_component_handle& lhs, const ir_component_handle& rhs)
  {
    return ! (lhs < rhs);
  }

  bool
  operator== (const ir_component_handle& lhs, std::nullptr_t) noexcept
  {
    return ! lhs;
  }

  bool
  operator== (std::nullptr_t, const ir_component_handle& rhs) noexcept
  {
    return ! rhs;
  }

  bool
  operator!= (const ir_component_handle& lhs, std::nullptr_t) noexcept
  {
    return bool (lhs);
  }

  bool
  operator!= (std::nullptr_t, const ir_component_handle& rhs) noexcept
  {
    return bool (rhs);
  }

  bool
  operator< (const ir_component_handle& lhs, std::nullptr_t)
  {
    return std::less<ir_component_handle::observer> { } (lhs.get (), nullptr);
  }

  bool
  operator< (std::nullptr_t, const ir_component_handle& rhs)
  {
    return std::less<ir_component_handle::observer> { } (nullptr, rhs.get ());
  }

  bool
  operator<= (const ir_component_handle& lhs, std::nullptr_t)
  {
    return ! (nullptr < lhs);
  }

  bool
  operator<= (std::nullptr_t, const ir_component_handle& rhs)
  {
    return ! (rhs < nullptr);
  }

  bool
  operator> (const ir_component_handle& lhs, std::nullptr_t)
  {
    return nullptr < lhs;
  }

  bool
  operator> (std::nullptr_t, const ir_component_handle& rhs)
  {
    return rhs < nullptr;
  }

  bool
  operator>= (const ir_component_handle& lhs, std::nullptr_t)
  {
    return ! (lhs < nullptr);
  }

  bool
  operator>= (std::nullptr_t, const ir_component_handle& rhs)
  {
    return ! (nullptr < rhs);
  }

  bool
  operator== (const ir_component_handle& lhs, const ir_component_handle::const_observer& rhs)
  {
    return lhs.get () == rhs;
  }

  bool
  operator== (const ir_component_handle::const_observer& lhs, const ir_component_handle& rhs)
  {
    return lhs == rhs.get ();
  }

  bool
  operator!= (const ir_component_handle& lhs, const ir_component_handle::const_observer& rhs)
  {
    return ! (lhs == rhs);
  }

  bool
  operator!= (const ir_component_handle::const_observer& lhs, const ir_component_handle& rhs)
  {
    return ! (lhs == rhs);
  }

  bool
  operator< (const ir_component_handle& lhs, const ir_component_handle::const_observer& rhs)
  {
    return std::less<ir_component_handle::const_observer> { } (lhs.get (), rhs);
  }

  bool
  operator< (const ir_component_handle::const_observer& lhs, const ir_component_handle& rhs)
  {
    return std::less<ir_component_handle::const_observer> { } (lhs, rhs.get ());
  }

  bool
  operator<= (const ir_component_handle& lhs, const ir_component_handle::const_observer& rhs)
  {
    return ! (rhs < lhs);
  }

  bool
  operator<= (const ir_component_handle::const_observer& lhs, const ir_component_handle& rhs)
  {
    return ! (rhs < lhs);
  }

  bool
  operator> (const ir_component_handle& lhs, const ir_component_handle::const_observer& rhs)
  {
    return rhs < lhs;
  }

  bool
  operator> (const ir_component_handle::const_observer& lhs, const ir_component_handle& rhs)
  {
    return rhs < lhs;
  }

  bool
  operator>= (const ir_component_handle& lhs, const ir_component_handle::const_observer& rhs)
  {
    return ! (lhs < rhs);
  }

  bool
  operator>= (const ir_component_handle::const_observer& lhs, const ir_component_handle& rhs)
  {
    return ! (lhs < rhs);
  }

  void
  swap (ir_component_handle& lhs, ir_component_handle& rhs) noexcept
  {
    lhs.swap (rhs);
  }

  template <typename T>
  optional_ref<T>
  maybe_cast_to (const ir_component_handle& comp)
  {
    return { dynamic_cast<T *> (comp.get_pointer ()) };
  }

  template <typename T>
  T&
  cast_to (const ir_component_handle& comp)
  {
    return static_cast<T&> (*comp);
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
      return std::hash<gch::ir_component_handle::storage_type> { } (comp.m_storage);
    }
  };

}

#endif // OCTAVE_IR_IR_COMPONENT_HANDLE_HPP

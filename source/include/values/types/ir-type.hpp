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

#if ! defined (octave_ir_type_h)
#define octave_ir_type_h 1

#ifndef GCH_CPP20_CONSTEVAL
#  if defined (__cpp_consteval) && __cpp_consteval >= 201811L
#    define GCH_CPP20_CONSTEVAL consteval
#  else
#    define GCH_CPP20_CONSTEVAL constexpr
#  endif
#endif

#include <type_traits>
#include <gch/nonnull_ptr.hpp>

class octave_base_value;

namespace gch
{

  using any = octave_base_value *;

  template <typename>
  struct ir_printer;

  class ir_type;

  class ir_type_array
  {

  public:

    using reference = const ir_type&;
    using iterator = const ir_type *;

    constexpr ir_type_array (void) noexcept
      : m_array {nullptr},
        m_numel {0}
    { }

    template <std::size_t N>
    explicit constexpr ir_type_array (const ir_type (&members)[N]) noexcept
      : m_array (members),
        m_numel (N)
    { }

    [[nodiscard]]
    constexpr std::size_t get_size (void) const noexcept
    {
      return sum_size (m_numel);
    }

    [[nodiscard]]
    constexpr iterator begin () const noexcept;

    [[nodiscard]]
    constexpr iterator end () const noexcept;

    constexpr reference operator[] (std::size_t n) const noexcept;

    [[nodiscard]]
    constexpr std::size_t get_numel (void) const noexcept { return m_numel; }

  private:

    [[nodiscard]]
    constexpr size_t sum_size (std::size_t n) const noexcept;

    const ir_type *m_array;
    std::size_t m_numel;
  };

  class ir_type
  {
    struct impl
    {
      // A user readable type name (this is the base name if it is a pointer).
      // This should not be used directly. Use ir_type_printer to print a name.
      const char *  m_name_base;
      const impl *  m_base_type;
      const impl *  m_pointer_base_type;
      std::size_t   m_rep_size;
      bool          m_is_integral;
      ir_type_array m_members;
    };

    template <typename ...T>
    struct instance;

    constexpr ir_type (const impl& impl_ref) noexcept
      : m_ptr (impl_ref)
    { }

  public:
    ir_type            (void)               = delete;
    ir_type            (const ir_type&)     = default;
    ir_type            (ir_type&&) noexcept = default;
    ir_type& operator= (const ir_type&)     = default;
    ir_type& operator= (ir_type&&) noexcept = default;
    ~ir_type           (void)               = default;

    [[nodiscard]] constexpr const char *  get_name_base    (void) const noexcept { return m_ptr->m_name_base;          }
    [[nodiscard]] constexpr ir_type       get_base         (void) const noexcept { return *m_ptr->m_base_type;         }
    [[nodiscard]] constexpr ir_type       get_pointer_base (void) const noexcept { return *m_ptr->m_pointer_base_type; }
    [[nodiscard]] constexpr std::size_t   get_size         (void) const noexcept { return m_ptr->m_rep_size;           }
    [[nodiscard]] constexpr bool          is_integral      (void) const noexcept { return m_ptr->m_is_integral;        }
    [[nodiscard]] constexpr ir_type_array get_members      (void) const noexcept { return m_ptr->m_members;            }

    [[nodiscard]] constexpr bool has_base         (void) const noexcept { return m_ptr->m_base_type         != nullptr; }
    [[nodiscard]] constexpr bool has_pointer_base (void) const noexcept { return m_ptr->m_pointer_base_type != nullptr; }

    template <typename ...Ts>
    static constexpr ir_type get (void) noexcept
    {
      return { instance<std::remove_cv_t<Ts>...>::m_impl };
    }

    friend constexpr bool operator== (const ir_type& lhs, const ir_type& rhs) noexcept
    {
      return lhs.m_ptr == rhs.m_ptr;
    }

  private:
    template <typename T>
    static constexpr impl create_type (const char *name, std::nullptr_t) noexcept
    {
      return { name, nullptr, nullptr, sizeof (T), std::is_integral_v<T>, { } };
    }

    template <typename T>
    static constexpr impl create_type (const char *name, ir_type base) noexcept
    {
      return { name, base.m_ptr, nullptr, sizeof (T), std::is_integral_v<T>, { } };
    }

    // shortcut for ir_type_impl::create_compound_type
    template <typename T, std::size_t N>
    static constexpr impl create_compound_type (const char *name, const ir_type (&members)[N],
                                                ir_type base) noexcept
    {
      return { name, base.m_ptr, nullptr, sizeof (T), std::is_integral_v<T>,
               ir_type_array (members) };
    }

    template <typename T>
    static constexpr impl create_type (const char *name) noexcept;

    template <typename T, std::size_t N>
    static constexpr impl create_compound_type (const char *name,
                                                const ir_type (&members)[N]) noexcept;

    template <typename T>
    static constexpr impl create_pointer (ir_type pointer_base) noexcept;

    nonnull_ptr<const impl> m_ptr;
  };

  template <typename ...Ts>
  inline constexpr ir_type ir_type_v = ir_type::get<Ts...> ();

  [[nodiscard]]
  constexpr std::size_t depth (ir_type ty) noexcept
  {
    return ty.has_base () ? (1 + depth (ty.get_base ())) : 0;
  }

  [[nodiscard]]
  constexpr std::size_t indirection_level (ir_type ty) noexcept
  {
    return ty.has_pointer_base () ? (1 + indirection_level (ty.get_pointer_base ())) : 0;
  }

  constexpr bool operator!= (ir_type lhs, ir_type rhs)  noexcept
  {
    return ! (lhs == rhs);
  }

  constexpr ir_type_array::iterator
  ir_type_array::begin () const noexcept
  {
    return m_array;
  }

  constexpr ir_type_array::iterator
  ir_type_array::end () const noexcept
  {
    return m_array + m_numel;
  }

  constexpr const ir_type&
  ir_type_array::operator[] (std::size_t n) const noexcept
  {
    return m_array[n];
  }

  constexpr size_t ir_type_array::sum_size (std::size_t n) const noexcept
  {
    return n > 0 ? sum_size (n - 1) + m_array[n - 1].get_size () : 0;
  }

  /////////
  // any //
  /////////
  // This is the top type. All types which are defined without a parent will
  // use this as a parent. This is to ensure interoperability of all types
  // even if they are not compatible. We also want to forward the conversion
  // operations which are not basic to octave_base_value.

  template <>
  struct ir_type::instance<any>
  {
    using type = any;
    static constexpr impl m_impl { create_type<any> ("any", nullptr) };
  };

  template <typename T>
  constexpr ir_type::impl
  ir_type::create_type (const char *name) noexcept
  {
    return create_type<T> (name, ir_type_v<any>);
  }

  template <typename T, std::size_t N>
  constexpr ir_type::impl
  ir_type::create_compound_type (const char *name, const ir_type (&members)[N]) noexcept
  {
    return create_compound_type<T, N> (name, members, ir_type_v<any>);
  }

  template <typename T>
  constexpr ir_type::impl
  ir_type::create_pointer (ir_type pointer_base) noexcept
  {
    return { pointer_base.get_name_base (), ir_type_v<any>.m_ptr, pointer_base.m_ptr, sizeof (T),
             false, { }};
  }

  // template instantiations
  // to define an ir_type_p implement a specialization of ir_type_instance
  // copying the structure seen below. For examples, see
  // ir_type_instance<any> for base types, ir_type_instance<Complex> for
  // struct types, and ir_type_instance<any *> for pointer types.

  //////////
  // void //
  //////////
  // This is the unit type; the only type which is not a descendent of 'any'.

  struct unit { };

  template <>
  struct ir_type::instance<void>
  {
    using type = unit;
    static constexpr impl m_impl { create_type<type> ("void", nullptr) };
  };

  /////////////////////////////////
  // pointer type instantiations //
  /////////////////////////////////

  template <typename T>
  struct ir_type::instance<T *>
  {
    using type = T *;
    static constexpr impl m_impl { create_pointer<T *> (ir_type_v<T>) };
  };

  //! Compute the lowest common ancestor between the two types.
  //!
  //! @param lhs an ir_type
  //! @param rhs another ir_type
  //! @return the lowest common ancestor
  GCH_CPP20_CONSTEVAL
  ir_type
  lca (ir_type lhs, ir_type rhs) noexcept
  {
    if (lhs == rhs)
      return lhs;

    if (depth (lhs) < depth (rhs) && rhs.has_base ())
      return lca (lhs, rhs.get_base ());

    if (depth (lhs) > depth (rhs) && lhs.has_base ())
      return lca (lhs.get_base (), rhs);

    if (! lhs.has_base () || ! rhs.has_base ())
      return ir_type_v<void>;

    return lca (lhs.get_base (), rhs.get_base ());
  }

  GCH_CPP20_CONSTEVAL
  ir_type
  operator^ (ir_type lhs, ir_type rhs) noexcept
  {
    return lca (lhs, rhs);
  }

}

#endif

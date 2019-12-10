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

///////////////
// NO MACROS //
///////////////
// Macros obfuscate the code, and make it annoying to debug.
// Please do not replace the boilerplate below with macros.
// Templates are ok.

#include "octave-config.h"

#include <type_traits>

class octave_base_value;

namespace octave
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

    constexpr std::size_t get_size (void) const noexcept
    {
      return sum_size (m_numel);
    }

    constexpr iterator begin () const noexcept;

    constexpr iterator end () const noexcept;

    constexpr reference operator[] (std::size_t n) const noexcept;

    constexpr std::size_t get_numel (void) const noexcept { return m_numel; }

  private:
    constexpr size_t sum_size (std::size_t n) const noexcept;

    const ir_type *m_array;
    std::size_t m_numel;
  };

  class ir_type
  {
    template <typename ...T>
    struct instance;

    struct impl;
    using impl_p = const impl *;

  public:

    ir_type (void) = delete;

    constexpr std::size_t get_size (void) const noexcept;

    constexpr bool is_integral (void) const noexcept;

    constexpr ir_type get_parent_type (void) const noexcept;

    constexpr ir_type get_dereference_type (void) const noexcept;

    constexpr std::size_t get_depth (void) const noexcept;

    constexpr std::size_t get_indirection_level (void) const noexcept;

    //! Compute the lowest common ancestor between the two types.
    //!
    //! @param ty1 an ir_type
    //! @param ty2 another ir_type
    //! @return the lowest common ancestor
    static constexpr ir_type lca (const ir_type& ty1, const ir_type& ty2)
    {
      return do_lca (ty1.m_ptr, ty2.m_ptr);
    }

    constexpr ir_type operator^ (const ir_type& ty) const
    {
      return do_lca (m_ptr, ty.m_ptr);
    }

    template <typename ...T>
    static constexpr ir_type get (void) noexcept
    {
      return { &instance<std::remove_cv_t<T>...>::m_impl };
    }

    constexpr bool operator== (const ir_type& ty) const noexcept
    {
      return m_ptr == ty.m_ptr;
    }

    constexpr bool operator== (std::nullptr_t) const noexcept
    {
      return m_ptr == nullptr;
    }
    
    friend constexpr bool
    operator== (std::nullptr_t, const ir_type& ty) noexcept
    {
      return ty == nullptr;
    }

    constexpr bool operator!= (const ir_type& ty) const noexcept
    {
      return m_ptr != ty.m_ptr;
    }

    constexpr bool operator!= (std::nullptr_t) const noexcept
    {
      return m_ptr != nullptr;
    }
  
    friend constexpr bool
    operator!= (std::nullptr_t, const ir_type& ty) noexcept
    {
      return ty != nullptr;
    }

    friend struct ir_printer<ir_type>;

  private:
    
    //! Implicit!
    constexpr ir_type (impl_p impl_ptr) noexcept
      : m_ptr (impl_ptr)
    { }

    static constexpr impl_p do_lca (impl_p ty1, impl_p ty2);

    // shortcut for ir_type_impl::create_type
    template <typename T>
    static constexpr impl
    create_type (const char *name) noexcept;

    template <typename T>
    static constexpr impl
    create_type (const char *name, ir_type parent) noexcept;

    // shortcut for ir_type_impl::create_compound_type
    template <typename T, std::size_t N>
    static constexpr impl
    create_compound_type (const char *name,
                          const ir_type (&members)[N]) noexcept;

    // shortcut for ir_type_impl::create_compound_type
    template <typename T, std::size_t N>
    static constexpr impl
    create_compound_type (const char *name, const ir_type (&members)[N],
                          ir_type parent) noexcept;

    impl_p m_ptr;

  };

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

  struct ir_type::impl
  {
  public:

    constexpr const char * get_base_name (void) const noexcept
    {
      return m_base_name;
    }

    // An abstract base type which may be null
    constexpr impl_p get_parent_p (void) const noexcept
    {
      return m_parent.m_ptr;
    }

    constexpr impl_p get_dereference_p (void) const noexcept
    {
      return m_ptr_to.m_ptr;
    }

    constexpr std::size_t get_size (void) const noexcept
    {
      return m_rep_size;
    }

    constexpr bool is_integral (void) const noexcept
    {
      return m_is_integral;
    }

    constexpr const ir_type_array& get_members (void) const noexcept
    {
      return m_members;
    }

    constexpr std::size_t get_depth (void) const noexcept
    {
      return m_parent != nullptr ? m_parent.get_depth () + 1 : 0;
    }

    // actually O(1) because of constexpr
    constexpr std::size_t get_indirection_level (void) const noexcept
    {
      return m_ptr_to != nullptr ? m_ptr_to.get_indirection_level () + 1 : 0;
    }

    template <typename T>
    static constexpr impl
    create_type (const char *name, ir_type parent) noexcept
    {
      return { name, parent, nullptr, sizeof (T),
               std::is_integral<T>::value, {} };
    }

    template <typename T>
    static constexpr impl
    create_compound_type (const char *name, ir_type_array members,
                          ir_type parent) noexcept
    {
      return { name, parent, nullptr, sizeof (T),
               std::is_integral<T>::value, members };
    }

    template <typename T>
    constexpr impl
    create_pointer (ir_type ptr_parent) const noexcept
    {
      return { m_base_name, ptr_parent, this, sizeof (T),
               std::is_integral<T>::value, {} };
    }

  private:

    constexpr impl (const char *name, ir_type parent, ir_type ptr_to,
                    size_t rep_size, bool is_integral,
                    ir_type_array members) noexcept
      : m_base_name (name),
        m_parent (parent),
        m_ptr_to (ptr_to),
        m_rep_size (rep_size),
        m_is_integral (is_integral),
        m_members (members)
    { }

    // A user readable type name (this is the base name if it is a pointer).
    // This should not be used directly. Use ir_type_printer to print a name.
    const char* m_base_name;
    ir_type m_parent;
    ir_type m_ptr_to;
    size_t m_rep_size;
    bool m_is_integral;
    ir_type_array m_members;
  };

  constexpr std::size_t ir_type::get_size (void) const noexcept
  {
    return m_ptr->get_size ();
  }

  constexpr bool ir_type::is_integral (void) const noexcept
  {
    return m_ptr->is_integral ();
  }

  // An abstract base type which may be null
  constexpr ir_type ir_type::get_parent_type (void) const noexcept
  {
    return m_ptr->get_parent_p ();
  }

  constexpr ir_type ir_type::get_dereference_type () const noexcept
  {
    return m_ptr->get_dereference_p ();
  }

  constexpr std::size_t ir_type::get_depth (void) const noexcept
  {
    return m_ptr->get_depth ();
  }

  constexpr std::size_t ir_type::get_indirection_level (void) const noexcept
  {
    return m_ptr->get_indirection_level ();
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
    static constexpr
    impl m_impl = impl::create_type<any> ("any", nullptr);
  };

  // shortcut for ir_type_impl::create_type
  template <typename T>
  constexpr ir_type::impl
  ir_type::create_type (const char *name) noexcept
  {
    return impl::create_type<T> (name, get<any> ());
  }

  template <typename T>
  constexpr ir_type::impl
  ir_type::create_type (const char *name, ir_type parent) noexcept
  {
    return impl::create_type<T> (name, parent);
  }

  // shortcut for ir_type_impl::create_compound_type
  template <typename T, std::size_t N>
  constexpr ir_type::impl
  ir_type::create_compound_type (const char *name,
                                 const ir_type (&members)[N]) noexcept
  {
    return impl::create_compound_type<T> (name, ir_type_array (members),
                                          get<any> ());
  }

  // shortcut for ir_type_impl::create_compound_type
  template <typename T, std::size_t N>
  constexpr ir_type::impl
  ir_type::create_compound_type (const char *name,
                                 const ir_type (&members)[N],
                                 ir_type parent) noexcept
  {
    return impl::create_compound_type<T> (name, ir_type_array (members),
                                          parent);
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
    static constexpr
    impl m_impl = create_type<type> ("void", nullptr);
  };

  /////////////////////////////////
  // pointer type instantiations //
  /////////////////////////////////

  template <typename T>
  struct ir_type::instance<T *>
  {
    using type = T *;
    static constexpr
    impl m_impl = get<T> ().m_ptr
      ->template create_pointer<T *> (get<any> ());
  };

  constexpr ir_type::impl_p ir_type::do_lca (impl_p ty1, impl_p ty2)
  {
    return ((ty1 == nullptr) || (ty2 == nullptr))
           ? &instance<void>::m_impl
           : (ty1 == ty2)
             ? ty1
             : (ty1->get_depth () > ty2->get_depth ())
               ? do_lca (ty1, ty2->get_parent_p ())
               : (ty1->get_depth () > ty2->get_depth ())
                 ? do_lca (ty1->get_parent_p (), ty2)
                 : do_lca (ty1->get_parent_p (), ty2->get_parent_p ());
  }

}

#endif

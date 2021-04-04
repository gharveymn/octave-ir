/** ir-type.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_TYPE_HPP
#define OCTAVE_IR_IR_TYPE_HPP


#if defined (__cpp_char8_t) && __cpp_char8_t >= 201811L
#  ifndef GCH_CHAR8_T
#    define GCH_CHAR8_T
#  endif
#endif

#include "gch/octave-ir-utilities/ir-common.hpp"
#include "gch/octave-ir-utilities/ir-functional.hpp"

#include <gch/nonnull_ptr.hpp>

#include <array>
#include <complex>
#include <iosfwd>

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

    constexpr
    ir_type_array (void) noexcept
      : m_array { nullptr },
        m_numel { 0 }
    { }

    template <std::size_t N>
    constexpr explicit
    ir_type_array (const ir_type (&members)[N]) noexcept
      : m_array (members),
        m_numel (N)
    { }

    [[nodiscard]]
    constexpr
    std::size_t
    get_size (void) const noexcept
    {
      return sum_size (m_numel);
    }

    [[nodiscard]]
    constexpr
    iterator
    begin (void) const noexcept
    {
      return m_array;
    }

    [[nodiscard]]
    constexpr
    iterator
    end (void) const noexcept;

    constexpr
    reference
    operator[] (std::size_t n) const noexcept;

    [[nodiscard]]
    constexpr
    std::size_t
    get_numel (void) const noexcept { return m_numel; }

  private:

    [[nodiscard]]
    constexpr
    std::size_t
    sum_size (std::size_t n) const noexcept;

    const ir_type *m_array;
    std::size_t    m_numel;
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
      ir_type_array m_members;
      bool          m_is_integral;
    };

    template <typename T>
    struct instance;

    constexpr
    ir_type (const impl& impl_ref) noexcept
      : m_ptr (impl_ref)
    { }

  public:
    friend struct std::hash<ir_type>;

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

    template <typename T,
              std::enable_if_t<std::is_same_v<
                const impl,
                decltype (instance<std::remove_cv_t<T>>::data)>
              > * = nullptr>
    static constexpr
    ir_type
    get (void) noexcept
    {
      return { instance<std::remove_cv_t<T>>::data };
    }

    friend constexpr
    bool
    operator== (const ir_type& lhs, const ir_type& rhs) noexcept
    {
      return lhs.m_ptr == rhs.m_ptr;
    }

  private:
    template <typename T>
    static constexpr
    impl
    create_type (const char *name, std::nullptr_t) noexcept
    {
      return { name, nullptr, nullptr, sizeof (T), { }, std::is_integral_v<T> };
    }

    template <typename T>
    static constexpr
    impl
    create_type (const char *name, ir_type base) noexcept
    {
      return { name, base.m_ptr, nullptr, sizeof (T), { }, std::is_integral_v<T> };
    }

    // shortcut for ir_type_impl::create_compound_type
    template <typename T, std::size_t N>
    static constexpr
    impl
    create_compound_type (const char *name, const ir_type (&members)[N], ir_type base) noexcept
    {
      return { name, base.m_ptr, nullptr, sizeof (T), ir_type_array (members),
               std::is_integral_v<T> };
    }

    template <typename T>
    static constexpr
    impl
    create_type (const char *name) noexcept;

    template <typename T, std::size_t N>
    static constexpr
    impl
    create_compound_type (const char *name, const ir_type (&members)[N]) noexcept;

    template <typename T>
    static constexpr
    impl
    create_pointer (ir_type pointer_base) noexcept;

    nonnull_ptr<const impl> m_ptr;
  };

  namespace detail
  {

    template <typename T, typename Enable = void>
    struct ir_type_value_impl
    { };

    template <typename T>
    struct ir_type_value_impl<T, std::enable_if_t<std::is_convertible_v<
                                   decltype (ir_type::get<T> ()),
                                   ir_type>>>
    {
      static constexpr
      ir_type
      value = ir_type::get<T> ();
    };

  }

  template <typename T>
  struct ir_type_value
    : detail::ir_type_value_impl<T>
  { };

  template <typename T>
  inline constexpr
  ir_type
  ir_type_v = ir_type_value<T>::value;

  namespace detail
  {

    template <typename T, typename Enable = void>
    struct is_ir_type_impl
      : std::false_type
    { };

    template <typename T>
    struct is_ir_type_impl<T, std::enable_if_t<std::is_convertible_v<
                                decltype (ir_type_value<T>::value),
                                ir_type>>>
      : std::true_type
    { };

  }

  template <typename T>
  struct is_ir_type
    : detail::is_ir_type_impl<T>
  { };

  template <typename T>
  inline constexpr
  bool
  is_ir_type_v = is_ir_type<T>::value;

  [[nodiscard]]
  constexpr
  std::size_t
  depth (ir_type ty) noexcept
  {
    return ty.has_base () ? (1 + depth (ty.get_base ())) : 0;
  }

  [[nodiscard]]
  constexpr
  std::size_t
  indirection_level (ir_type ty) noexcept
  {
    return ty.has_pointer_base () ? (1 + indirection_level (ty.get_pointer_base ())) : 0;
  }

  constexpr
  bool
  operator!= (ir_type lhs, ir_type rhs)  noexcept
  {
    return ! (lhs == rhs);
  }

  constexpr
  ir_type_array::iterator
  ir_type_array::
  end () const noexcept
  {
    return m_array + m_numel;
  }

  constexpr
  const ir_type&
  ir_type_array::
  operator[] (std::size_t n) const noexcept
  {
    return m_array[n];
  }

  constexpr
  size_t
  ir_type_array::
  sum_size (std::size_t n) const noexcept
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
    static constexpr
    impl
    data { create_type<any> ("any", nullptr) };
  };

  template <typename T>
  constexpr
  ir_type::impl
  ir_type::create_type (const char *name) noexcept
  {
    return create_type<T> (name, ir_type_v<any>);
  }

  template <typename T, std::size_t N>
  constexpr
  ir_type::impl
  ir_type::create_compound_type (const char *name, const ir_type (&members)[N]) noexcept
  {
    return create_compound_type<T, N> (name, members, ir_type_v<any>);
  }

  template <typename T>
  constexpr
  ir_type::impl
  ir_type::create_pointer (ir_type pointer_base) noexcept
  {
    return { pointer_base.get_name_base (), ir_type_v<any>.m_ptr, pointer_base.m_ptr, sizeof (T),
             { }, false };
  }

  // template instantiations
  // to define an ir_type_p implement a specialization of ir_type_instance
  // copying the structure seen below. For examples, see
  // ir_type_instance<any> for base types, ir_type_instance<Complex> for
  // struct types, and ir_type_instance<any *> for pointer types.

  //////////
  // void //
  //////////
  // This is the unit type; the only type which is not a descendant of 'any'.

  struct unit { };

  template <>
  struct ir_type::instance<void>
  {
    using type = unit;
    static constexpr
    impl
    data { create_type<type> ("void", nullptr) };
  };

  /////////////////////////////////
  // pointer type instantiations //
  /////////////////////////////////

  template <typename T>
  struct ir_type::instance<T *>
  {
    using type = T *;
    static constexpr
    impl
    data { create_pointer<T *> (ir_type_v<T>) };
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

  //////////////////////////
  // floating point types //
  //////////////////////////

  // maybe ifdef this
  template <>
  struct ir_type::instance<long double>
  {
    using type = long double;
    static constexpr
    impl
    data { create_type<type> ("ldouble") };
  };

  template <>
  struct ir_type::instance<double>
  {
    using type = double;
    static constexpr
    impl
    data { create_type<type> ("double") };
  };

  template <>
  struct ir_type::instance<single>
  {
    using type = single;
    static constexpr
    impl
    data { create_type<type> ("single", ir_type_v<double>) };
  };

  ///////////////////////
  // fundamental types //
  ///////////////////////

  template <>
  struct ir_type::instance<std::int64_t>
  {
    using type = std::int64_t;
    static constexpr
    impl
    data { create_type<type> ("i64") };
  };

  template <>
  struct ir_type::instance<std::int32_t>
  {
    using type = std::int32_t;
    static constexpr
    impl
    data { create_type<type> ("i32", ir_type_v<std::int64_t>) };
  };

  template <>
  struct ir_type::instance<std::int16_t>
  {
    using type = std::int16_t;
    static constexpr
    impl
    data { create_type<type> ("i16", ir_type_v<std::int32_t>) };
  };

  template <>
  struct ir_type::instance<int8_t>
  {
    using type = std::int8_t;
    static constexpr
    impl
    data { create_type<type> ("i8", ir_type_v<std::int16_t>) };
  };

  template <>
  struct ir_type::instance<std::uint64_t>
  {
    using type = std::uint64_t;
    static constexpr
    impl
    data { create_type<type> ("ui64") };
  };

  template <>
  struct ir_type::instance<std::uint32_t>
  {
    using type = std::uint32_t;
    static constexpr
    impl
    data { create_type<type> ("ui32", ir_type_v<std::uint64_t>) };
  };

  template <>
  struct ir_type::instance<std::uint16_t>
  {
    using type = std::uint16_t;
    static constexpr
    impl
    data { create_type<type> ("ui16", ir_type_v<std::uint32_t>) };
  };

  template <>
  struct ir_type::instance<std::uint8_t>
  {
    using type = std::uint8_t;
    static constexpr
    impl
    data { create_type<type> ("ui8", ir_type_v<std::uint16_t>) };
  };

  template <>
  struct ir_type::instance<char>
  {
    using type = char;
    static constexpr
    impl
    data { create_type<type> ("char") };
  };

  template <>
  struct ir_type::instance<wchar_t>
  {
    using type = wchar_t;
    static constexpr
    impl
    data { create_type<type> ("wchar") };
  };

  template <>
  struct ir_type::instance<char32_t>
  {
    using type = char32_t;
    static constexpr
    impl
    data { create_type<type> ("char32") };
  };

  template <>
  struct ir_type::instance<char16_t>
  {
    using type = char16_t;
    static constexpr
    impl
    data { create_type<type> ("char16") };
  };

#ifdef GCH_CHAR8_T

  template <>
  struct ir_type::instance<char8_t>
  {
    using type = char8_t;
    static constexpr
    impl
    data { create_type<char8_t> ("char8") };
  };

#endif

  template <>
  struct ir_type::instance<bool>
  {
    using type = bool;
    static constexpr
    impl
    data { create_type<type> ("bool") };
  };

  template <>
  struct ir_type::instance<ir_type>
  {
    using type = ir_type;
    static constexpr
    impl
    data { create_type<type> ("ir_type") };
  };

  ///////////////////
  // complex types //
  ///////////////////

  template <>
  struct ir_type::instance<std::complex<double>>
  {
    using type = std::complex<double>;

    static constexpr
    ir_type
    m_members[] { ir_type_v<double>, ir_type_v<double> };

    static_assert (ir_type_array (m_members).get_size () == sizeof (type),
                   "The size of Complex is not equal to its IR counterpart.");

    static constexpr
    impl
    data { create_compound_type<type> ("complex", m_members) };
  };

  template <>
  struct ir_type::instance<std::complex<single>>
  {
    using type = std::complex<single>;

    static constexpr
    ir_type
    m_members[] { ir_type_v<single>, ir_type_v<single> };

    static_assert (ir_type_array (m_members).get_size () == sizeof (type),
              "The size of FloatComplex is not equal to its IR counterpart.");

    static constexpr
    impl
    data { create_compound_type<type> ("fcomplex", m_members) };
  };

  using ir_type_pack = type_pack<
    any,
    void,

    long double,
    double,
    single,
    std::int64_t,
    std::int32_t,
    std::int16_t,
    std::int8_t,
    std::uint64_t,
    std::uint32_t,
    std::uint16_t,
    std::uint8_t,
    char,
    wchar_t,
    char32_t,
    char16_t,
#ifdef GCH_CHAR8_T
    char8_t,
#endif
    bool,
    ir_type,
    std::complex<double>,
    std::complex<single>,

    long double *,
    double *,
    single *,
    std::int64_t *,
    std::int32_t *,
    std::int16_t *,
    std::int8_t *,
    std::uint64_t *,
    std::uint32_t *,
    std::uint16_t *,
    std::uint8_t *,
    char *,
    wchar_t *,
    char32_t *,
    char16_t *,
#ifdef GCH_CHAR8_T
    char8_t *,
#endif
    bool *,
    ir_type *,
    std::complex<double> *,
    std::complex<single> *
  >;

  template <typename T>
  struct ir_type_mapper
  {
    constexpr
    ir_type
    operator() (void) const noexcept
    {
      return ir_type_v<T>;
    }
  };

  inline constexpr
  auto
  ir_type_list = pack_transform_v<ir_type_pack, ir_type_mapper>;

  template <typename T>
  inline constexpr
  std::size_t
  ir_type_index = std::distance (std::begin (ir_type_list),
                                 std::find (std::begin (ir_type_list), std::end (ir_type_list),
                                            ir_type_v<T>));
  inline constexpr
  auto
  num_ir_types = pack_size_v<ir_type_pack>;

}

namespace std
{

  template <>
  struct hash<gch::ir_type>
  {
    std::size_t
    operator() (const gch::ir_type& ty) const noexcept
    {
      return std::hash<gch::nonnull_ptr<const gch::ir_type::impl>> { } (ty.m_ptr);
    }
  };

}

#endif

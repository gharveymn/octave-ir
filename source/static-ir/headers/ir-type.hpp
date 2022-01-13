/** ir-type.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_STATIC_IR_IR_TYPE_HPP
#define OCTAVE_IR_STATIC_IR_IR_TYPE_HPP


#if defined (__cpp_char8_t) && __cpp_char8_t >= 201811L
#  ifndef GCH_CHAR8_T
#    define GCH_CHAR8_T
#  endif
#endif

#include "ir-common.hpp"
#include "ir-functional.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>

#include <array>
#include <complex>
#include <optional>

class octave_base_value;

namespace gch
{

  using any = octave_base_value *;

  class ir_type;

  using ir_type_pack = type_pack<
    void,
    any,

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
    std::complex<double> *,
    std::complex<single> *
  >;

  static_assert (std::is_same_v<ir_type_pack, pack_unique_t<ir_type_pack>>,
                 "Types are not unique.");

  inline constexpr
  unsigned
  num_ir_types = pack_size_v<ir_type_pack>;

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

  class ir_type_base
  {
    struct impl;

    template <typename T>
    struct instance;

    constexpr explicit
    ir_type_base (const impl& impl_ref) noexcept
      : m_ptr (impl_ref)
    { }

  public:
    friend struct std::hash<ir_type>;
    friend class ir_type;

    ir_type_base            (void)                    = delete;
    ir_type_base            (const ir_type_base&)     = default;
    ir_type_base            (ir_type_base&&) noexcept = default;
    ir_type_base& operator= (const ir_type_base&)     = default;
    ir_type_base& operator= (ir_type_base&&) noexcept = default;
    ~ir_type_base           (void)                    = default;

    template <typename T,
              std::enable_if_t<std::is_same_v<
                const impl,
                decltype (instance<std::remove_cv_t<T>>::data)>
              > * = nullptr>
    static constexpr
    ir_type_base
    get (void) noexcept
    {
      return ir_type_base { instance<std::remove_cv_t<T>>::data };
    }

  private:
    template <typename T>
    static constexpr
    impl
    create_type (const char *name, std::nullptr_t) noexcept;

    template <typename T>
    static constexpr
    impl
    create_type (const char *name, ir_type base) noexcept;

    // shortcut for ir_type_impl::create_compound_type
    template <typename T, std::size_t N>
    static constexpr
    impl
    create_compound_type (const char *name, const ir_type (&members)[N], ir_type base) noexcept;

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

  class ir_type
  {
  public:
    ir_type            (void)               = delete;
    ir_type            (const ir_type&)     = default;
    ir_type            (ir_type&&) noexcept = default;
    ir_type& operator= (const ir_type&)     = default;
    ir_type& operator= (ir_type&&) noexcept = default;
    ~ir_type           (void)               = default;

    constexpr
    ir_type (ir_type_base ty, unsigned idx) noexcept
      : m_base (ty),
        m_idx  (idx)
    { }

    [[nodiscard]] constexpr
    const char *
    get_name_base (void) const noexcept;

    [[nodiscard]] constexpr
    ir_type
    get_base (void) const noexcept;

    [[nodiscard]] constexpr
    ir_type
    get_pointer_base (void) const noexcept;

    [[nodiscard]] constexpr
    std::size_t
    get_size  (void) const noexcept;

    [[nodiscard]] constexpr
    bool
    is_integral (void) const noexcept;

    [[nodiscard]] constexpr
    ir_type_array
    get_members (void) const noexcept;

    [[nodiscard]] constexpr
    bool
    has_base (void) const noexcept;

    [[nodiscard]] constexpr
    bool
    has_pointer_base (void) const noexcept;

    friend constexpr
    bool
    operator== (const ir_type& lhs, const ir_type& rhs) noexcept
    {
      return lhs.m_idx == rhs.m_idx;
    }

    [[nodiscard]] constexpr
    unsigned
    get_index (void) const noexcept
    {
      return m_idx;
    }

  private:
    [[nodiscard]] constexpr
    const ir_type_base::impl&
    get_impl (void) const noexcept
    {
      return *m_base.m_ptr;
    }

    ir_type_base m_base;
    unsigned     m_idx;
  };

  struct ir_type_base::impl
  {
    // A user readable type name (this is the base name if it is a pointer).
    // This should not be used directly. Use ir_type_printer to print a name.
    const char * const           m_name_base;
    const std::optional<ir_type> m_base_type;
    const std::optional<ir_type> m_pointer_base_type;
    const std::size_t            m_rep_size;
    const ir_type_array          m_members;
    const bool                   m_is_integral;
  };

  [[nodiscard]] constexpr
  const char *
  ir_type::
  get_name_base (void) const noexcept
  {
    return get_impl ().m_name_base;
  }

  [[nodiscard]] constexpr
  ir_type
  ir_type::
  get_base (void) const noexcept
  {
    return *get_impl ().m_base_type;
  }

  [[nodiscard]] constexpr
  ir_type
  ir_type::
  get_pointer_base (void) const noexcept
  {
    return *get_impl ().m_pointer_base_type;
  }

  [[nodiscard]] constexpr
  std::size_t
  ir_type::
  get_size  (void) const noexcept
  {
    return get_impl ().m_rep_size;
  }

  [[nodiscard]] constexpr
  bool
  ir_type::
  is_integral (void) const noexcept
  {
    return get_impl ().m_is_integral;
  }

  [[nodiscard]] constexpr
  ir_type_array
  ir_type::
  get_members (void) const noexcept
  {
    return get_impl ().m_members;
  }

  [[nodiscard]] constexpr
  bool
  ir_type::
  has_base (void) const noexcept
  {
    return get_impl ().m_base_type.has_value ();
  }

  [[nodiscard]] constexpr
  bool
  ir_type::
  has_pointer_base (void) const noexcept
  {
    return get_impl ().m_pointer_base_type.has_value ();
  }

  template <typename T>
  constexpr
  auto
  ir_type_base::
  create_type (const char *name, std::nullptr_t) noexcept
    -> impl
  {
    return { name, std::nullopt, std::nullopt, sizeof (T), { }, std::is_integral_v<T> };
  }

  template <typename T>
  constexpr
  auto
  ir_type_base::
  create_type (const char *name, ir_type base) noexcept
    -> impl
  {
    return { name, base, std::nullopt, sizeof (T), { }, std::is_integral_v<T> };
  }

  // shortcut for ir_type_impl::create_compound_type
  template <typename T, std::size_t N>
  constexpr
  auto
  ir_type_base::
  create_compound_type (const char *name, const ir_type (&members)[N], ir_type base) noexcept
    -> impl
  {
    return { name, base, std::nullopt, sizeof (T), ir_type_array (members),
             std::is_integral_v<T> };
  }

  constexpr
  bool
  operator!= (ir_type lhs, ir_type rhs) noexcept
  {
    return ! (lhs == rhs);
  }

  namespace detail
  {

    template <typename T, typename Enable = void>
    struct ir_type_value_impl
    { };

    template <typename T>
    struct ir_type_value_impl<T, std::enable_if_t<std::is_convertible_v<
                                   decltype (ir_type_base::get<T> ()),
                                   ir_type_base>>>
    {
      static constexpr
      ir_type
      value = { ir_type_base::get<T> (), pack_index_v<ir_type_pack, T> };
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
  struct ir_type_base::instance<any>
  {
    using type = any;
    static constexpr
    impl
    data = create_type<any> ("any", nullptr);
  };

  template <typename T>
  constexpr
  auto
  ir_type_base::
  create_type (const char *name) noexcept
    -> impl
  {
    return create_type<T> (name, ir_type_v<any>);
  }

  template <typename T, std::size_t N>
  constexpr
  auto
  ir_type_base::
  create_compound_type (const char *name, const ir_type (&members)[N]) noexcept
    -> impl
  {
    return create_compound_type<T, N> (name, members, ir_type_v<any>);
  }

  template <typename T>
  constexpr
  auto
  ir_type_base::
  create_pointer (ir_type pointer_base) noexcept
    -> impl
  {
    return { pointer_base.get_name_base (), ir_type_v<any>, pointer_base, sizeof (T), { }, false };
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
  struct ir_type_base::instance<void>
  {
    using type = unit;
    static constexpr
    impl
    data = create_type<type> ("void", nullptr);
  };

  /////////////////////////////////
  // pointer type instantiations //
  /////////////////////////////////

  template <typename T>
  struct ir_type_base::instance<T *>
  {
    using type = T *;
    static constexpr
    impl
    data = create_pointer<T *> (ir_type_v<T>);
  };

  //////////////////////////
  // floating point types //
  //////////////////////////

  // maybe ifdef this
  template <>
  struct ir_type_base::instance<long double>
  {
    using type = long double;
    static constexpr
    impl
    data = create_type<type> ("ldouble");
  };

  template <>
  struct ir_type_base::instance<double>
  {
    using type = double;
    static constexpr
    impl
    data = create_type<type> ("double");
  };

  template <>
  struct ir_type_base::instance<single>
  {
    using type = single;
    static constexpr
    impl
    data = create_type<type> ("single", ir_type_v<double>);
  };

  ///////////////////////
  // fundamental types //
  ///////////////////////

  template <>
  struct ir_type_base::instance<std::int64_t>
  {
    using type = std::int64_t;
    static constexpr
    impl
    data = create_type<type> ("i64");
  };

  template <>
  struct ir_type_base::instance<std::int32_t>
  {
    using type = std::int32_t;
    static constexpr
    impl
    data = create_type<type> ("i32", ir_type_v<std::int64_t>);
  };

  template <>
  struct ir_type_base::instance<std::int16_t>
  {
    using type = std::int16_t;
    static constexpr
    impl
    data = create_type<type> ("i16", ir_type_v<std::int32_t>);
  };

  template <>
  struct ir_type_base::instance<int8_t>
  {
    using type = std::int8_t;
    static constexpr
    impl
    data = create_type<type> ("i8", ir_type_v<std::int16_t>);
  };

  template <>
  struct ir_type_base::instance<std::uint64_t>
  {
    using type = std::uint64_t;
    static constexpr
    impl
    data = create_type<type> ("ui64");
  };

  template <>
  struct ir_type_base::instance<std::uint32_t>
  {
    using type = std::uint32_t;
    static constexpr
    impl
    data = create_type<type> ("ui32", ir_type_v<std::uint64_t>);
  };

  template <>
  struct ir_type_base::instance<std::uint16_t>
  {
    using type = std::uint16_t;
    static constexpr
    impl
    data = create_type<type> ("ui16", ir_type_v<std::uint32_t>);
  };

  template <>
  struct ir_type_base::instance<std::uint8_t>
  {
    using type = std::uint8_t;
    static constexpr
    impl
    data = create_type<type> ("ui8", ir_type_v<std::uint16_t>);
  };

  template <>
  struct ir_type_base::instance<char>
  {
    using type = char;
    static constexpr
    impl
    data = create_type<type> ("char");
  };

  template <>
  struct ir_type_base::instance<wchar_t>
  {
    using type = wchar_t;
    static constexpr
    impl
    data = create_type<type> ("wchar");
  };

  template <>
  struct ir_type_base::instance<char32_t>
  {
    using type = char32_t;
    static constexpr
    impl
    data = create_type<type> ("char32");
  };

  template <>
  struct ir_type_base::instance<char16_t>
  {
    using type = char16_t;
    static constexpr
    impl
    data = create_type<type> ("char16");
  };

#ifdef GCH_CHAR8_T

  template <>
  struct ir_type_base::instance<char8_t>
  {
    using type = char8_t;
    static constexpr
    impl
    data = create_type<char8_t> ("char8");
  };

#endif

  template <>
  struct ir_type_base::instance<bool>
  {
    using type = bool;
    static constexpr
    impl
    data = create_type<type> ("bool");
  };

  ///////////////////
  // complex types //
  ///////////////////

  template <>
  struct ir_type_base::instance<std::complex<double>>
  {
    using type = std::complex<double>;

    static constexpr
    ir_type
    m_members[] { ir_type_v<double>, ir_type_v<double> };

    static_assert (ir_type_array (m_members).get_size () == sizeof (type),
                   "The size of Complex is not equal to its IR counterpart.");

    static constexpr
    impl
    data = create_compound_type<type> ("complex", m_members);
  };

  template <>
  struct ir_type_base::instance<std::complex<single>>
  {
    using type = std::complex<single>;

    static constexpr
    ir_type
    m_members[] { ir_type_v<single>, ir_type_v<single> };

    static_assert (ir_type_array (m_members).get_size () == sizeof (type),
              "The size of FloatComplex is not equal to its IR counterpart.");

    static constexpr
    impl
    data = create_compound_type<type> ("fcomplex", m_members);
  };

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

}

namespace std
{

  template <>
  struct hash<gch::ir_type>
  {
    std::size_t
    operator() (const gch::ir_type& ty) const noexcept
    {
      return ty.get_index ();
    }
  };

}

#endif // OCTAVE_IR_STATIC_IR_IR_TYPE_HPP

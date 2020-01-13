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

#if ! defined (ir_type_base_h)
#define ir_type_base_h 1

#include "ir-type.hpp"

#include <complex>

namespace octave
{

  using single = float;

  //////////////////////////
  // floating point types //
  //////////////////////////

  // maybe ifdef this
  template <>
  struct ir_type::instance<long double>
  {
    using type = long double;
    static constexpr
    impl m_impl = create_type<type> ("ldouble");
  };

  template <>
  struct ir_type::instance<double>
  {
    using type = double;
    static constexpr
    impl m_impl = create_type<type> ("double");
  };

  template <>
  struct ir_type::instance<single>
  {
    using type = single;
    static constexpr
    impl m_impl = create_type<type> ("single", get<double> ());
  };

  ///////////////////////
  // fundamental types //
  ///////////////////////

  template <>
  struct ir_type::instance<int64_t>
  {
    using type = int64_t;
    static constexpr
    impl m_impl = create_type<type> ("i64");
  };

  template <>
  struct ir_type::instance<int32_t>
  {
    using type = int32_t;
    static constexpr
    impl m_impl = create_type<type> ("i32", get<int64_t> ());
  };

  template <>
  struct ir_type::instance<int16_t>
  {
    using type = int16_t;
    static constexpr
    impl m_impl = create_type<type> ("i16", get<int32_t> ());
  };

  template <>
  struct ir_type::instance<int8_t>
  {
    using type = int8_t;
    static constexpr
    impl m_impl = create_type<type> ("i8", get<int16_t> ());
  };

  template <>
  struct ir_type::instance<uint64_t>
  {
    using type = uint64_t;
    static constexpr
    impl m_impl = create_type<type> ("ui64");
  };

  template <>
  struct ir_type::instance<uint32_t>
  {
    using type = uint32_t;
    static constexpr
    impl m_impl = create_type<type> ("ui32", get<uint64_t> ());
  };

  template <>
  struct ir_type::instance<uint16_t>
  {
    using type = uint16_t;
    static constexpr
    impl m_impl = create_type<type> ("ui16", get<uint32_t> ());
  };

  template <>
  struct ir_type::instance<uint8_t>
  {
    using type = uint8_t;
    static constexpr
    impl m_impl = create_type<type> ("ui8", get<uint16_t> ());
  };

  template <>
  struct ir_type::instance<char>
  {
    using type = char;
    static constexpr
    impl m_impl = create_type<type> ("char");
  };
	
  template <>
  struct ir_type::instance<const char>
  {
    using type = char;
    static constexpr
    impl m_impl = create_type<type> ("const char");
  };

  template <>
  struct ir_type::instance<wchar_t>
  {
    using type = wchar_t;
    static constexpr
    impl m_impl = create_type<type> ("wchar");
  };

  template <>
  struct ir_type::instance<char32_t>
  {
    using type = char32_t;
    static constexpr
    impl m_impl = create_type<type> ("char32");
  };

  template <>
  struct ir_type::instance<char16_t>
  {
    using type = char16_t;
    static constexpr
    impl m_impl = create_type<type> ("char16");
  };

#if __cpp_char8_t >= 201811L

  template <>
  struct ir_type::instance<char8_t>
  {
    using type = char8_t;
    static constexpr
    impl m_impl = create_type<char8_t> ("char8");
  };

#endif

  template <>
  struct ir_type::instance<bool>
  {
    using type = bool;
    static constexpr
    impl m_impl = create_type<type> ("bool");
  };
  
  template <>
  struct ir_type::instance<ir_type>
  {
    using type = ir_type;
    static constexpr
    impl m_impl = create_type<type> ("ir_type");
  };

  ///////////////////
  // complex types //
  ///////////////////

  template <>
  struct ir_type::instance<std::complex<double>>
  {
    using type = std::complex<double>;
    static constexpr ir_type m_members[]
      {
        get<double> (),
        get<double> (),
      };
    static_assert (ir_type_array (m_members).get_size () == sizeof (type),
                   "The size of Complex is not equal to its IR counterpart.");
    static constexpr
    impl m_impl = create_compound_type<type> ("complex", m_members);
  };

  template <>
  struct ir_type::instance<std::complex<single>>
  {
    using type = std::complex<single>;
    static constexpr ir_type m_members[]
      {
        get<single> (),
        get<single> (),
      };
    static_assert (ir_type_array (m_members).get_size () == sizeof (type),
              "The size of FloatComplex is not equal to its IR counterpart.");
    static constexpr
    impl m_impl = create_compound_type<type> ("fcomplex", m_members);
  };

}

#endif

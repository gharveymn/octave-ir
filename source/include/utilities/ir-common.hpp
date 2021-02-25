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

//! this header forward-declares relevant classes and declares some typedefs
#if ! defined (octave_ir_common_h)
#define octave_ir_common_h 1

#define GCH_PRINT_SIZE(TYPE) \
char (*__gch__fail) (void)[sizeof(TYPE)] = 1;

#define GCH_STRCAT_(x, y) x ## y
#define GCH_STRCAT(x, y) GCH_STRCAT_(x, y)
#define GCH_PRINT_VALUE(x) \
template <int> struct GCH_STRCAT(GCH_STRCAT(value_of_, x), _is); \
static_assert (GCH_STRCAT (GCH_STRCAT (value_of_, x), _is)<x>::x, " ");

#ifndef GCH_ACCUMULATE_REF
#  if __cplusplus > 201703L
#    define GCH_ACCUMULATE_LHS(TYPE) TYPE&&
#  else
#    define GCH_ACCUMULATE_LHS(TYPE) TYPE&
#  endif
#endif

#ifndef GCH_CPP20_CONSTEVAL
#  if defined (__cpp_consteval) && __cpp_consteval >= 201811L
#    define GCH_CPP20_CONSTEVAL consteval
#  else
#    define GCH_CPP20_CONSTEVAL constexpr
#  endif
#endif

class octave_base_value;

namespace gch
{

#ifdef NDEBUG
  inline constexpr
  bool
  OCTAVE_IR_DEBUG = false;
#else
  inline constexpr
  bool
  OCTAVE_IR_DEBUG = true;
#endif

  using any = octave_base_value *;
  using single = float;

}

#endif

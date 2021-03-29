/** ir-common.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

//! this header forward-declares relevant classes and declares some typedefs
#ifndef OCTAVE_IR_IR_COMMON_HPP
#define OCTAVE_IR_IR_COMMON_HPP

#define GCH_PRINT_SIZE(TYPE) \
char (*__gch__fail) (void)[sizeof(TYPE)] = 1;

#define GCH_STRCAT_(x, y) x ## y
#define GCH_STRCAT(x, y) GCH_STRCAT_(x, y)
#define GCH_PRINT_VALUE(x) \
template <auto> struct GCH_STRCAT(GCH_STRCAT(value_of_, x), _is); \
static_assert (GCH_STRCAT (GCH_STRCAT (value_of_, x), _is)<x>::x, " ");

#ifndef GCH_ACCUMULATE_REF
#  if __cplusplus > 201703L
#    define GCH_ACCUMULATE_LHS(TYPE) TYPE&&
#  else
#    define GCH_ACCUMULATE_LHS(TYPE) TYPE&
#  endif
#endif

#ifndef GCH_CPP20_CONSTEXPR
#  if defined (__cpp_constexpr) && __cpp_constexpr >= 201907L
#    define GCH_CPP20_CONSTEXPR constexpr
#    ifndef GCH_HAS_CPP20_CONSTEXPR
#      define GCH_HAS_CPP20_CONSTEXPR
#    endif
#  else
#    define GCH_CPP20_CONSTEXPR
#  endif
#endif

#ifndef GCH_CPP20_CONSTEVAL
#  if defined (__cpp_consteval) && __cpp_consteval >= 201811L
#    define GCH_CPP20_CONSTEVAL consteval
#  else
#    define GCH_CPP20_CONSTEVAL constexpr
#  endif
#endif

#if defined (__cpp_constinit) && __cpp_constinit >= 201907L
#  ifndef GCH_CONSTINIT
#    define GCH_CONSTINIT
#  endif
#endif

#ifndef GCH_IMPLICIT_CONVERSION
#  if defined (__cpp_conditional_explicit) && __cpp_conditional_explicit >= 201806L
#    define GCH_IMPLICIT_CONVERSION explicit (false)
#  else
#    define GCH_IMPLICIT_CONVERSION /* implicit */
#  endif
#endif

#if defined (__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
#  ifndef GCH_IMPL_THREE_WAY_COMPARISON
#    define GCH_IMPL_THREE_WAY_COMPARISON
#  endif
#endif

#if defined (__cpp_concepts) && __cpp_concepts >= 201907L
#  ifndef GCH_CONCEPTS
#    define GCH_CONCEPTS
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

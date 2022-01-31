/** test_common.hpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_UTILITIES_IR_LINK_SET_TEST_COMMON_HPP
#define OCTAVE_IR_UTILITIES_IR_LINK_SET_TEST_COMMON_HPP

#include "ir-link-set.hpp"

#include <array>
#include <cstdio>

#define CHECK(EXPR)                                                           \
if (! (EXPR))                                                                 \
{                                                                             \
  printf ("Check failed in file " __FILE__ " at line %i:\n" #EXPR, __LINE__); \
  return 1;                                                                   \
} (void)0

struct test_struct
{ };

static inline std::array<test_struct, 10> blks { };

extern template class gch::ir_link_set<test_struct>;

#endif // OCTAVE_IR_UTILITIES_IR_LINK_SET_TEST_COMMON_HPP

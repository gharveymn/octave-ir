/** extern-funcs.cpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "jit-exception.hpp"

#include <csetjmp>
#include <cstdio>
#include <iostream>
#include <memory>

#ifdef _WIN32
#  define DLLEXPORT __declspec(dllexport)
#else
#  define DLLEXPORT
#endif

std::jmp_buf env_buffer;
gch::jit_exception current_exception;

extern "C" DLLEXPORT
void
print_error (const char *s);

extern "C" DLLEXPORT
void
throw_error (const char *s);

extern "C"
void
print_error (const char *s)
{
  std::fprintf (stderr, "%s", s);
}

extern "C"
void
throw_error (const char *s)
{
  current_exception = gch::jit_exception (s);
  std::longjmp (env_buffer, 1);
}

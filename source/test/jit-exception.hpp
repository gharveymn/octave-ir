/** jit-exception.hpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_JIT_EXCEPTION_HPP
#define OCTAVE_IR_JIT_EXCEPTION_HPP

#include <stdexcept>

namespace gch
{

  class jit_exception
    : public std::exception
  {
  public:
    jit_exception            (void)                     = default;
    jit_exception            (const jit_exception&)     = default;
    jit_exception            (jit_exception&&) noexcept = default;
    jit_exception& operator= (const jit_exception&)     = default;
    jit_exception& operator= (jit_exception&&) noexcept = default;
    ~jit_exception           (void)                     = default;

    jit_exception (std::string_view str)
      : m_error_string (str)
    { }

    [[nodiscard]]
    const char *
    what (void) const noexcept override
    {
      return m_error_string.c_str ();
    }

  private:
    std::string m_error_string;
  };

}

#endif // OCTAVE_IR_JIT_EXCEPTION_HPP

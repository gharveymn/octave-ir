/** jit-exception.hpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_JIT_EXCEPTION_HPP
#define OCTAVE_IR_JIT_EXCEPTION_HPP

#include <stdexcept>
#include <memory>

namespace gch
{

  class jit_exception
    : public std::exception
  {
  public:
    jit_exception (void) = default;

    explicit
    jit_exception (std::unique_ptr<std::exception>&& except) noexcept
      : m_except (std::move (except))
    { }

    [[nodiscard]]
    const char *
    what (void) const noexcept override
    {
      if (m_except)
        return m_except->what ();
      return "Failed to allocate for an exception.";
    }

  private:
    std::unique_ptr<std::exception> m_except;
  };

}

#endif // OCTAVE_IR_JIT_EXCEPTION_HPP

/** ir-error.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_UTILITIES_IR_ERROR_HPP
#define OCTAVE_IR_UTILITIES_IR_ERROR_HPP

#include <exception>
#include <stdexcept>
#include <cstring>
#include <iostream>

namespace gch
{

  class ir_exception
    : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

  enum class reason
  {
    impossible,
    logic_error,
  };

  template <reason R>
  struct abort_message;

  template <reason R>
  inline constexpr bool abort_message_v = abort_message<R>::value;

  template <>
  struct abort_message<reason::impossible>
  {
    static constexpr
    const char *
    value = "The program reached an impossible state and will now abort.";
  };

  template <>
  struct abort_message<reason::logic_error>
  {
    static constexpr
    const char *
    value = "The program encountered a logic error and must abort.";
  };

  template <reason R>
  [[noreturn]]
  void
  abort (const char *message = "")
  {
    if (std::strlen (message) != 0)
      std::cerr << message << "\n\n";
    std::cerr << abort_message_v<R> << std::endl;
    std::abort ();
  }

}

#endif // OCTAVE_IR_UTILITIES_IR_ERROR_HPP

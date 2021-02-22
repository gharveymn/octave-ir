/** ir-error.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_ERROR_HPP
#define OCTAVE_IR_IR_ERROR_HPP

#include <exception>
#include <stdexcept>

namespace gch
{

  class ir_exception
    : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

  namespace abort
  {

    inline constexpr
    const char *
    ir_impossible_message = "The program reached an impossible state and will now abort.";

    [[noreturn]]
    void
    ir_impossible (const char *message = ir_impossible_message);

    inline constexpr
    const char *
    ir_logic_error_message = "The program encountered a logic error and must abort.";

    [[noreturn]]
    void
    ir_logic_error (const char *message = "");

  }

}

#endif // OCTAVE_IR_IR_ERROR_HPP

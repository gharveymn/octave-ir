/** octave-ir-compiler.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_OCTAVE_IR_COMPILER_HPP
#define OCTAVE_IR_COMPILER_OCTAVE_IR_COMPILER_HPP

#include "gch/octave-ir-static-ir/ir-static-function.hpp"

#include <filesystem>

namespace gch
{

  std::filesystem::path
  compile (const ir_static_function& func);

}

#endif // OCTAVE_IR_COMPILER_OCTAVE_IR_COMPILER_HPP

/** octave-ir-compiler.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_OCTAVE_IR_COMPILER_HPP
#define OCTAVE_IR_OCTAVE_IR_COMPILER_HPP

#include "gch/octave-static-ir/ir-static-module.hpp"

#include <filesystem>

namespace gch
{

  std::filesystem::path
  compile (const ir_static_module& module);

}

#endif // OCTAVE_IR_OCTAVE_IR_COMPILER_HPP

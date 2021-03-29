/** octave-ir-llvm-compiler.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <llvm/ADT/SmallVector.h>

#include "gch/octave-ir-compiler.hpp"

#include <vector>

namespace gch
{

  std::filesystem::path
  compile (const ir_static_module&)
  {
    std::vector<int> x;
    llvm::report_bad_alloc_error ("", false);
    llvm::SmallVector<std::string, 2> v;
    v.emplace_back ("hi");
    return v[0];
  }

}

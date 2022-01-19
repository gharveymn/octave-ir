/** llvm-constant.cpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "llvm-constant.hpp"

namespace gch
{

  llvm::Value&
  get_constant (llvm_module_interface& module, const ir_constant& c)
  {
    constexpr auto map = generate_ir_type_map<llvm_constant_mapper> ();
    return map[c.get_type ()] (module, c);
  }

}

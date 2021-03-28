/** ir-instruction-metadata.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "values/types/ir-instruction-metadata.hpp"

namespace gch
{

  inline constexpr auto x = get_metadata (ir_opcode::band);
  inline constexpr auto y = ir_metadata_v<ir_opcode::band>;

}

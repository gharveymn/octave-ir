/** ir-substructure-visitors-fwd.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_SUBSTRUCTURE_VISITORS_FWD_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_SUBSTRUCTURE_VISITORS_FWD_HPP

#include "substructure/inspectors/ir-substructure-inspectors-fwd.hpp"
#include "substructure/mutators/ir-substructure-mutators-fwd.hpp"

#include "structure/ir-structure-visitors-fwd.hpp"
#include "subcomponent/ir-subcomponent-visitors-fwd.hpp"

namespace gch
{

  template <>
  struct consolidated_visitors<ir_substructure>
    : pack_union<exclusive_visitors_t<ir_substructure>,
                 consolidated_visitors_t<ir_structure>,
                 consolidated_visitors_t<ir_subcomponent>>
  { };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_SUBSTRUCTURE_VISITORS_FWD_HPP

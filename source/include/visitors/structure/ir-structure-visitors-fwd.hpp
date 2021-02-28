/** ir-structure-visitors-fwd.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_STRUCTURE_VISITORS_FWD_HPP
#define OCTAVE_IR_IR_STRUCTURE_VISITORS_FWD_HPP

#include "inspectors/ir-structure-inspectors-fwd.hpp"
#include "mutators/ir-structure-mutators-fwd.hpp"

#include "visitors/component/inspectors/ir-component-inspectors-fwd.hpp"

namespace gch
{

  template <>
  struct exclusive_visitors<ir_structure>
    : pack_union<exclusive_inspectors_t<ir_structure>,
                 exclusive_mutators_t<ir_structure>,
                 visitor_types<ir_leaf_collector>>
  { };

}

#endif // OCTAVE_IR_IR_STRUCTURE_VISITORS_FWD_HPP

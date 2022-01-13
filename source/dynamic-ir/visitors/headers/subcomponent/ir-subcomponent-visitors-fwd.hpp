/** ir-subcomponent-visitors-fwd.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_SUBCOMPONENT_VISITORS_FWD_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_SUBCOMPONENT_VISITORS_FWD_HPP

#include "subcomponent/inspectors/ir-subcomponent-inspectors-fwd.hpp"
#include "subcomponent/mutators/ir-subcomponent-mutators-fwd.hpp"

#include "component/ir-component-visitors-fwd.hpp"

namespace gch
{

  template <>
  struct consolidated_visitors<ir_subcomponent>
    : pack_union<exclusive_visitors_t<ir_subcomponent>,
                 consolidated_visitors_t<ir_component>>
  { };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_SUBCOMPONENT_VISITORS_FWD_HPP

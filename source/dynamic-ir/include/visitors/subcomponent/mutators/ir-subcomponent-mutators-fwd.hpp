/** ir-subcomponent-mutators-fwd.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_SUBCOMPONENT_MUTATORS_FWD_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_SUBCOMPONENT_MUTATORS_FWD_HPP

#include "visitors/ir-visitor-fwd.hpp"

namespace gch
{

  template <>
  struct exclusive_mutators<ir_subcomponent>
  {
    using type = visitor_types<>;
  };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_SUBCOMPONENT_MUTATORS_FWD_HPP

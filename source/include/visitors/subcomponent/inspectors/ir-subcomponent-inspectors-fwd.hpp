/** ir-subcomponent-inspectors-fwd.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_SUBCOMPONENT_INSPECTORS_FWD_HPP
#define OCTAVE_IR_IR_SUBCOMPONENT_INSPECTORS_FWD_HPP

#include "visitors/ir-visitor-fwd.hpp"

namespace gch
{

  class ir_block_counter;

  template <>
  struct exclusive_inspectors<ir_subcomponent>
  {
    using type = visitor_types<ir_block_counter>;
  };

  template <>
  struct visitor_traits<ir_block_counter>
    : acceptor_trait<ir_block_counter>
  {
    using result_type      = std::size_t;
    using visitor_category = const_inspector_tag;
  };

}

#endif // OCTAVE_IR_IR_SUBCOMPONENT_INSPECTORS_FWD_HPP

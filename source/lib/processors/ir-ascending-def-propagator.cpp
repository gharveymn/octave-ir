/** ir-def-propagator.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "processors/ir-ascending-def-propagator.hpp"

#include "components/ir-component.hpp"

namespace gch
{

  //
  // ir_def_propagator_frame
  //

  ir_def_propagator_frame::
  ir_def_propagator_frame (ir_block& join_block, ir_link_set<ir_block>&& propagated_incoming)
    : m_join_block          (join_block),
      m_propagated_incoming (propagated_incoming),
      m_merged_incoming     (get_predecessors (join_block) - propagated_incoming)
  { }

}

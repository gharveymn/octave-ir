/** ir-block-common.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_BLOCK_COMMON_HPP
#define OCTAVE_IR_IR_BLOCK_COMMON_HPP

#include <gch/partition/list_partition.hpp>

namespace gch
{

  class ir_incoming_node;
  class ir_def_timeline;

  class ir_block;
  class ir_condition_block;

  enum class ir_instruction_range : std::size_t
  {
    all  = partition_base_index,
    phi  = 0,
    body = 1
  };

}

#endif // OCTAVE_IR_IR_BLOCK_COMMON_HPP

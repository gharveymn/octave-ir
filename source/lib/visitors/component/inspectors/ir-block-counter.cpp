/** ir-block-counter.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/component/inspectors/ir-block-counter.hpp"

#include "components/ir-block.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"

namespace gch
{

  auto
  ir_block_counter::
  operator() (const ir_component& c) const
  -> result_type
  {
    return func.get_body ().accept (*this);
  }

  auto
  ir_block_counter::
  visit (const ir_block& block) const
    -> result_type
  {
    return { nonnull_ptr (as_mutable (block)) };
  }

  auto
  ir_block_counter::
  visit (const ir_component_fork& fork) const
  -> result_type
  {
    result_type ret;
    std::for_each (fork.cases_begin (), fork.cases_end (),
                   [&](const ir_subcomponent& sub) { ret += subcomponent_result (sub); });
    return ret;
  }

  auto
  ir_block_counter::
  visit (const ir_component_loop& loop) const
    -> result_type
  {
    return subcomponent_result (loop.get_condition ());
  }

  auto
  ir_block_counter::
  visit (const ir_component_sequence& seq) const
    -> result_type
  {
    return subcomponent_result (*seq.last ());
  }

  auto
  ir_block_counter::
  subcomponent_result (const ir_subcomponent& sub) const
    -> result_type
  {
    return sub.accept (*this);
  }

}

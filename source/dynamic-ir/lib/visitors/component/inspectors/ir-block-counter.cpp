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

#include <numeric>

namespace gch
{

  auto
  ir_block_counter::
  operator() (const ir_component& c) const
  -> result_type
  {
    return c.accept (*this);
  }

  auto
  ir_block_counter::
  visit (const ir_block&)
    -> result_type
  {
    return 1;
  }

  auto
  ir_block_counter::
  visit (const ir_component_fork& fork)
  -> result_type
  {
    return std::accumulate (fork.cases_begin (), fork.cases_end (), result_type (1),
                            [](result_type res, const ir_subcomponent& sub)
                            {
                              return res + subcomponent_result (sub);
                            });
  }

  auto
  ir_block_counter::
  visit (const ir_component_loop& loop)
    -> result_type
  {
    return subcomponent_result (loop.get_start ())
         + result_type (1) // condition block
         + subcomponent_result (loop.get_body ())
         + subcomponent_result (loop.get_update ());
  }

  auto
  ir_block_counter::
  visit (const ir_component_sequence& seq)
    -> result_type
  {
    return std::accumulate (seq.begin (), seq.end (), result_type (0),
                            [](result_type res, const ir_subcomponent& sub)
                            {
                              return res + subcomponent_result (sub);
                            });
  }

  auto
  ir_block_counter::
  visit (const ir_function& func)
  -> result_type
  {
    return subcomponent_result (func.get_body ());
  }

  auto
  ir_block_counter::
  subcomponent_result (const ir_subcomponent& sub)
    -> result_type
  {
    return sub.accept (ir_block_counter { });
  }

}

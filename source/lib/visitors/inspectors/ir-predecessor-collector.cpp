/** ir-predecessor-collector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/inspectors/ir-predecessor-collector.hpp"

#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"

namespace gch
{

  void
  ir_predecessor_collector::
  visit (const ir_component_fork& fork)
  {
    if (fork.is_condition (get_subcomponent ()))
      set_result (get_predecessors (fork));
    else
      set_result (copy_leaves (fork.get_condition ()));
  }

  void
  ir_predecessor_collector::
  visit (const ir_component_loop& loop)
  {
    using id = ir_component_loop::subcomponent_id;
    switch (loop.get_id (get_subcomponent ()))
    {
      case id::start     : set_result (get_predecessors (loop));                             break;
      case id::condition : set_result (copy_leaves (loop.get_start (), loop.get_update ())); break;
      case id::body      : set_result (copy_leaves (loop.get_condition ()));                 break;
      case id::update    : set_result (copy_leaves (loop.get_body ()));                      break;
    }
  }

  void
  ir_predecessor_collector::
  visit (const ir_component_sequence& seq)
  {
    auto found = seq.find (get_subcomponent ());
    assert (found != seq.end ());
    if (found == seq.begin ())
      set_result (get_predecessors (seq));
    else
      set_result (copy_leaves (*std::prev (found)));
  }

  void
  ir_predecessor_collector::
  visit (const ir_function& func)
  { }

}

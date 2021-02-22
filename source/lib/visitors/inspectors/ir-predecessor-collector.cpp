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
#include "utilities/ir-error.hpp"

namespace gch
{

  auto
  ir_predecessor_collector::
  operator() (void) const
    -> result_type
  {
    return get_parent ().accept (*this);
  }

  auto
  ir_predecessor_collector::
  visit (const ir_component_fork& fork) const
    -> result_type
  {
    if (fork.is_condition (get_subcomponent ()))
      return get_predecessors (fork);
    else
      return copy_leaves (fork.get_condition ());
  }

  auto
  ir_predecessor_collector::
  visit (const ir_component_loop& loop) const
    -> result_type
  {
    using id = ir_component_loop::subcomponent_id;
    switch (loop.get_id (get_subcomponent ()))
    {
      case id::start     : return get_predecessors (loop);
      case id::condition : return copy_leaves (loop.get_start (), loop.get_update ());
      case id::body      : return copy_leaves (loop.get_condition ());
      case id::update    : return copy_leaves (loop.get_body ());
    }
    ir_abort_impossible ();
  }

  auto
  ir_predecessor_collector::
  visit (const ir_component_sequence& seq) const
    -> result_type
  {
    auto found = seq.find (get_subcomponent ());
    assert (found != seq.end ());
    if (found == seq.begin ())
      return get_predecessors (seq);
    else
      return copy_leaves (*std::prev (found));
  }

}

/** ir-leaf-inspector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/inspectors/ir-leaf-inspector.hpp"

#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"

namespace gch
{

  void
  ir_leaf_inspector::
  visit (const ir_component_fork& fork)
  {
    set_result (&get_subcomponent () != &fork.get_condition ());
  }

  void
  ir_leaf_inspector::
  visit (const ir_component_loop& loop)
  {
    set_result (loop.is_condition (get_subcomponent ()));
  }

  void
  ir_leaf_inspector::
  visit (const ir_component_sequence& seq)
  {
    auto found = seq.find (get_subcomponent ());
    assert (found != seq.end ());
    set_result (found == seq.last ());
  }

  void
  ir_leaf_inspector::
  visit (const ir_function& func)
  {
    set_result (true);
  }

}

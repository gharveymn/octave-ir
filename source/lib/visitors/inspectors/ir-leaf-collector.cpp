/** ir-leaf-collector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/inspectors/ir-leaf-collector.hpp"

#include "components/ir-component.hpp"
#include "components/ir-structure.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"

namespace gch
{

  ir_link_set<ir_block>
  ir_leaf_collector::
  operator() (const ir_structure& s)
  {
    m_result.clear ();
    s.accept (*this);
    return std::move (m_result);
  }

  void
  ir_leaf_collector::
  visit (const ir_block& block)
  {
    m_result.emplace (as_mutable (block));
  }

  void
  ir_leaf_collector::
  visit (const ir_component_fork& fork)
  {
    std::for_each (fork.cases_begin (), fork.cases_end (),
                   [this](const ir_subcomponent& sub) { append (sub); });
  }

  void
  ir_leaf_collector::
  visit (const ir_component_loop& loop)
  {
    append (loop.get_condition ());
  }

  void
  ir_leaf_collector::
  visit (const ir_component_sequence& seq)
  {
    append (*seq.last ());
  }

  void
  ir_leaf_collector::
  visit (const ir_function& func)
  {
    append (func.get_body ());
  }

  void
  ir_leaf_collector::
  append (const ir_subcomponent& sub)
  {
    sub.accept (*this);
  }

  ir_link_set<ir_block>
  collect_leaves (const ir_structure& s)
  {
    return ir_leaf_collector { } (s);
  }

}

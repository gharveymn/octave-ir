/** ir-structure-inspector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/inspectors/ir-structure-inspector.hpp"

#include "components/ir-component.hpp"
#include "components/ir-structure.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"

namespace gch
{

  //
  // ir_entry_collector
  //

  const ir_subcomponent&
  ir_entry_collector::
  operator() (const ir_structure& s)
  {
    s.accept (*this);
    assert (m_result != nullptr && "Result should not be nullptr.");
    return *m_result;
  }

  void
  ir_entry_collector::
  visit (const ir_component_fork& fork)
  {
    m_result = &fork.get_condition ();
  }

  void
  ir_entry_collector::
  visit (const ir_component_loop& loop)
  {
    m_result = &loop.get_start ();
  }

  void
  ir_entry_collector::
  visit (const ir_component_sequence& seq)
  {
    m_result = &seq.front ();
  }

  void
  ir_entry_collector::
  visit (const ir_function& func)
  {
    m_result = &func.get_body ();
  }

  ir_subcomponent&
  get_entry_component (ir_structure& s)
  {
    return as_mutable (get_entry_component (std::as_const (s)));
  }

  const ir_subcomponent&
  get_entry_component (const ir_structure& s)
  {
    return ir_entry_collector { } (s);
  }

  ir_block&
  get_entry_block (ir_structure& s)
  {
    return as_mutable (get_entry_block (std::as_const (s)));
  }

  const ir_block&
  get_entry_block (const ir_structure& s)
  {
    nonnull_ptr curr { get_entry_component (s) };

    while (optional_ref structure { maybe_cast<ir_structure> (*curr) })
      curr.emplace (get_entry_component (*structure));

    assert (is_a<ir_block> (*curr) && "Result should be a block.");
    return static_cast<const ir_block&> (*curr);
  }

  bool
  is_entry (const ir_subcomponent& sub)
  {
    return &sub == &get_entry_component (sub.get_parent ());
  }

}

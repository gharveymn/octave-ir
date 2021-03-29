/** ir-entry-collector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/structure/inspectors/ir-entry-collector.hpp"

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

  template <>
  auto
  ir_entry_collector::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_entry_collector>) const
    -> result_type
  {
    return ir_entry_collector::visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_entry_collector::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_entry_collector>) const
    -> result_type
  {
    return ir_entry_collector::visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_entry_collector::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_entry_collector>) const
    -> result_type
  {
    return ir_entry_collector::visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_entry_collector::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_entry_collector>) const
    -> result_type
  {
    return ir_entry_collector::visit (static_cast<concrete_reference> (*this));
  }

  auto
  ir_entry_collector::
  operator() (const ir_structure& s) const
    -> result_type
  {
    return s.accept (*this);
  }

  auto
  ir_entry_collector::
  visit (const ir_component_fork& fork)
    -> result_type
  {
    return fork.get_condition ();
  }

  auto
  ir_entry_collector::
  visit (const ir_component_loop& loop)
    -> result_type
  {
    return loop.get_start ();
  }

  auto
  ir_entry_collector::
  visit (const ir_component_sequence& seq)
    -> result_type
  {
    return seq.front ();
  }

  auto
  ir_entry_collector::
  visit (const ir_function& func)
    -> result_type
  {
    return func.get_body ();
  }

}

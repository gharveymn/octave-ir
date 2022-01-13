/** ir-entry-collector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "structure/inspectors/ir-entry-collector.hpp"

#include "ir-component.hpp"
#include "ir-structure.hpp"
#include "ir-component-fork.hpp"
#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-function.hpp"

namespace gch
{

  template class acceptor<ir_component_fork,     inspector_type<ir_entry_collector>>;
  template class acceptor<ir_component_loop,     inspector_type<ir_entry_collector>>;
  template class acceptor<ir_component_sequence, inspector_type<ir_entry_collector>>;
  template class acceptor<ir_function,           inspector_type<ir_entry_collector>>;

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

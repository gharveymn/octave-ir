/** ir-structure-flattener.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/mutators/ir-structure-flattener.hpp"

#include "components/ir-component.hpp"
#include "components/ir-structure.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"

namespace gch
{

  template <>
  auto
  acceptor<ir_component_fork, ir_structure_flattener>::
  accept (visitor_reference v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<ir_component_loop, ir_structure_flattener>::
  accept (visitor_reference v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<ir_component_sequence, ir_structure_flattener>::
  accept (visitor_reference v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<ir_function, ir_structure_flattener>::
  accept (visitor_reference v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  void
  ir_structure_flattener::
  operator() (ir_structure& s) const
  {
    s.accept (*this);
  }

  void
  ir_structure_flattener::
  visit (ir_component_fork& fork) const
  {
    maybe_recurse (fork.get_condition ());
    std::for_each (fork.cases_begin (), fork.cases_end (),
                   [this](ir_subcomponent& sub) { maybe_recurse (sub); });
  }

  void
  ir_structure_flattener::
  visit (ir_component_loop& loop) const
  {
    maybe_recurse (loop.get_start ());
    maybe_recurse (loop.get_condition ());
    maybe_recurse (loop.get_body ());
    maybe_recurse (loop.get_update ());
  }

  void
  ir_structure_flattener::
  visit (ir_component_sequence& seq) const
  {
    seq.recursive_flatten ();
  }

  void
  ir_structure_flattener::
  visit (ir_function& func) const
  {
    maybe_recurse (func.get_body ());
  }

  void
  ir_structure_flattener::
  maybe_recurse (ir_subcomponent& sub) const
  {
    maybe_cast<ir_structure> (sub) >>= [this](ir_structure& s) { (*this) (s); };
  }

  void
  flatten (ir_structure& s)
  {
    return ir_structure_flattener { } (s);
  }

}

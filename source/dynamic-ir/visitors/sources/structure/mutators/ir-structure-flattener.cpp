/** ir-structure-flattener.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "structure/mutators/ir-structure-flattener.hpp"

#include "ir-component.hpp"
#include "ir-structure.hpp"
#include "ir-component-fork.hpp"
#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-function.hpp"

namespace gch
{

  template class acceptor<ir_component_fork,     mutator_type<ir_structure_flattener>>;
  template class acceptor<ir_component_loop,     mutator_type<ir_structure_flattener>>;
  template class acceptor<ir_component_sequence, mutator_type<ir_structure_flattener>>;
  template class acceptor<ir_function,           mutator_type<ir_structure_flattener>>;

  template <>
  auto
  ir_structure_flattener::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_structure_flattener> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_structure_flattener::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_structure_flattener> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_structure_flattener::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_structure_flattener> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_structure_flattener::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_structure_flattener> v)
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
    std::for_each (fork.cases_begin (), fork.cases_end (), [&](ir_subcomponent& sub) {
      maybe_recurse (sub);
    });
  }

  void
  ir_structure_flattener::
  visit (ir_component_loop& loop) const
  {
    maybe_recurse (loop.get_start ());
    maybe_recurse (loop.get_condition ());
    maybe_recurse (loop.get_body ());
    maybe_recurse (loop.get_update ());
    maybe_recurse (loop.get_after ());
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
    maybe_cast<ir_structure> (sub) >>= [&](ir_structure& s) { (*this) (s); };
  }

}

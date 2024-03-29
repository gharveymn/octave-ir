/** ir-predecessor-collector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include "structure/inspectors/ir-predecessor-collector.hpp"

#include "ir-component-fork.hpp"
#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-function.hpp"
#include "ir-error.hpp"

namespace gch
{

  template class acceptor<ir_component_fork,     inspector_type<ir_predecessor_collector>>;
  template class acceptor<ir_component_loop,     inspector_type<ir_predecessor_collector>>;
  template class acceptor<ir_component_sequence, inspector_type<ir_predecessor_collector>>;
  template class acceptor<ir_function,           inspector_type<ir_predecessor_collector>>;

  template <>
  auto
  ir_predecessor_collector::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_predecessor_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_predecessor_collector::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_predecessor_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_predecessor_collector::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_predecessor_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_predecessor_collector::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_predecessor_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

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
      case id::after     : return { nonnull_ptr { as_mutable (loop.get_condition ()) } };
#ifndef __clang__
      default            : abort<reason::impossible> ();
#endif
    }
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

  auto
  ir_predecessor_collector::
  visit (const ir_function&)
    -> result_type
  {
    return { };
  }

}

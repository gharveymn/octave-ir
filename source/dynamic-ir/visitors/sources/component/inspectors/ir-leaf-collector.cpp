/** ir-leaf-collector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "component/inspectors/ir-leaf-collector.hpp"

#include "ir-block.hpp"
#include "ir-component-fork.hpp"
#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-function.hpp"

namespace gch
{

  template class acceptor<ir_block,              inspector_type<ir_leaf_collector>>;
  template class acceptor<ir_component_fork,     inspector_type<ir_leaf_collector>>;
  template class acceptor<ir_component_loop,     inspector_type<ir_leaf_collector>>;
  template class acceptor<ir_component_sequence, inspector_type<ir_leaf_collector>>;
  template class acceptor<ir_function,           inspector_type<ir_leaf_collector>>;

  template <>
  auto
  ir_leaf_collector::acceptor_type<ir_block>::
  accept (visitor_reference_t<ir_leaf_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_leaf_collector::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_leaf_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_leaf_collector::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_leaf_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_leaf_collector::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_leaf_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_leaf_collector::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_leaf_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  auto
  ir_leaf_collector::
  operator() (const ir_structure& s) const
    -> result_type
  {
    return s.accept (*this);
  }

  auto
  ir_leaf_collector::
  visit (const ir_block& block)
    -> result_type
  {
    return { nonnull_ptr { as_mutable (block) } };
  }

  auto
  ir_leaf_collector::
  visit (const ir_component_fork& fork) const
    -> result_type
  {
    result_type ret;
    std::for_each (fork.cases_begin (), fork.cases_end (),
                   [&](const ir_subcomponent& sub) { ret |= subcomponent_result (sub); });
    return ret;
  }

  auto
  ir_leaf_collector::
  visit (const ir_component_loop& loop) const
    -> result_type
  {
    return subcomponent_result (loop.get_condition ());
  }

  auto
  ir_leaf_collector::
  visit (const ir_component_sequence& seq) const
    -> result_type
  {
    return subcomponent_result (*seq.last ());
  }

  auto
  ir_leaf_collector::
  visit (const ir_function& func) const
    -> result_type
  {
    return subcomponent_result (func.get_body ());
  }

  auto
  ir_leaf_collector::
  subcomponent_result (const ir_subcomponent& sub) const
    -> result_type
  {
    return sub.accept (*this);
  }

  template class acceptor<ir_block,              inspector_type<ir_const_leaf_collector>>;
  template class acceptor<ir_component_fork,     inspector_type<ir_const_leaf_collector>>;
  template class acceptor<ir_component_loop,     inspector_type<ir_const_leaf_collector>>;
  template class acceptor<ir_component_sequence, inspector_type<ir_const_leaf_collector>>;
  template class acceptor<ir_function,           inspector_type<ir_const_leaf_collector>>;

  template <>
  auto
  ir_const_leaf_collector::acceptor_type<ir_block>::
  accept (visitor_reference_t<ir_const_leaf_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_const_leaf_collector::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_const_leaf_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_const_leaf_collector::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_const_leaf_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_const_leaf_collector::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_const_leaf_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_const_leaf_collector::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_const_leaf_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  auto
  ir_const_leaf_collector::
  operator() (const ir_structure& s) const
    -> result_type
  {
    return s.accept (*this);
  }

  auto
  ir_const_leaf_collector::
  visit (const ir_block& block)
    -> result_type
  {
    return { nonnull_ptr { block } };
  }

  auto
  ir_const_leaf_collector::
  visit (const ir_component_fork& fork) const
    -> result_type
  {
    result_type ret;
    std::for_each (fork.cases_begin (), fork.cases_end (),
                   [&](const ir_subcomponent& sub) { ret |= subcomponent_result (sub); });
    return ret;
  }

  auto
  ir_const_leaf_collector::
  visit (const ir_component_loop& loop) const
    -> result_type
  {
    return subcomponent_result (loop.get_condition ());
  }

  auto
  ir_const_leaf_collector::
  visit (const ir_component_sequence& seq) const
    -> result_type
  {
    return subcomponent_result (*seq.last ());
  }

  auto
  ir_const_leaf_collector::
  visit (const ir_function& func) const
    -> result_type
  {
    return subcomponent_result (func.get_body ());
  }

  auto
  ir_const_leaf_collector::
  subcomponent_result (const ir_subcomponent& sub) const
    -> result_type
  {
    return sub.accept (*this);
  }

}

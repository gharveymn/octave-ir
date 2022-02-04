/** ir-descending-forward-mutator.cpp.c
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "component/mutators/ir-descending-forward-mutator.hpp"

#include "ir-block.hpp"
#include "ir-component-fork.hpp"
#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-function.hpp"

namespace gch
{

  template class acceptor<ir_block,              mutator_type<ir_descending_forward_mutator>>;
  template class acceptor<ir_component_fork,     mutator_type<ir_descending_forward_mutator>>;
  template class acceptor<ir_component_loop,     mutator_type<ir_descending_forward_mutator>>;
  template class acceptor<ir_component_sequence, mutator_type<ir_descending_forward_mutator>>;
  template class acceptor<ir_function,           mutator_type<ir_descending_forward_mutator>>;

  template <>
  auto
  ir_descending_forward_mutator::acceptor_type<ir_block>::
  accept (visitor_reference_t<ir_descending_forward_mutator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_descending_forward_mutator::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_descending_forward_mutator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_descending_forward_mutator::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_descending_forward_mutator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_descending_forward_mutator::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_descending_forward_mutator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_descending_forward_mutator::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_descending_forward_mutator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  ir_descending_forward_mutator::
  ir_descending_forward_mutator (const functor_type& functor)
    : m_functor (functor)
  { }

  ir_descending_forward_mutator::
  ir_descending_forward_mutator (functor_type&& functor)
    : m_functor (std::move (functor))
  { }

  auto
  ir_descending_forward_mutator::
  operator() (ir_component& c) const
    -> result_type
  {
    return c.accept (*this);
  }

  auto
  ir_descending_forward_mutator::
  operator() (ir_block& block) const
    -> result_type
  {
    return visit (block);
  }

  auto
  ir_descending_forward_mutator::
  visit (ir_block& block) const
    -> result_type
  {
    return std::invoke (m_functor, block);
  }

  auto
  ir_descending_forward_mutator::
  visit (ir_component_fork& fork) const
    -> result_type
  {
    return dispatch_descender (fork.get_condition ())
       ||  std::all_of (fork.cases_begin (), fork.cases_end (), [&](ir_subcomponent& sub) {
             return dispatch_descender (sub);
           });
  }

  auto
  ir_descending_forward_mutator::
  visit (ir_component_loop& loop) const
    -> result_type
  {
    return dispatch_descender (loop.get_start ()    )
       ||  dispatch_descender (loop.get_condition ())
       ||  dispatch_descender (loop.get_body ()     )
       ||  dispatch_descender (loop.get_update ()   );
  }

  auto
  ir_descending_forward_mutator::
  visit (ir_component_sequence& seq) const
    -> result_type
  {
    return std::any_of (seq.begin (), seq.end (), [&](ir_subcomponent& sub) {
      return dispatch_descender (sub);
    });
  }

  auto
  ir_descending_forward_mutator::
  visit (ir_function& func) const
    -> result_type
  {
    return dispatch_descender (func.get_body ());
  }

  auto
  ir_descending_forward_mutator::
  dispatch_descender (ir_subcomponent& sub) const
    -> result_type
  {
    return sub.accept (*this);
  }

  auto
  ir_descending_forward_mutator::
  dispatch_descender (ir_block& block) const
    -> result_type
  {
    return visit (block);
  }

}

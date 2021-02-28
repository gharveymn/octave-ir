/** ir-forward-mutator.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/structure/mutators/ir-ascending-forward-mutator.hpp"

#include "components/ir-block.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"
#include "visitors/component/mutators/ir-descending-forward-mutator.hpp"

namespace gch
{

  template <>
  auto
  ir_ascending_forward_mutator::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_ascending_forward_mutator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_ascending_forward_mutator::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_ascending_forward_mutator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_ascending_forward_mutator::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_ascending_forward_mutator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_ascending_forward_mutator::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_ascending_forward_mutator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  ir_ascending_forward_mutator::
  ir_ascending_forward_mutator (ir_subcomponent& sub, const functor_type& functor)
    : ir_subcomponent_mutator (sub),
      m_functor         (functor)
  { }

  ir_ascending_forward_mutator::
  ir_ascending_forward_mutator (ir_subcomponent& sub, functor_type&& functor)
    : ir_subcomponent_mutator (sub),
      m_functor         (std::move (functor))
  { }

  auto
  ir_ascending_forward_mutator::
  operator() (void) const
    -> result_type
  {
    return get_parent ().accept (*this);
  }

  auto
  ir_ascending_forward_mutator::
  visit (ir_component_fork& fork) const
    -> result_type
  {
    if (! fork.is_condition (get_subcomponent ()))
      return ascend (fork);

    bool all_stopped = std::all_of (fork.cases_begin (), fork.cases_end (),
                                    [this](ir_subcomponent& sub)
                                    {
                                      return dispatch_descender (sub);
                                    });
    if (! all_stopped)
      return ascend (fork);
  }

  auto
  ir_ascending_forward_mutator::
  visit (ir_component_loop& loop) const
    -> result_type
  {
    using id = ir_component_loop::subcomponent_id;

    bool stop = false;
    switch (loop.get_id (get_subcomponent ()))
    {
      case id::start:     stop =         dispatch_descender (loop.get_condition ());
      case id::condition: stop = stop || dispatch_descender (loop.get_body ()     );
      case id::body:      stop = stop || dispatch_descender (loop.get_update ()   );
      case id::update:    break;
    }

    if (! stop)
      return ascend (loop);
  }

  auto
  ir_ascending_forward_mutator::
  visit (ir_component_sequence& seq) const
    -> result_type
  {
    auto pos = seq.find (get_subcomponent ());
    assert (pos != seq.end ());

    bool found_stop = std::any_of (std::next (pos), seq.end (),
                                   [this](ir_subcomponent& sub)
                                   {
                                     return dispatch_descender (sub);
                                   });
    if (! found_stop)
      return ascend (seq);
  }

  auto
  ir_ascending_forward_mutator::
  ascend (ir_substructure& sub) const
    -> result_type
  {
    return ir_ascending_forward_mutator { sub, m_functor } ();
  }

  bool
  ir_ascending_forward_mutator::
  dispatch_descender (ir_subcomponent& c) const
  {
    return ir_descending_forward_mutator { m_functor } (c);
  }

  bool
  ir_ascending_forward_mutator::
  dispatch_descender (ir_block& block) const
  {
    return ir_descending_forward_mutator { m_functor } (block);
  }

}

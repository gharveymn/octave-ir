/** ir-forward-mutator.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/mutators/ir-forward-mutator.hpp"

#include "components/ir-block.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"

namespace gch
{

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
       ||  std::all_of (fork.cases_begin (), fork.cases_end (),
                        [this](ir_subcomponent& sub) { return dispatch_descender (sub); });
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
    return std::any_of (seq.begin (), seq.end (),
                        [this](ir_subcomponent& sub) { return dispatch_descender (sub); });
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

  auto
  ir_ascending_forward_mutator::
  dispatch_descender (ir_subcomponent& c) const
    ->  descender_type::result_type
  {
    return ir_descending_forward_mutator { m_functor } (c);
  }

  auto
  ir_ascending_forward_mutator::
  dispatch_descender (ir_block& block) const
    ->  descender_type::result_type
  {
    return ir_descending_forward_mutator { m_functor } (block);
  }

}

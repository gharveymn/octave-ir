/** ir-def-propagator.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/structure/mutators/ir-ascending-def-propagator.hpp"

#include "components/ir-block.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"
#include "gch/octave-ir-utilities/ir-error.hpp"
#include "visitors/component/mutators/ir-descending-def-propagator.hpp"

#include <gch/select-iterator.hpp>

#include <numeric>

namespace gch
{

  template <>
  auto
  ir_ascending_def_propagator::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_ascending_def_propagator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_ascending_def_propagator::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_ascending_def_propagator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_ascending_def_propagator::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_ascending_def_propagator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_ascending_def_propagator::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_ascending_def_propagator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  ir_ascending_def_propagator::
  ir_ascending_def_propagator (ir_subcomponent& sub, ir_def_timeline& dominator,
                               ir_link_set<ir_block>&& incoming)
    : ir_subcomponent_mutator (sub),
      m_dominator             (dominator),
      m_incoming_blocks       (std::move (incoming))
  { }

  auto
  ir_ascending_def_propagator::
  operator() (void) const
    -> result_type
  {
    return get_parent ().accept (*this);
  }

  auto
  ir_ascending_def_propagator::
  visit (ir_component_fork& fork) const
    -> result_type
  {
    if (! fork.is_condition (get_subcomponent ()))
      return ascend (fork, std::move (m_incoming_blocks));

    ir_link_set cases_res {
      std::accumulate (fork.cases_begin (), fork.cases_end (), ir_link_set<ir_block> { },
                       [this](auto&& curr_res, ir_subcomponent& sub)
                       {
                         return std::move (curr_res) | dispatch_descender (sub);
                       })
    };

    if (! needs_stop (cases_res))
      return ascend (fork, std::move (cases_res));
  }

  auto
  ir_ascending_def_propagator::
  visit (ir_component_loop& loop) const
    -> result_type
  {
    using id = ir_component_loop::subcomponent_id;

    switch (loop.get_id (get_subcomponent ()))
    {
      case id::start:
      {
        if (needs_stop (dispatch_descender (loop.get_condition ())))
          return;
        if (! needs_stop (dispatch_descender (loop.get_body ())))
          dispatch_descender (loop.get_update ());
        break;
      }
      case id::condition:
      {
        if (! needs_stop (dispatch_descender (loop.get_body ())))
          dispatch_descender (loop.get_update ());
        break;
      }
      case id::body:
      {
        if (needs_stop (dispatch_descender (loop.get_update ()))
        ||  needs_stop (dispatch_descender (loop.get_condition ())))
          return;
        break;
      }
      case id::update:
      {
        if (needs_stop (dispatch_descender (loop.get_condition ())))
          return;
        dispatch_descender (loop.get_body ());
        break;
      }
      default:
        abort<reason::impossible> ();
    }
    return ascend (loop, { loop.get_condition () });
  }

  auto
  ir_ascending_def_propagator::
  visit (ir_component_sequence& seq) const
    -> result_type
  {
    auto pos = seq.find (get_subcomponent ());
    assert (pos != seq.end ());

    ir_link_set<ir_block> res;
    for (auto it = seq.begin (); it != seq.end (); ++it)
    {
      if (needs_stop (res = dispatch_descender (*it)))
        return;
    }
    return ascend (seq, std::move (res));
  }

  auto
  ir_ascending_def_propagator::
  ascend (ir_substructure& sub, ir_link_set<ir_block>&& sub_result) const
    -> result_type
  {
    return ir_ascending_def_propagator { sub, m_dominator, std::move (sub_result) } ();
  }

  ir_link_set<ir_block>
  ir_ascending_def_propagator::
  dispatch_descender (ir_subcomponent& c) const
  {
    // incoming blocks get used up on first call
    return ir_descending_def_propagator { m_dominator,
                                          std::exchange (m_incoming_blocks, { }) } (c);
  }

  ir_link_set<ir_block>
  ir_ascending_def_propagator::
  dispatch_descender (ir_block& block) const
  {
    // incoming blocks get used up on first call
    return ir_descending_def_propagator { m_dominator,
                                          std::exchange (m_incoming_blocks, { }) } (block);
  }

  bool
  ir_ascending_def_propagator::
  needs_stop (const ir_link_set<ir_block>& res) noexcept
  {
    return res.empty ();
  }

  void
  propagate_def (ir_def_timeline& dt)
  {
    ir_ascending_def_propagator { dt.get_block (), dt, { dt.get_block () } } ();
  }

}

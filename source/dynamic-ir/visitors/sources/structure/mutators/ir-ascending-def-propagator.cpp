/** ir-def-propagator.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "structure/mutators/ir-ascending-def-propagator.hpp"

#include "ir-block.hpp"
#include "ir-component-fork.hpp"
#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-function.hpp"
#include "ir-error.hpp"
#include "component/mutators/ir-descending-def-propagator.hpp"

#include <gch/select-iterator.hpp>

#include <numeric>

namespace gch
{

  template class acceptor<ir_component_fork,     mutator_type<ir_ascending_def_propagator>>;
  template class acceptor<ir_component_loop,     mutator_type<ir_ascending_def_propagator>>;
  template class acceptor<ir_component_sequence, mutator_type<ir_ascending_def_propagator>>;
  template class acceptor<ir_function,           mutator_type<ir_ascending_def_propagator>>;

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
  accept (visitor_reference_t<ir_ascending_def_propagator>)
    -> result_type
  {
    return ir_ascending_def_propagator::visit (static_cast<concrete_reference> (*this));
  }

  ir_ascending_def_propagator::
  ir_ascending_def_propagator (ir_subcomponent& sub, ir_def_timeline& dominator,
                               ir_link_set<ir_block>&& incoming)
    : ir_subcomponent_mutator (sub),
      m_dominator             (dominator),
      m_incoming_block_cache (std::move (incoming))
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
      return ascend (fork, std::move (m_incoming_block_cache));

    ir_link_set cases_res {
      std::accumulate (fork.cases_begin (), fork.cases_end (), ir_link_set<ir_block> { },
                       [this](auto&& curr_res, ir_subcomponent& sub) {
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
        if (optional_ref s { maybe_cast<ir_structure> (loop.get_start ()) })
          m_incoming_block_cache = s->get_leaves ();
        else
          m_incoming_block_cache.emplace (static_cast<ir_block&> (loop.get_start ()));

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
        if (needs_stop (dispatch_descender (loop.get_update ())))
          return;

        // Common body-update
        if (optional_ref s { maybe_cast<ir_structure> (loop.get_update ()) })
          m_incoming_block_cache = s->get_leaves ();
        else
          m_incoming_block_cache.emplace (static_cast<ir_block&> (loop.get_update ()));

        if (needs_stop (dispatch_descender (loop.get_condition ())))
          return;
        // End common body-update

        break;
      }
      case id::update:
      {
        // Common body-update
        if (optional_ref s { maybe_cast<ir_structure> (loop.get_update ()) })
          m_incoming_block_cache = s->get_leaves ();
        else
          m_incoming_block_cache.emplace (static_cast<ir_block&> (loop.get_update ()));

        if (needs_stop (dispatch_descender (loop.get_condition ())))
          return;
        // End common body-update

        dispatch_descender (loop.get_body ());
        break;
      }
      case id::after:
        // Do nothing.
        break;
#ifndef __clang__
      default:
        abort<reason::impossible> ();
#endif
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
    for (auto it = std::next (pos); it != seq.end (); ++it)
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
    return ir_descending_def_propagator {
      m_dominator,
      std::exchange (m_incoming_block_cache, { })
    } (c);
  }

  ir_link_set<ir_block>
  ir_ascending_def_propagator::
  dispatch_descender (ir_block& block) const
  {
    // incoming blocks get used up on first call
    return ir_descending_def_propagator {
      m_dominator,
      std::exchange (m_incoming_block_cache, { })
    } (block);
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

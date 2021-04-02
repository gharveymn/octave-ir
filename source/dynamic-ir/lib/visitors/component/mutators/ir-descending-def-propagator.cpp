/** ir-descending-def-propagator.cpp.c
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/component/mutators/ir-descending-def-propagator.hpp"

#include "components/ir-block.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"

#include <gch/select-iterator.hpp>

#include <numeric>

namespace gch
{

  template <>
  auto
  ir_descending_def_propagator::acceptor_type<ir_block>::
  accept (visitor_reference_t<ir_descending_def_propagator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_descending_def_propagator::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_descending_def_propagator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_descending_def_propagator::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_descending_def_propagator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_descending_def_propagator::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_descending_def_propagator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_descending_def_propagator::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_descending_def_propagator> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  ir_descending_def_propagator::
  ir_descending_def_propagator (ir_def_timeline& dominator, ir_link_set<ir_block>&& incoming)
    : m_dominator       (dominator),
      m_incoming_blocks (std::move (incoming))
  { }

  auto
  ir_descending_def_propagator::
  operator() (ir_component& c) const
    -> result_type
  {
    return c.accept (*this);
  }

  auto
  ir_descending_def_propagator::
  operator() (ir_block& block) const
    -> result_type
  {
    return visit (block);
  }

  auto
  ir_descending_def_propagator::
  visit (ir_block& block) const
    -> result_type
  {
    ir_variable& var      = m_dominator.get_variable ();
    auto rebinder = applied { [&](auto&&, ir_incoming_node& node) { node.rebind (m_dominator); } };

    if (optional_ref block_dt { block.maybe_get_def_timeline (var) })
    {
      if (block_dt->has_incoming_blocks ())
      {
        if (m_incoming_blocks.empty ())
        {
          // replace all
          std::for_each (block_dt->incoming_begin (), block_dt->incoming_end (), rebinder);
        }
        else
        {
          // targeted replacement
          small_vector<std::reference_wrapper<ir_def_timeline::incoming_value_type>> replaced_nodes;
          replaced_nodes.reserve (block_dt->num_incoming_blocks ());

          std::set_intersection (
            block_dt->incoming_begin (), block_dt->incoming_begin (),
            m_incoming_blocks.begin (),  m_incoming_blocks.end (),
            std::back_inserter (replaced_nodes),
            overloaded
            {
              [](nonnull_ptr<ir_block> lhs, const auto&           rhs) { return lhs == rhs.first; },
              [](const auto&           lhs, nonnull_ptr<ir_block> rhs) { return lhs.first == rhs; }
            });

          std::for_each (replaced_nodes.begin (), replaced_nodes.end (),
                         [&](auto ref) { rebinder (ref.get ()); });

          // m_incoming_blocks is a transient cache, so clear it after first use (on entry)
          m_incoming_blocks.clear ();
        }
      }
      else
        assert (block_dt->has_local_timelines ());
      return { };
    }
    return { block };
  }

  auto
  ir_descending_def_propagator::
  visit (ir_component_fork& fork) const
    -> result_type
  {
    result_type cond_res { dispatch_descender (fork.get_condition ()) };
    if (cond_res.empty ())
      return cond_res;

    return std::accumulate (fork.cases_begin (), fork.cases_end (), result_type { },
                            [this](auto&& curr_res, ir_subcomponent& sub)
                            {
                              return std::move (curr_res) | dispatch_descender (sub);
                            });
  }

  auto
  ir_descending_def_propagator::
  visit (ir_component_loop& loop) const
    -> result_type
  {
    result_type res;

    res = dispatch_descender (loop.get_start ());
    if (res.empty ())
      return res;

    res = dispatch_descender (loop.get_condition ());
    if (res.empty ())
      return res;

    res = dispatch_descender (loop.get_body ());
    if (res.empty ())
      return res;

    return dispatch_descender (loop.get_update ());
  }

  auto
  ir_descending_def_propagator::
  visit (ir_component_sequence& seq) const
    -> result_type
  {
    result_type res;
    for (auto it = seq.begin (); it != seq.end (); ++it)
    {
      res = dispatch_descender (*it);
      if (res.empty ())
        return res;
    }
    return res;
  }

  auto
  ir_descending_def_propagator::
  visit (ir_function& func) const
    -> result_type
  {
    return dispatch_descender (func.get_body ());
  }

  auto
  ir_descending_def_propagator::
  dispatch_descender (ir_subcomponent& sub) const
    -> result_type
  {
    return sub.accept (*this);
  }

  auto
  ir_descending_def_propagator::
  dispatch_descender (ir_block& block) const
    -> result_type
  {
    return visit (block);
  }

}
/** ir-def-propagator.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/structure/mutators/ir-def-propagator.hpp"

#include "components/ir-block.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"
#include "utilities/ir-error.hpp"
#include "visitors/ir-visitor-fwd.hpp"

#include <gch/select-iterator.hpp>

#include <numeric>

namespace gch
{

  //
  // ir_descending_def_propagator
  //

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
    ir_variable& var = m_dominator.get_variable ();
    if (optional_ref block_dt { block.maybe_get_def_timeline (var) })
    {
      if (block_dt->has_incoming_blocks ())
      {
        if (m_incoming_blocks.empty ())
        {
          // replace all
          std::for_each (selected<ir_incoming_node> (block_dt->incoming_begin ()),
                         selected<ir_incoming_node> (block_dt->incoming_begin ()),
                         [this](ir_incoming_node& node)
                         {
                           node.clear ();
                           node.add_predecessor (m_dominator);
                         });
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
            [](const auto& lhs, const auto& rhs)
            {
              if constexpr (std::is_same_v<const nonnull_ptr<ir_block>&, decltype (lhs)>)
                return lhs == std::get<const nonnull_cptr<ir_block>> (rhs);
              else
                return std::get<const nonnull_cptr<ir_block>> (lhs) == rhs;
            });

          std::for_each (replaced_nodes.begin (), replaced_nodes.end (),
                         [this](ir_def_timeline::incoming_reference pair)
                         {
                           auto&& [blk, node] = pair;
                           node.clear ();
                           node.add_predecessor (m_dominator);
                         });

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

  //
  // ir_ascending_def_propagator
  //

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

    descender_type::result_type cases_res {
      std::accumulate (fork.cases_begin (), fork.cases_end (), descender_type::result_type { },
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
        abort::ir_impossible ();
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

    descender_type::result_type res;
    for (auto it = seq.begin (); it != seq.end (); ++it)
    {
      if (needs_stop (res = dispatch_descender (*it)))
        return;
    }
    return ascend (seq, std::move (res));
  }

  auto
  ir_ascending_def_propagator::
  ascend (ir_substructure& sub, descender_type::result_type&& sub_result) const
    -> result_type
  {
    return ir_ascending_def_propagator { sub, m_dominator, std::move (sub_result) } ();
  }

  auto
  ir_ascending_def_propagator::
  dispatch_descender (ir_subcomponent& c) const
    ->  descender_type::result_type
  {
    // incoming blocks get used up on first call
    return ir_descending_def_propagator { m_dominator,
                                          std::exchange (m_incoming_blocks, { }) } (c);
  }

  auto
  ir_ascending_def_propagator::
  dispatch_descender (ir_block& block) const
    ->  descender_type::result_type
  {
    // incoming blocks get used up on first call
    return ir_descending_def_propagator { m_dominator,
                                          std::exchange (m_incoming_blocks, { }) } (block);
  }

  bool
  ir_ascending_def_propagator::
  needs_stop (const descender_type::result_type& res) noexcept
  {
    return res.empty ();
  }

  void
  propagate_def (ir_def_timeline& dt)
  {
    ir_ascending_def_propagator { dt.get_block (), dt, { dt.get_block () } } ();
  }

}

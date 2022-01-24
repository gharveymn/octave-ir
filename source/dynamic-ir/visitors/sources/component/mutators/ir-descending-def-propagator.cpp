/** ir-descending-def-propagator.cpp.c
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "component/mutators/ir-descending-def-propagator.hpp"

#include "ir-block.hpp"
#include "ir-component-fork.hpp"
#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-function.hpp"

#include <gch/select-iterator.hpp>

#include <numeric>

namespace gch
{

  template class acceptor<ir_block,              mutator_type<ir_descending_def_propagator>>;
  template class acceptor<ir_component_fork,     mutator_type<ir_descending_def_propagator>>;
  template class acceptor<ir_component_loop,     mutator_type<ir_descending_def_propagator>>;
  template class acceptor<ir_component_sequence, mutator_type<ir_descending_def_propagator>>;
  template class acceptor<ir_function,           mutator_type<ir_descending_def_propagator>>;

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
      if (block_dt->has_incoming ())
      {
        if (m_incoming_blocks.empty ())
        {
          // replace all
          std::for_each (block_dt->incoming_begin (), block_dt->incoming_end (), applied {
            [&](auto&&, ir_incoming_node& node) { node.rebind (m_dominator); }
          });
        }
        else
        {
          // targeted replacement
          // small_vector<nonnull_ptr<ir_block>> added_nodes;
          // added_nodes.reserve (m_incoming_blocks.size ());
          //
          // std::for_each (m_incoming_blocks.begin (),  m_incoming_blocks.end (),
          //                [&](nonnull_ptr<ir_block> inc_block) {
          //   auto found = std::find_if (block_dt->incoming_begin (), block_dt->incoming_end (),
          //                              [inc_block](const auto& inc_pair) {
          //     return std::get<const nonnull_cptr<ir_block>> (inc_pair) == inc_block;
          //   });
          //
          //   if (found != block_dt->incoming_end ())
          //     std::get<ir_incoming_node> (*found).rebind (m_dominator);
          //   else
          //     added_nodes.push_back (inc_block);
          // });
          //
          // std::for_each (added_nodes.begin (), added_nodes.end (),
          //                [&](nonnull_ptr<ir_block> inc_block) {
          //   block_dt->append_incoming (*inc_block, m_dominator);
          // });

          std::for_each (m_incoming_blocks.begin (),  m_incoming_blocks.end (),
                         [&](nonnull_ptr<ir_block> inc_block) {
            auto found = std::find_if (block_dt->incoming_begin (), block_dt->incoming_end (),
                                       [inc_block](const auto& inc_pair) {
              return std::get<const nonnull_cptr<ir_block>> (inc_pair) == inc_block;
            });

            if (found != block_dt->incoming_end ())
              std::get<ir_incoming_node> (*found).rebind (m_dominator);
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
                            [this](auto&& curr_res, ir_subcomponent& sub) {
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

/** ir-descending-def-resolution-builder.cpp.c
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "component/inspectors/ir-descending-def-resolution-builder.hpp"

#include "ir-block.hpp"
#include "ir-component-fork.hpp"
#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-function.hpp"
#include "ir-def-resolution.hpp"
#include "ir-error.hpp"

namespace gch
{

  template class acceptor<ir_block,              inspector_type<ir_descending_def_resolution_builder>>;
  template class acceptor<ir_component_fork,     inspector_type<ir_descending_def_resolution_builder>>;
  template class acceptor<ir_component_loop,     inspector_type<ir_descending_def_resolution_builder>>;
  template class acceptor<ir_component_sequence, inspector_type<ir_descending_def_resolution_builder>>;
  template class acceptor<ir_function,           inspector_type<ir_descending_def_resolution_builder>>;

  template <>
  auto
  ir_descending_def_resolution_builder::acceptor_type<ir_block>::
  accept (visitor_reference_t<ir_descending_def_resolution_builder> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_descending_def_resolution_builder::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_descending_def_resolution_builder> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_descending_def_resolution_builder::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_descending_def_resolution_builder> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_descending_def_resolution_builder::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_descending_def_resolution_builder> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_descending_def_resolution_builder::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_descending_def_resolution_builder> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  ir_descending_def_resolution_builder::
  ir_descending_def_resolution_builder (const ir_variable& var)
    : m_variable (var)
  { }

  auto
  ir_descending_def_resolution_builder::
  operator() (const ir_component& c) const &&
    -> result_type
  {
    return c.accept (*this);
  }

  auto
  ir_descending_def_resolution_builder::
  operator() (const ir_block& block) const &&
    -> result_type
  {
    return visit (block);
  }

  auto
  ir_descending_def_resolution_builder::
  visit (const ir_block& block) const
    -> result_type
  {
    if (optional_ref dt { block.maybe_get_def_timeline (get_variable ()) })
    {
      if (dt->has_outgoing_timeline ())
      {
        return { { get_variable (), block, { nonnull_ptr { *dt } } },
                 result_type::join::yes,
                 result_type::resolvable::yes };
      }
    }

    return { { get_variable (), block },
             result_type::join::no,
             result_type::resolvable::no };
  }

  auto
  ir_descending_def_resolution_builder::
  visit (const ir_component_fork& fork) const
    -> result_type
  {
    small_vector<result_type> case_results;
    std::transform (fork.cases_begin (), fork.cases_end (), std::back_inserter (case_results),
                    [this](const ir_subcomponent& sub) { return dispatch_descender (sub); });

    assert (! case_results.empty ());

    ir_def_resolution_stack stack         { get_variable () };
    bool                    needs_join    { false           };
    bool                    is_resolvable { true            };

    std::for_each (case_results.begin (), case_results.end (),
                   [&](result_type& r)
                   {
                     stack.add_leaf  (r.release_stack ());
                     needs_join    |= r.needs_join ();
                     is_resolvable &= r.is_resolvable ();
                   });

    if (! is_resolvable)
    {
      result_type cond_res { dispatch_descender (fork.get_condition ()) };
      if (cond_res.is_resolvable ())
      {
        stack.set_block_resolution (cond_res.release_stack ().get_block_resolution ());
        needs_join    = true;
        is_resolvable = true;
      }
    }

    return { std::move (stack),
             static_cast<result_type::join>       (needs_join),
             static_cast<result_type::resolvable> (is_resolvable) };
  }

  auto
  ir_descending_def_resolution_builder::
  visit (const ir_component_loop& loop) const
    -> result_type
  {
    result_type cond_res { dispatch_descender (loop.get_condition ()) };
    if (cond_res.is_resolvable ())
      return cond_res;

    ir_def_resolution_stack stack         { get_variable () };
    bool                    needs_join    { false           };
    bool                    is_resolvable { false           };

    // set leaf block to the condition block
    stack.add_leaf (loop.get_condition ());

    result_type             update_res      { dispatch_descender (loop.get_update ()) };
    ir_def_resolution_stack body_stack      { update_res.release_stack ()             };
    bool                    body_needs_join { update_res.needs_join ()                };

    if (! update_res.is_resolvable ())
    {
      result_type body_res { dispatch_descender (loop.get_body ()) };
      if (body_res.needs_join ())
      {
        body_stack.add_leaf (std::exchange (body_stack, ir_def_resolution_stack (get_variable ())));
        body_stack.push_frame (get_entry_block (loop.get_update ()), body_res.release_stack ());
        body_needs_join = true;
      }
    }

    result_type start_res { dispatch_descender (loop.get_start ()) };
    if (body_needs_join || start_res.needs_join ())
    {
      stack.push_frame (loop.get_condition (), std::move (body_stack));
      stack.push_frame (loop.get_condition (), start_res.release_stack ());
      needs_join    = true;
      is_resolvable = start_res.is_resolvable ();
    }

    return { std::move (stack),
             static_cast<result_type::join>       (needs_join),
             static_cast<result_type::resolvable> (is_resolvable) };
  }

  auto
  ir_descending_def_resolution_builder::
  visit (const ir_component_sequence& seq) const
    -> result_type
  {
    result_type leaf_res { dispatch_descender (seq.back ()) };
    if (leaf_res.is_resolvable ())
      return leaf_res;

    ir_def_resolution_stack stack         { get_variable ()        };
    bool                    needs_join    { leaf_res.needs_join () };
    bool                    is_resolvable { false                  };

    for (auto rit = std::next (seq.rbegin ()); rit != seq.rend () && ! is_resolvable; ++rit)
    {
      result_type res { dispatch_descender (*rit) };
      if (res.needs_join ())
        stack.push_frame (get_entry_block (*std::prev (rit)), res.release_stack ());

      needs_join    |= res.needs_join ();
      is_resolvable = res.is_resolvable ();
    }

    if (needs_join)
     stack.add_leaf (leaf_res.release_stack ());

    return { std::move (stack),
             static_cast<result_type::join>       (needs_join),
             static_cast<result_type::resolvable> (is_resolvable) };
  }

  auto
  ir_descending_def_resolution_builder::
  visit (const ir_function& func) const
    -> result_type
  {
    return dispatch_descender (func.get_body ());
  }

  auto
  ir_descending_def_resolution_builder::
  dispatch_descender (const ir_subcomponent& sub) const
    -> result_type
  {
    return sub.accept (*this);
  }

  auto
  ir_descending_def_resolution_builder::
  dispatch_descender (const ir_block& block) const
    -> result_type
  {
    return visit (block);
  }

  const ir_variable&
  ir_descending_def_resolution_builder::
  get_variable () const noexcept
  {
    return m_variable;
  }

}

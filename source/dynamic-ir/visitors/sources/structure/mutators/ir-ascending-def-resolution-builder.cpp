/** ir-def-resolution-builder.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "structure/mutators/ir-ascending-def-resolution-builder.hpp"

#include "ir-block.hpp"
#include "ir-component-fork.hpp"
#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-function.hpp"
#include "ir-def-resolution.hpp"
#include "ir-error.hpp"
#include "component/mutators/ir-descending-def-resolution-builder.hpp"

namespace gch
{

  template class acceptor<ir_component_fork,     mutator_type<ir_ascending_def_resolution_builder>>;
  template class acceptor<ir_component_loop,     mutator_type<ir_ascending_def_resolution_builder>>;
  template class acceptor<ir_component_sequence, mutator_type<ir_ascending_def_resolution_builder>>;
  template class acceptor<ir_function,           mutator_type<ir_ascending_def_resolution_builder>>;

  template <>
  auto
  ir_ascending_def_resolution_builder::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_ascending_def_resolution_builder> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_ascending_def_resolution_builder::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_ascending_def_resolution_builder> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_ascending_def_resolution_builder::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_ascending_def_resolution_builder> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_ascending_def_resolution_builder::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_ascending_def_resolution_builder> v)
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  ir_ascending_def_resolution_builder::
  ir_ascending_def_resolution_builder (ir_subcomponent& sub, ir_variable& var)
    : ir_subcomponent_mutator (sub),
      m_variable        (var)
  { }

  auto
  ir_ascending_def_resolution_builder::
  operator() (void) const
    -> result_type
  {
    return get_parent ().accept (*this);
  }

  auto
  ir_ascending_def_resolution_builder::
  visit (ir_component_fork& fork) const
    -> result_type
  {
    if (! fork.is_condition (get_subcomponent ()))
    {
      result_type cond_res { dispatch_descender (fork.get_condition ()) };
      if (cond_res.is_resolvable ())
        return cond_res;
    }
    return ascend (fork);
  }

  auto
  ir_ascending_def_resolution_builder::
  visit (ir_component_loop& loop) const
    -> result_type
  {
    using id = ir_component_loop::subcomponent_id;

    ir_def_resolution_stack stack         { get_variable () };
    bool                    needs_join    { false           };
    bool                    is_resolvable { false           };

    switch (loop.get_id (get_subcomponent ()))
    {
      case id::condition:
      {
        result_type             update_res      { dispatch_descender (loop.get_update ()) };
        ir_def_resolution_stack body_stack      { update_res.release_stack ()             };
        bool                    body_needs_join { update_res.needs_join ()                };

        if (! update_res.is_resolvable ())
        {
          result_type body_res { dispatch_descender (loop.get_body ()) };
          if (body_res.needs_join ())
          {
            auto tmp = std::exchange (body_stack, ir_def_resolution_stack (get_variable ()));
            body_stack.add_leaf (std::move (tmp));
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
        break;
      }
      case id::update:
      {
        // body
        result_type body_res { dispatch_descender (loop.get_body ()) };
        if (body_res.needs_join ())
        {
          stack.push_frame (get_entry_block (loop.get_update ()), body_res.release_stack ());
          needs_join    = true;
          is_resolvable = body_res.is_resolvable ();
          if (body_res.is_resolvable ())
            break;
        }

        // condition
        result_type cond_res { dispatch_descender (loop.get_condition ()) };
        if (cond_res.is_resolvable ())
        {
          stack.set_block_resolution (cond_res.release_stack ().get_block_resolution ());
          needs_join    = true;
          is_resolvable = true;
          break;
        }
        assert (! cond_res.needs_join ());

        // start
        result_type start_res { dispatch_descender (loop.get_start ()) };
        if (start_res.needs_join ())
          stack.push_frame (loop.get_condition (), start_res.release_stack ());
        needs_join    |= start_res.needs_join ();
        is_resolvable = start_res.is_resolvable ();
        break;
      }
      case id::body:
      {
        // condition
        result_type cond_res { dispatch_descender (loop.get_condition ()) };
        if (cond_res.is_resolvable ())
          return cond_res;
        assert (! cond_res.needs_join ());

        result_type update_res { dispatch_descender (loop.get_update ()) };
        result_type start_res  { dispatch_descender (loop.get_start ()) };

        if (update_res.needs_join () || start_res.needs_join ())
        {
          stack.push_frame (loop.get_condition (), update_res.release_stack ());
          stack.push_frame (loop.get_condition (), start_res.release_stack ());
          needs_join    = true;
          is_resolvable = start_res.is_resolvable ();
        }
        break;
      }
      case id::start:
      {
        return ascend (loop);
      }
      default:
        abort<reason::impossible> ();
    }
    return maybe_ascend (loop, { std::move (stack),
                                 static_cast<result_type::join>       (needs_join),
                                 static_cast<result_type::resolvable> (is_resolvable) });
  }

  auto
  ir_ascending_def_resolution_builder::
  visit (ir_component_sequence& seq) const
    -> result_type
  {
    const ir_component_sequence::riter rpos { seq.find (get_subcomponent ()) };
    assert (rpos != seq.rend ());

    ir_def_resolution_stack stack         { get_variable () };
    bool                    needs_join    { false           };
    bool                    is_resolvable { false           };

    for (auto rit = rpos; rit != seq.rend () && ! is_resolvable; ++rit)
    {
      result_type res { dispatch_descender (*rit) };
      if (res.needs_join ())
        stack.push_frame (get_entry_block (*std::prev (rit)), res.release_stack ());

      needs_join    |= res.needs_join ();
      is_resolvable = res.is_resolvable ();
    }

    return maybe_ascend (seq, { std::move (stack),
                                static_cast<result_type::join>       (needs_join),
                                static_cast<result_type::resolvable> (is_resolvable) });
  }

  auto
  ir_ascending_def_resolution_builder::
  visit (ir_function&) const
    -> result_type
  {
    return { { get_variable () },
             result_type::join::      no,
             result_type::resolvable::no };
  }

  auto
  ir_ascending_def_resolution_builder::
  maybe_ascend (ir_substructure& sub, ir_def_resolution_build_result&& sub_result) const
    -> result_type
  {
    if (sub_result.is_resolvable ())
      return std::move (sub_result);

    result_type ascent_res { ascend (sub) };
    if (! ascent_res.needs_join ())
      return std::move (sub_result);

    ir_def_resolution_stack stack { ascent_res.release_stack () };

    assert (! stack.has_leaves ());
    stack.add_leaf (sub_result.release_stack ());

    return { std::move (stack),
             result_type::join::yes,
             ascent_res.get_resolvable_state () };
  }

  auto
  ir_ascending_def_resolution_builder::
  ascend (ir_substructure& sub) const
    -> result_type
  {
    return ir_ascending_def_resolution_builder { sub, get_variable () } ();
  }

  ir_def_resolution_build_result
  ir_ascending_def_resolution_builder::
  dispatch_descender (ir_subcomponent& c) const
  {
    return ir_descending_def_resolution_builder { get_variable () } (c);
  }

ir_def_resolution_build_result
  ir_ascending_def_resolution_builder::
  dispatch_descender (ir_block& block) const
  {
    return ir_descending_def_resolution_builder { get_variable () } (block);
  }

  ir_variable&
  ir_ascending_def_resolution_builder::
  get_variable (void) const noexcept
  {
    return m_variable;
  }

}

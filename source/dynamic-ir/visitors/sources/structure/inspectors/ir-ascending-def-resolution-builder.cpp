/** ir-def-resolution-builder.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "structure/inspectors/ir-ascending-def-resolution-builder.hpp"

#include "ir-block.hpp"
#include "ir-component-fork.hpp"
#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-function.hpp"
#include "ir-def-resolution.hpp"
#include "ir-error.hpp"
#include "component/inspectors/ir-descending-def-resolution-builder.hpp"

// FIXME: There are a lot of unnecessary joins going on with this system, so it will need a rework.

namespace gch
{

  template class acceptor<ir_component_fork,     inspector_type<ir_ascending_def_resolution_builder>>;
  template class acceptor<ir_component_loop,     inspector_type<ir_ascending_def_resolution_builder>>;
  template class acceptor<ir_component_sequence, inspector_type<ir_ascending_def_resolution_builder>>;
  template class acceptor<ir_function,           inspector_type<ir_ascending_def_resolution_builder>>;

  template <>
  auto
  ir_ascending_def_resolution_builder::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_ascending_def_resolution_builder> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_ascending_def_resolution_builder::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_ascending_def_resolution_builder> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_ascending_def_resolution_builder::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_ascending_def_resolution_builder> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_ascending_def_resolution_builder::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_ascending_def_resolution_builder> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  ir_ascending_def_resolution_builder::
  ir_ascending_def_resolution_builder (const ir_subcomponent& sub,  const ir_variable& var)
    : ir_subcomponent_inspector (sub),
      m_sub_result              (var, result_type::join<false>, result_type::resolvable<false>)
  { }

  ir_ascending_def_resolution_builder::
  ir_ascending_def_resolution_builder (const ir_subcomponent& sub, result_type&& sub_result)
    : ir_subcomponent_inspector (sub),
      m_sub_result              (std::move (sub_result))
  { }

  auto
  ir_ascending_def_resolution_builder::
  operator() (void)
    -> result_type
  {
    return get_parent ().accept (*this);
  }

  auto
  ir_ascending_def_resolution_builder::
  visit (const ir_component_fork& fork)
    -> result_type
  {
    if (! fork.is_condition (get_subcomponent ()))
    {
      result_type cond_res { dispatch_descender (fork.get_condition ()) };
      if (cond_res.needs_join ())
      {
        m_sub_result.get_stack ().push_frame (get_entry_block (get_subcomponent ()),
                                              cond_res.release_stack ());
        m_sub_result.set_join (cond_res.needs_join ());
        m_sub_result.set_resolvable (cond_res.is_resolvable ());
        if (cond_res.is_resolvable ())
          return std::move (m_sub_result);
      }
    }
    return ascend (fork);
  }

  auto
  ir_ascending_def_resolution_builder::
  visit (const ir_component_loop& loop)
    -> result_type
  {
    using id = ir_component_loop::subcomponent_id;

    switch (loop.get_id (get_subcomponent ()))
    {
      case id::after:
      {
        // condition
        result_type cond_res { dispatch_descender (loop.get_condition ()) };
        if (cond_res.is_resolvable ())
        {
          // FIXME: This doesn't need a join.
          m_sub_result.get_stack ().push_frame (get_entry_block (loop.get_body ()),
                                                cond_res.release_stack ());
          return {
            m_sub_result.release_stack (),
            result_type::join<true>,
            result_type::resolvable<true>
          };
        }
        [[fallthrough]];
      }
      case id::condition:
      {
        ir_def_resolution_stack& stack = m_sub_result.get_stack ();
        result_type             update_res        { dispatch_descender (loop.get_update ()) };
        result_type::join_type  update_needs_join { update_res.needs_join ()                };

        if (! update_res.is_resolvable ())
        {
          result_type body_res { dispatch_descender (loop.get_body ()) };
          if (body_res.needs_join ())
          {
            update_res.get_stack ().dominate_with (get_entry_block (loop.get_update ()),
                                                   body_res.release_stack ());
            update_needs_join = true;
          }
        }

        result_type start_res { dispatch_descender (loop.get_start ()) };
        if (update_needs_join || start_res.needs_join ())
        {
          stack.push_frame (loop.get_condition (), update_res.release_stack ());
          stack.push_frame (loop.get_condition (), start_res.release_stack ());
          m_sub_result.set_join (true);
          m_sub_result.set_resolvable (start_res.is_resolvable ());
        }

        // FIXME: Wrong.
        return ascend (loop);
      }
      case id::update:
      {
        ir_def_resolution_stack& stack = m_sub_result.get_stack ();

        // body
        result_type body_res { dispatch_descender (loop.get_body ()) };
        if (body_res.needs_join ())
        {
          stack.push_frame (get_entry_block (loop.get_update ()), body_res.release_stack ());
          m_sub_result.set_join (true);
          m_sub_result.set_resolvable (body_res.is_resolvable ());
          if (body_res.is_resolvable ())
            return std::move (m_sub_result);
        }

        // condition
        result_type cond_res { dispatch_descender (loop.get_condition ()) };
        if (cond_res.is_resolvable ())
        {
          // FIXME: This doesn't need a join.
          m_sub_result.get_stack ().push_frame (get_entry_block (loop.get_body ()),
                                                cond_res.release_stack ());
          return {
            m_sub_result.release_stack (),
            result_type::join<true>,
            result_type::resolvable<true>
          };
        }
        assert (! cond_res.needs_join ());

        // FIXME: This should only be dispatched up to the relevant block.
        result_type update_res { dispatch_descender (loop.get_update ()) };

        // start
        result_type start_res { dispatch_descender (loop.get_start ()) };

        if (start_res.needs_join () || update_res.needs_join ())
        {
          stack.push_frame (loop.get_condition (), update_res.release_stack ());
          stack.push_frame (loop.get_condition (), start_res.release_stack ());
          m_sub_result.set_join (true);
          m_sub_result.set_resolvable (start_res.is_resolvable ());

          if (start_res.is_resolvable ())
            return std::move (m_sub_result);
        }

        return ascend (loop);
      }
      case id::body:
      {
        // condition
        result_type cond_res { dispatch_descender (loop.get_condition ()) };
        if (cond_res.is_resolvable ())
        {
          m_sub_result.get_stack ().push_frame (get_entry_block (loop.get_body ()),
                                                cond_res.release_stack ());
          return {
            m_sub_result.release_stack (),
            result_type::join<true>,
            result_type::resolvable<true>
          };
        }

        // We should never need a join because the condition is a single block.
        assert (! cond_res.needs_join ());

        ir_def_resolution_stack stack      { get_variable () };
        result_type::join_type  needs_join { result_type::join<false> };

        // FIXME: This should only be dispatched up to the relevant block.

        result_type            update_res        { dispatch_descender (loop.get_update ()) };
        result_type::join_type update_needs_join { update_res.needs_join ()                };

        if (! update_res.is_resolvable ())
        {
          result_type body_res { dispatch_descender (loop.get_body ()) };
          if (body_res.needs_join ())
          {
            update_res.get_stack ().dominate_with (get_entry_block (loop.get_update ()),
                                                   body_res.release_stack ());
            update_needs_join = result_type::join<true>;
          }
        }

        // Assume that we need a join at the condition block.
        stack.push_frame (loop.get_condition (), update_res.release_stack ());

        // If the sub-result needs a join, then push the frame to supercede the update stack.
        if (m_sub_result.needs_join ())
        {
          stack.push_frame (get_entry_block (loop.get_update ()), m_sub_result.release_stack ());
          needs_join = result_type::join<true>;
        }
        else
          stack.add_leaf (m_sub_result.release_stack ());

        result_type start_res  { dispatch_descender (loop.get_start ()) };
        stack.push_frame (loop.get_condition (), start_res.release_stack ());
        stack.add_leaf (loop.get_condition ());

        if (start_res.is_resolvable ())
          return { std::move (stack), result_type::join<true>, result_type::resolvable<true> };

        // Otherwise, we need to ascend.

        needs_join |= update_res.needs_join () || start_res.needs_join ();

        result_type::join_type sub_needs_join = m_sub_result.needs_join ();
        result_type::resolvable_type sub_is_resolvable = m_sub_result.is_resolvable ();

        m_sub_result = {
          std::move (stack),
          result_type::resolvable<true>,
          result_type::resolvable<false>
        };

        result_type ascent_res { ascend (loop) };
        if (! ascent_res.needs_join ())
        {
          ir_def_resolution_stack& ascent_stack = ascent_res.get_stack ();
          assert (ascent_stack.num_leaves () == 1);
          assert (ascent_stack.has_frames ());
          assert (&ascent_stack.top ().get_join_block () == &loop.get_condition ());
          if (! needs_join)
          {
            assert (ascent_stack.num_frames () == 2);
            // Reacquire the origin substack and return it.
            ir_def_resolution_stack& sub_stack = ascent_stack.leaves_front ();
            return { std::move (sub_stack), sub_needs_join, sub_is_resolvable };
          }
        }

        return ascent_res;
      }
      case id::start:
        return ascend (loop);
#ifndef __clang__
      default:
        abort<reason::impossible> ();
#endif
    }
  }

  auto
  ir_ascending_def_resolution_builder::
  visit (const ir_component_sequence& seq)
    -> result_type
  {
    const ir_component_sequence::criter rpos { seq.find (get_subcomponent ()) };

    ir_def_resolution_stack& stack = m_sub_result.get_stack ();
    for (auto rit = rpos; rit != seq.rend (); ++rit)
    {
      result_type res { dispatch_descender (*rit) };
      if (res.needs_join ())
        stack.push_frame (get_entry_block (*std::prev (rit)), res.release_stack ());
      m_sub_result |= res.needs_join ();
      m_sub_result |= res.is_resolvable ();

      if (res.is_resolvable ())
        return std::move (m_sub_result);
    }

    return ascend (seq);
  }

  auto
  ir_ascending_def_resolution_builder::
  visit (const ir_function&)
    -> result_type
  {
    return std::move (m_sub_result);
  }

  auto
  ir_ascending_def_resolution_builder::
  maybe_ascend (const ir_substructure& sub,
                ir_def_resolution_stack&& stack,
                result_type::join_type needs_join,
                result_type::resolvable_type is_resolvable)
    -> result_type
  {
    // FIXME: Remove this function.
    if (needs_join)
    {
      if (is_resolvable)
      {
        stack.add_leaf (std::move (m_sub_result).release_stack ());
        return { std::move (stack), result_type::join<true>, result_type::resolvable<true> };
      }
      else
      {
        m_sub_result.get_stack ().dominate_with (get_entry_block (sub), std::move (stack));
        m_sub_result.set_join (true);
        m_sub_result.set_resolvable (false);
      }
    }

    assert (! is_resolvable);
    return ascend (sub);
  }

  auto
  ir_ascending_def_resolution_builder::
  maybe_ascend (const ir_substructure& sub, result_type&& res)
    -> result_type
  {
    return maybe_ascend (sub, res.release_stack (), res.needs_join (), res.is_resolvable ());
  }

  auto
  ir_ascending_def_resolution_builder::
  ascend (const ir_substructure& sub)
    -> result_type
  {
    return ir_ascending_def_resolution_builder { sub, std::move (m_sub_result) } ();
  }

  ir_def_resolution_build_result
  ir_ascending_def_resolution_builder::
  dispatch_descender (const ir_subcomponent& c)
  {
    return ir_descending_def_resolution_builder { get_variable () } (c);
  }

  ir_def_resolution_build_result
  ir_ascending_def_resolution_builder::
  dispatch_descender (const ir_block& block)
  {
    return ir_descending_def_resolution_builder { get_variable () } (block);
  }

  auto
  ir_ascending_def_resolution_builder::
  create_dominating_result (result_type&& res)
    -> result_type&&
  {
    res.get_stack ().add_leaf (m_sub_result.release_stack ());
    return std::move (res);
  }

  const ir_variable&
  ir_ascending_def_resolution_builder::
  get_variable (void) const noexcept
  {
    return m_sub_result.get_variable ();
  }

}

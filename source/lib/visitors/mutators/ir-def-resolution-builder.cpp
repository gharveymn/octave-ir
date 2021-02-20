/** ir-def-resolution-builder.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/mutators/ir-def-resolution-builder.hpp"

#include "components/ir-block.hpp"
#include "components/ir-component.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"

#include <numeric>
#include <vector>

namespace gch
{

  ir_def_resolution_build_result::
  ir_def_resolution_build_result (ir_variable& var, join j, resolvable r)
    : m_stack      (var),
      m_join       (j),
      m_resolvable (r)
  { }

  ir_def_resolution_build_result::
  ir_def_resolution_build_result (ir_def_resolution_stack&& s, join j, resolvable r)
    : m_stack      (std::move (s)),
      m_join       (j),
      m_resolvable (r)
  { }

  auto
  ir_def_resolution_build_result::
  join_state (void) const noexcept
  -> join
  {
    return m_join;
  }

  auto
  ir_def_resolution_build_result::
  resolvable_state (void) const noexcept
  -> resolvable
  {
    return m_resolvable;
  }

  ir_def_resolution_stack&&
  ir_def_resolution_build_result::
  release_stack (void) noexcept
  {
    return std::move (m_stack);
  }

  bool
  ir_def_resolution_build_result::
  needs_join (void) const noexcept
  {
    return join_state () == join::yes;
  }

  bool
  ir_def_resolution_build_result::
  is_resolvable (void) const noexcept
  {
    return resolvable_state () == resolvable::yes;
  }

  //
  // ir_def_resolution_build_descender
  //

  ir_def_resolution_build_descender::
  ir_def_resolution_build_descender (ir_variable& var)
    : m_variable (var)
  { }

  auto
  ir_def_resolution_build_descender::
  operator() (ir_component& c) const &&
    -> result_type
  {
    return c.accept (*this);
  }

  auto
  ir_def_resolution_build_descender::
  operator() (ir_block& block) const &&
    -> result_type
  {
    return visit (block);
  }

  auto
  ir_def_resolution_build_descender::
  visit (ir_block& block) const
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
      else
        block.remove_def_timeline (get_variable ());
    }

    return { { get_variable (), block },
             result_type::join::no,
             result_type::resolvable::no };
  }

  auto
  ir_def_resolution_build_descender::
  visit (ir_component_fork& fork) const
    -> result_type
  {
    small_vector<result_type> case_results;
    std::transform (fork.cases_begin (), fork.cases_end (), std::back_inserter (case_results),
                    [this](ir_subcomponent& sub) { return dispatch_descender (sub); });

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
  ir_def_resolution_build_descender::
  visit (ir_component_loop& loop) const
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

    return { std::move (stack),
             static_cast<result_type::join>       (needs_join),
             static_cast<result_type::resolvable> (is_resolvable) };
  }

  auto
  ir_def_resolution_build_descender::
  visit (ir_component_sequence& seq) const
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
  ir_def_resolution_build_descender::
  visit (ir_function& func) const
    -> result_type
  {
    return dispatch_descender (func.get_body ());
  }

  auto
  ir_def_resolution_build_descender::
  dispatch_descender (ir_subcomponent& sub) const
    -> result_type
  {
    return sub.accept (*this);
  }

  auto
  ir_def_resolution_build_descender::
  dispatch_descender (ir_block& block) const
    -> result_type
  {
    return visit (block);
  }

  ir_variable&
  ir_def_resolution_build_descender::
  get_variable () const noexcept
  {
    return m_variable;
  }

  //
  // ir_def_resolution_build_ascender
  //

  ir_def_resolution_build_ascender::
  ir_def_resolution_build_ascender (ir_subcomponent& sub, ir_variable& var)
    : ir_parent_mutator (sub),
      m_variable        (var)
  { }

  auto
  ir_def_resolution_build_ascender::
  operator() (void) const
    -> result_type
  {
    return get_parent ().accept (*this);
  }

  auto
  ir_def_resolution_build_ascender::
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
  ir_def_resolution_build_ascender::
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
    }

    return maybe_ascend (loop, { std::move (stack),
                                 static_cast<result_type::join>       (needs_join),
                                 static_cast<result_type::resolvable> (is_resolvable) });
  }

  auto
  ir_def_resolution_build_ascender::
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
  ir_def_resolution_build_ascender::
  visit (ir_function& func) const
    -> result_type
  {
    return { { get_variable () },
             result_type::join::no,
             result_type::resolvable::no };
  }

  auto
  ir_def_resolution_build_ascender::
  maybe_ascend (ir_substructure& sub, result_type&& sub_result) const
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
             ascent_res.resolvable_state () };
  }

  auto
  ir_def_resolution_build_ascender::
  ascend (ir_substructure& sub) const
    -> result_type
  {
    return ir_def_resolution_build_ascender { sub, get_variable () } ();
  }

  auto
  ir_def_resolution_build_ascender::
  dispatch_descender (ir_subcomponent& c) const
    -> result_type
  {
    return ir_def_resolution_build_descender { get_variable () } (c);
  }

  auto
  ir_def_resolution_build_ascender::
  dispatch_descender (ir_block& block) const
    -> result_type
  {
    return ir_def_resolution_build_descender { get_variable () } (block);
  }

  ir_variable&
  ir_def_resolution_build_ascender::
  get_variable (void) const noexcept
  {
    return m_variable;
  }

  ir_def_resolution_build_result
  build_def_resolution_stack (ir_block& block, ir_variable& var)
  {
    using result_type = ir_def_resolution_build_result;

    result_type             ret   { ir_def_resolution_build_ascender { block, var } () };
    ir_def_resolution_stack stack { ret.release_stack () };

    assert (! stack.has_leaves ());
    stack.add_leaf (block);

    return { std::move (stack), ret.join_state (), ret.resolvable_state () };
  }

}

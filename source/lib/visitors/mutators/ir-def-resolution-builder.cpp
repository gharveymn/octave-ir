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

  //
  // ir_def_resolution_build_descender
  //

  ir_def_resolution_build_descender::
  ir_def_resolution_build_descender (ir_component& c, ir_variable& v)
    : m_component     (c),
      m_stack         (v),
      m_is_resolvable (false)
  { }

  auto
  ir_def_resolution_build_descender::
  operator() (void) &&
    -> result_type
  {
    m_stack.get_component ().accept (*this);
    return { std::move (m_stack), m_is_resolvable };
  }

  void
  ir_def_resolution_build_descender::
  visit (ir_component_fork& fork)
  {
    m_is_resolvable = std::accumulate (fork.cases_begin (), fork.cases_end (), true,
                                       [this](bool curr, ir_subcomponent& sub)
                                       {
                                         result_type res { dispatch_descender (sub) };
                                         m_stack.add_leaf (std::move (res.stack));
                                         return curr && res.is_resolvable;
                                       });
    if (! m_is_resolvable)
      check_block (fork.get_condition ());
  }

  void
  ir_def_resolution_build_descender::
  visit (ir_component_loop& loop)
  {
    if (! check_block (loop.get_condition ()))
    {
      // set leaf block to the condition block
      m_stack.add_leaf (loop.get_condition (), get_variable ());
      m_stack.push_frame (loop.get_condition ())
    }

    set_state (dispatch_child (loop.get_condition ()));
    if (! is_stopped ())
    {
      if (! dispatch_child (loop.get_update ()))
        dispatch_child (loop.get_body ());
      set_state (dispatch_child (loop.get_start ()));
    }
  }

  void
  ir_def_resolution_build_descender::
  visit (ir_component_sequence& seq)
  {
    set_state (std::any_of (seq.rbegin (), seq.rend (),
                            [&](ir_component& sub) { return dispatch_child (sub); }));
  }

  void
  ir_def_resolution_build_descender::
  visit (const ir_function& func)
  {
    dispatch_child (func.get_body ());
    set_state (ir_traverser_state::stop);
  }

  ir_variable&
  ir_def_resolution_build_descender::
  get_variable () const noexcept
  {
    return m_stack.get_variable ();
  }

  auto
  ir_def_resolution_build_descender::
  dispatch_descender (ir_subcomponent& sub)
    -> result_type
  {
    return ir_def_resolution_build_descender { sub } ();
  }



  bool
  ir_def_resolution_build_descender::
  check_block (ir_block& block)
  {
    if (optional_ref dt { block.maybe_get_def_timeline (get_variable ()) })
    {
      if (dt->has_outgoing_timeline ())
      {
        m_stack.set_block_resolution (*dt);
        m_is_resolvable = true;
      }
      else
        block.remove_def_timeline (get_variable ());
    }
    return m_is_resolvable;
  }

  //
  // ir_def_resolution_builder
  //

  ir_def_resolution_builder::
  ir_def_resolution_builder (ir_component& c)
    : m_stack (c)
  { }

  ir_def_resolution_stack
  ir_def_resolution_builder::
  operator() (void) &&
  {
    m_stack.get_component ().accept (*this);
    return std::move (m_stack);
  }

  ir_def_resolution_stack
  ir_def_resolution_builder::
  dispatch_descender (ir_component& c)
  {
    return ir_def_resolution_build_descender { c } ();
  }

  void
  ir_def_resolution_builder::
  ascend (void)
  {
    optional_ref sub { maybe_cast<ir_subcomponent> (m_stack.get_component ()) };
    assert (sub && "Invalid state.");

    ir_def_resolution_stack parent_stack { sub->get_parent () };
    parent_stack.add_leaf (std::move (m_stack));
    m_stack = std::move (parent_stack);
  }

}

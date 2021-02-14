/** ir_stack_builder.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_STACK_BUILDER_HPP
#define OCTAVE_IR_IR_STACK_BUILDER_HPP

#include "ir-structure-fwd.hpp"
#include "ir-def-resolution.hpp"
#include "ir-traverser.hpp"

namespace gch
{

  class ir_def_resolution_stack_builder
  {
  public:
    explicit
    ir_def_resolution_stack_builder (ir_block& leaf_block)
      : m_current_join_block (leaf_block),
        m_stack (leaf_block)
    { }

    ir_def_resolution_stack_builder&
    operator() (ir_component& c)
    {
      if (! ir_substructure_ascender { *this } (c))
        dispatch_parent (c);
      return *this;
    }

    bool
    visit (ir_block&)
    {
      return false;
    }

    // let B = block
    // 1. pass B (is set as stack leaf)
    // 2. visit B
    // 3. if continue then push to stack with the B as the join-block
    // 4. ascend to parent
    // 5. visit preds (if we have a fork then we recurse,
    //                 creating a new builder and storing
    //                 the result in the current frame.
    // 6. if continue go to 4

    bool
    dispatch_child (ir_component& c)
    {
      if (optional_ref b { maybe_cast<ir_block> (c) })
      {
        ir_resolution_stack_builder sub_scope { *b };
        bool result = ir_substructure_ascender { sub_scope } (c);
      }
      else
        return ir_substructure_ascender { *this } (c);
    }

    small_vector<ir_def_resolution_stack_builder, 1>
    dispatch_fork (ir_component& c);

    void
    dispatch_parent (ir_component& c)
    {
      m_stack.push (get_entry_block (c));
      ir_superstructure_ascender { *this, c } (*c.maybe_get_parent ());
    }

  private:
    ir_def_resolution_stack m_stack;
  };



}

#endif // OCTAVE_IR_IR_STACK_BUILDER_HPP

/** ir-def-resolution-builder.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_DEF_RESOLUTION_BUILDER_HPP
#define OCTAVE_IR_IR_DEF_RESOLUTION_BUILDER_HPP

#include "components/ir-component-fwd.hpp"
#include "processors/ir-def-resolution.hpp"

namespace gch
{

  class ir_variable;

  class ir_def_resolution_build_descender
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    struct result_type
    {
      ir_def_resolution_stack stack;
      bool                    is_resolvable;
    };

    ir_def_resolution_build_descender (ir_component& c, ir_variable& v);

    result_type
    operator() (void) &&;

  private:
    void
    visit (ir_block& block);

    void
    visit (ir_component_fork& fork);

    void
    visit (ir_component_loop& loop);

    void
    visit (ir_component_sequence& seq);

    void
    visit (ir_function& func);

    [[nodiscard]]
    ir_variable&
    get_variable (void) const noexcept;

    static
    result_type
    dispatch_descender (ir_subcomponent& sub);

    bool
    check_block (ir_block& block);

    ir_component&           m_component;
    ir_def_resolution_stack m_stack;
    bool                    m_is_resolvable;
  };

  class ir_def_resolution_builder
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    ir_def_resolution_builder (ir_component& c);

    ir_def_resolution_stack
    operator() (void) &&;

  private:
    void
    visit (const ir_block& block);

    void
    visit (const ir_component_fork& fork);

    void
    visit (const ir_component_loop& loop);

    void
    visit (const ir_component_sequence& seq);

    void
    visit (const ir_function& func);

    void
    ascend (void);

    static
    ir_def_resolution_build_descender::result_type
    dispatch_descender (ir_component& c);

    ir_def_resolution_stack m_stack;
  };

  // if returns nullopt then no resolution in descent
  // else if not resolvable we need to continue
  // else resolvable we collapse
  std::optional<ir_def_resolution_stack>
  build_resolution (ir_component& c);


  // class ir_def_resolution_stack_builder
  // {
  // public:
  //   explicit
  //   ir_def_resolution_stack_builder (ir_block& leaf_block)
  //     : m_current_join_block (leaf_block),
  //       m_stack (leaf_block)
  //   { }
  //
  //   ir_def_resolution_stack_builder&
  //   operator() (ir_component& c)
  //   {
  //     if (! ir_backward_descender { *this } (c))
  //       dispatch_parent (c);
  //     return *this;
  //   }
  //
  //   bool
  //   visit (ir_block&)
  //   {
  //     return false;
  //   }
  //
  //   // let B = block
  //   // 1. pass B (is set as stack leaf)
  //   // 2. visit B
  //   // 3. if continue then push to stack with the B as the join-block
  //   // 4. ascend to parent
  //   // 5. visit preds (if we have a fork then we recurse,
  //   //                 creating a new builder and storing
  //   //                 the result in the current frame.
  //   // 6. if continue go to 4
  //
  //   bool
  //   dispatch_child (ir_component& c)
  //   {
  //     if (optional_ref b { maybe_cast<ir_block> (c) })
  //     {
  //       ir_resolution_stack_builder sub_scope { *b };
  //       bool result = ir_backward_descender { sub_scope } (c);
  //     }
  //     else
  //       return ir_backward_descender { *this } (c);
  //   }
  //
  //   small_vector<ir_def_resolution_stack_builder, 1>
  //   dispatch_fork (ir_component& c);
  //
  //   void
  //   dispatch_parent (ir_component& c)
  //   {
  //     m_stack.push_frame (get_entry_block (c));
  //     ir_backward_ascender { *this, c } (*c.maybe_get_parent ());
  //   }
  //
  // private:
  //   ir_def_resolution_stack m_stack;
  // };

}

#endif // OCTAVE_IR_IR_DEF_RESOLUTION_BUILDER_HPP

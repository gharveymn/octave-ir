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
#include "visitors/mutators/ir-subcomponent-mutator.hpp"

namespace gch
{

  class ir_variable;

  class ir_def_resolution_build_result
  {
  public:
    ir_def_resolution_build_result            (void)                                      = delete;
    ir_def_resolution_build_result            (const ir_def_resolution_build_result&)     = delete;
    ir_def_resolution_build_result            (ir_def_resolution_build_result&&) noexcept = default;
    ir_def_resolution_build_result& operator= (const ir_def_resolution_build_result&)     = delete;
    ir_def_resolution_build_result& operator= (ir_def_resolution_build_result&&) noexcept = default;
    ~ir_def_resolution_build_result           (void)                                      = default;

    enum class join : bool
    {
      yes = true,
      no  = false,
    };

    enum class resolvable : bool
    {
      yes = true,
      no  = false,
    };

    ir_def_resolution_build_result (ir_variable& var, join j, resolvable r);

    ir_def_resolution_build_result (ir_def_resolution_stack&& s, join j, resolvable r);

    [[nodiscard]]
    join
    get_join_state (void) const noexcept;

    [[nodiscard]]
    resolvable
    get_resolvable_state (void) const noexcept;

    // returning rvalue reference just for semantic reasons
    [[nodiscard]]
    ir_def_resolution_stack&&
    release_stack (void) noexcept;

    [[nodiscard]]
    bool
    needs_join (void) const noexcept;

    [[nodiscard]]
    bool
    is_resolvable (void) const noexcept;

  private:
    ir_def_resolution_stack m_stack;
    join                    m_join;
    resolvable              m_resolvable;
  };

  class ir_descending_def_resolution_builder
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    using result_type = ir_def_resolution_build_result;

    explicit
    ir_descending_def_resolution_builder (ir_variable& var);

    [[nodiscard]]
    result_type
    operator() (ir_component& c) const &&;

    [[nodiscard]]
    result_type
    operator() (ir_block& block) const &&;

  private:
    [[nodiscard]]
    result_type
    visit (ir_block& block) const;

    [[nodiscard]]
    result_type
    visit (ir_component_fork& fork) const;

    [[nodiscard]]
    result_type
    visit (ir_component_loop& loop) const;

    [[nodiscard]]
    result_type
    visit (ir_component_sequence& seq) const;

    [[nodiscard]]
    result_type
    visit (ir_function& func) const;

    [[nodiscard]]
    result_type
    dispatch_descender (ir_subcomponent& sub) const;

    [[nodiscard]]
    result_type
    dispatch_descender (ir_block& block) const;

    [[nodiscard]]
    ir_variable&
    get_variable (void) const noexcept;

    ir_variable& m_variable;
  };

  class ir_ascending_def_resolution_builder
    : protected ir_subcomponent_mutator
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    using result_type    = ir_def_resolution_build_result;
    using descender_type = ir_descending_def_resolution_builder;

    explicit
    ir_ascending_def_resolution_builder (ir_subcomponent& sub, ir_variable& var);

    result_type
    operator() (void) const;

  private:
    [[nodiscard]]
    result_type
    visit (ir_component_fork& fork) const;

    [[nodiscard]]
    result_type
    visit (ir_component_loop& loop) const;

    [[nodiscard]]
    result_type
    visit (ir_component_sequence& seq) const;

    [[nodiscard]]
    result_type
    visit (ir_function& func) const;

    [[nodiscard]]
    result_type
    maybe_ascend (ir_substructure& sub, result_type&& sub_result) const;

    [[nodiscard]]
    result_type
    ascend (ir_substructure& sub) const;

    [[nodiscard]]
    descender_type::result_type
    dispatch_descender (ir_subcomponent& sub) const;

    [[nodiscard]]
    descender_type::result_type
    dispatch_descender (ir_block& block) const;

    [[nodiscard]]
    ir_variable&
    get_variable (void) const noexcept;

    ir_variable& m_variable;
  };

  [[nodiscard]]
  ir_def_resolution_build_result
  build_def_resolution_stack (ir_block& block, ir_variable& var);

}

#endif // OCTAVE_IR_IR_DEF_RESOLUTION_BUILDER_HPP

/** ir-def-resolution-builder.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_DEF_RESOLUTION_BUILDER_HPP
#define OCTAVE_IR_IR_DEF_RESOLUTION_BUILDER_HPP

#include "ir-structure-mutators-fwd.hpp"

#include "visitors/component/mutators/ir-component-mutators-fwd.hpp"

#include "processors/ir-def-resolution.hpp"
#include "visitors/structure/mutators/utility/ir-subcomponent-mutator.hpp"

namespace gch
{

  class ir_descending_def_resolution_builder
    : public visitor_traits<ir_descending_def_resolution_builder>
  {
  public:
    friend acceptor_type<ir_block>;
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

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
    : public    visitor_traits<ir_ascending_def_resolution_builder>,
      protected ir_subcomponent_mutator
  {
  public:
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

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
    maybe_ascend (ir_substructure& sub, descender_type::result_type&& sub_result) const;

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

}

#endif // OCTAVE_IR_IR_DEF_RESOLUTION_BUILDER_HPP

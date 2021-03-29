/** ir-descending-def-resolution-builder.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_DESCENDING_DEF_RESOLUTION_BUILDER_HPP
#define OCTAVE_IR_IR_DESCENDING_DEF_RESOLUTION_BUILDER_HPP

#include "ir-component-mutators-fwd.hpp"

#include "visitors/ir-visitor.hpp"

namespace gch
{

  class ir_def_resolution_build_result;
  class ir_variable;

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

}

#endif // OCTAVE_IR_IR_DESCENDING_DEF_RESOLUTION_BUILDER_HPP

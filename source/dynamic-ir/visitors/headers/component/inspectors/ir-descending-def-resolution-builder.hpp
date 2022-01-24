/** ir-descending-def-resolution-builder.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_DESCENDING_DEF_RESOLUTION_BUILDER_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_DESCENDING_DEF_RESOLUTION_BUILDER_HPP

#include "component/inspectors/ir-component-inspectors-fwd.hpp"

#include "ir-visitor.hpp"

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
    ir_descending_def_resolution_builder (const ir_variable& var);

    [[nodiscard]]
    result_type
    operator() (const ir_component& c) const &&;

    [[nodiscard]]
    result_type
    operator() (const ir_block& block) const &&;

  private:
    [[nodiscard]]
    result_type
    visit (const ir_block& block) const;

    [[nodiscard]]
    result_type
    visit (const ir_component_fork& fork) const;

    [[nodiscard]]
    result_type
    visit (const ir_component_loop& loop) const;

    [[nodiscard]]
    result_type
    visit (const ir_component_sequence& seq) const;

    [[nodiscard]]
    result_type
    visit (const ir_function& func) const;

    [[nodiscard]]
    result_type
    dispatch_descender (const ir_subcomponent& sub) const;

    [[nodiscard]]
    result_type
    dispatch_descender (const ir_block& block) const;

    [[nodiscard]]
    const ir_variable&
    get_variable (void) const noexcept;

    const ir_variable& m_variable;
  };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_DESCENDING_DEF_RESOLUTION_BUILDER_HPP

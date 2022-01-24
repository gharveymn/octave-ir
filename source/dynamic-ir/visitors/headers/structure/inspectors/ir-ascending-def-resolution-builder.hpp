/** ir-def-resolution-builder.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_ASCENDING_DEF_RESOLUTION_BUILDER_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_ASCENDING_DEF_RESOLUTION_BUILDER_HPP

#include "structure/inspectors/ir-structure-inspectors-fwd.hpp"
#include "structure/inspectors/utility/ir-subcomponent-inspector.hpp"

namespace gch
{

  class ir_def_resolution_build_result;
  class ir_variable;

  class ir_ascending_def_resolution_builder
    : public    visitor_traits<ir_ascending_def_resolution_builder>,
      protected ir_subcomponent_inspector
  {
  public:
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

    using result_type = ir_def_resolution_build_result;

    explicit
    ir_ascending_def_resolution_builder (const ir_subcomponent& sub, const ir_variable& var);

    result_type
    operator() (void) const;

  private:
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
    maybe_ascend (const ir_substructure& sub, ir_def_resolution_build_result&& sub_result) const;

    [[nodiscard]]
    result_type
    ascend (const ir_substructure& sub) const;

    [[nodiscard]]
    ir_def_resolution_build_result
    dispatch_descender (const ir_subcomponent& sub) const;

    [[nodiscard]]
    ir_def_resolution_build_result
    dispatch_descender (const ir_block& block) const;

    [[nodiscard]]
    const ir_variable&
    get_variable (void) const noexcept;

    const ir_variable& m_variable;
  };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_ASCENDING_DEF_RESOLUTION_BUILDER_HPP

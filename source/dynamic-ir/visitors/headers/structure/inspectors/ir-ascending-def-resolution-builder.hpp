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

#include "ir-def-resolution.hpp"

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

    explicit
    ir_ascending_def_resolution_builder (const ir_subcomponent& sub, result_type&& sub_result);

    result_type
    operator() (void);

  private:
    [[nodiscard]]
    result_type
    visit (const ir_component_fork& fork);

    [[nodiscard]]
    result_type
    visit (const ir_component_loop& loop);

    [[nodiscard]]
    result_type
    visit (const ir_component_sequence& seq);

    [[nodiscard]]
    result_type
    visit (const ir_function& func);

    [[nodiscard]]
    result_type
    maybe_ascend (const ir_substructure& sub,
                  ir_def_resolution_stack&& stack,
                  result_type::join_type needs_join,
                  result_type::resolvable_type is_resolvable);

    [[nodiscard]]
    result_type
    maybe_ascend (const ir_substructure& sub, result_type&& res);

    [[nodiscard]]
    result_type
    ascend (const ir_substructure& sub);

    [[nodiscard]]
    ir_def_resolution_build_result
    dispatch_descender (const ir_subcomponent& sub);

    [[nodiscard]]
    ir_def_resolution_build_result
    dispatch_descender (const ir_block& block);

    [[nodiscard]]
    result_type&&
    create_dominating_result (result_type&& res);

    [[nodiscard]]
    const ir_variable&
    get_variable (void) const noexcept;

    result_type m_sub_result;
  };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_ASCENDING_DEF_RESOLUTION_BUILDER_HPP

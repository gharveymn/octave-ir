/** ir-leaf-inspector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_LEAF_INSPECTOR_HPP
#define OCTAVE_IR_IR_LEAF_INSPECTOR_HPP

#include "ir-structure-inspectors-fwd.hpp"

#include "utilities/ir-common.hpp"
#include "visitors/structure/inspectors/utility/ir-subcomponent-inspector.hpp"

namespace gch
{

  class ir_leaf_inspector
    : public    visitor_traits<ir_leaf_inspector>,
      protected ir_subcomponent_inspector
  {
  public:
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

    using result_type  = bool;

    ir_leaf_inspector            (void)                         = delete;
    ir_leaf_inspector            (const ir_leaf_inspector&)     = delete;
    ir_leaf_inspector            (ir_leaf_inspector&&) noexcept = delete;
    ir_leaf_inspector& operator= (const ir_leaf_inspector&)     = delete;
    ir_leaf_inspector& operator= (ir_leaf_inspector&&) noexcept = delete;
    ~ir_leaf_inspector           (void)                         = default;

    using ir_subcomponent_inspector::ir_subcomponent_inspector;

    [[nodiscard]]
    result_type
    operator() (void) const noexcept;

  private:
    [[nodiscard]]
    result_type
    visit (const ir_component_fork& fork) const noexcept;

    [[nodiscard]]
    result_type
    visit (const ir_component_loop& loop) const noexcept;

    [[nodiscard]]
    result_type
    visit (const ir_component_sequence& seq) const noexcept;

    [[nodiscard]]
    static GCH_CPP20_CONSTEVAL
    result_type
    visit (const ir_function& func) noexcept { return true; };
  };

}

#endif // OCTAVE_IR_IR_LEAF_INSPECTOR_HPP

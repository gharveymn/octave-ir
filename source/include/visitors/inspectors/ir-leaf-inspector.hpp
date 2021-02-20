/** ir-leaf-inspector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_LEAF_INSPECTOR_HPP
#define OCTAVE_IR_IR_LEAF_INSPECTOR_HPP

#include "visitors/inspectors/ir-parent-inspector.hpp"

#include "components/ir-component-fwd.hpp"
#include "utilities/ir-link-set.hpp"

namespace gch
{

  class ir_leaf_inspector
    : public ir_parent_inspector
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    using result_type = bool;

    ir_leaf_inspector            (void)                         = delete;
    ir_leaf_inspector            (const ir_leaf_inspector&)     = delete;
    ir_leaf_inspector            (ir_leaf_inspector&&) noexcept = delete;
    ir_leaf_inspector& operator= (const ir_leaf_inspector&)     = delete;
    ir_leaf_inspector& operator= (ir_leaf_inspector&&) noexcept = delete;
    ~ir_leaf_inspector           (void)                         = default;

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

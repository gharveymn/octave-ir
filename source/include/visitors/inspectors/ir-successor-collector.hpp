/** ir-successor-collector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_SUCCESSOR_COLLECTOR_HPP
#define OCTAVE_IR_IR_SUCCESSOR_COLLECTOR_HPP

#include "visitors/inspectors/ir-parent-inspector.hpp"

#include "components/ir-component-fwd.hpp"
#include "utilities/ir-link-set.hpp"

namespace gch
{

  class ir_successor_collector
    : public ir_parent_inspector
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    using result_type = ir_link_set<ir_block>;

    ir_successor_collector            (void)                              = delete;
    ir_successor_collector            (const ir_successor_collector&)     = delete;
    ir_successor_collector            (ir_successor_collector&&) noexcept = delete;
    ir_successor_collector& operator= (const ir_successor_collector&)     = delete;
    ir_successor_collector& operator= (ir_successor_collector&&) noexcept = delete;
    ~ir_successor_collector           (void)                              = default;

    [[nodiscard]]
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
    static
    result_type
    visit (const ir_function&) { return { }; };
  };

}

#endif // OCTAVE_IR_IR_SUCCESSOR_COLLECTOR_HPP

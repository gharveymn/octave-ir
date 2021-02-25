/** ir-predecessor-collector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_PREDECESSOR_COLLECTOR_HPP
#define OCTAVE_IR_IR_PREDECESSOR_COLLECTOR_HPP

#include "visitors/inspectors/ir-subcomponent-inspector.hpp"

#include "components/ir-component-fwd.hpp"
#include "visitors/ir-visitor-fwd.hpp"

namespace gch
{

  class ir_predecessor_collector
    : public    visitor_traits<ir_predecessor_collector>,
      protected ir_subcomponent_inspector
  {
  public:
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

    using result_type = ir_link_set<ir_block>;

    ir_predecessor_collector            (void)                                = delete;
    ir_predecessor_collector            (const ir_predecessor_collector&)     = delete;
    ir_predecessor_collector            (ir_predecessor_collector&&) noexcept = delete;
    ir_predecessor_collector& operator= (const ir_predecessor_collector&)     = delete;
    ir_predecessor_collector& operator= (ir_predecessor_collector&&) noexcept = delete;
    ~ir_predecessor_collector           (void)                                = default;

    using ir_subcomponent_inspector::ir_subcomponent_inspector;

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
    visit (const ir_function&);
  };

}

#endif // OCTAVE_IR_IR_PREDECESSOR_COLLECTOR_HPP

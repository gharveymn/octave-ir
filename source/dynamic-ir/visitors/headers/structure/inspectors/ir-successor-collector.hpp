/** ir-successor-collector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_SUCCESSOR_COLLECTOR_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_SUCCESSOR_COLLECTOR_HPP

#include "ir-structure-inspectors-fwd.hpp"
#include "structure/inspectors/utility/ir-subcomponent-inspector.hpp"

#include "ir-visitor.hpp"
#include <gch/small_vector.hpp>

namespace gch
{

  class ir_successor_collector
    : public    visitor_traits<ir_successor_collector>,
      protected ir_subcomponent_inspector
  {
  public:
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

    using result_type = small_vector<nonnull_ptr<ir_block>>;

    ir_successor_collector            (void)                              = delete;
    ir_successor_collector            (const ir_successor_collector&)     = delete;
    ir_successor_collector            (ir_successor_collector&&) noexcept = delete;
    ir_successor_collector& operator= (const ir_successor_collector&)     = delete;
    ir_successor_collector& operator= (ir_successor_collector&&) noexcept = delete;
    ~ir_successor_collector           (void)                              = default;

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
    visit (const ir_function&) { return { }; }
  };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_SUCCESSOR_COLLECTOR_HPP

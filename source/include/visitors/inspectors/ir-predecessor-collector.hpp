/** ir-predecessor-collector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_PREDECESSOR_COLLECTOR_HPP
#define OCTAVE_IR_IR_PREDECESSOR_COLLECTOR_HPP

#include "visitors/inspectors/ir-parent-inspector.hpp"

#include "utilities/ir-link-set.hpp"

namespace gch
{

  class ir_predecessor_collector
    : public ir_parent_inspector<ir_predecessor_collector, ir_link_set<ir_block>>
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    ir_predecessor_collector            (void)                                = default;
    ir_predecessor_collector            (const ir_predecessor_collector&)     = delete;
    ir_predecessor_collector            (ir_predecessor_collector&&) noexcept = delete;
    ir_predecessor_collector& operator= (const ir_predecessor_collector&)     = delete;
    ir_predecessor_collector& operator= (ir_predecessor_collector&&) noexcept = delete;
    ~ir_predecessor_collector           (void)                                = default;

  private:
    void
    visit (const ir_component_fork& fork);

    void
    visit (const ir_component_loop& loop);

    void
    visit (const ir_component_sequence& seq);

    void
    visit (const ir_function& func);
  };

}

#endif // OCTAVE_IR_IR_PREDECESSOR_COLLECTOR_HPP

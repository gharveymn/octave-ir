/** ir-component-inspector.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_LEAF_COLLECTOR_HPP
#define OCTAVE_IR_IR_LEAF_COLLECTOR_HPP

#include "components/ir-component-fwd.hpp"
#include "utilities/ir-link-set.hpp"

namespace gch
{

  class ir_leaf_collector
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    ir_leaf_collector            (void)                         = default;
    ir_leaf_collector            (const ir_leaf_collector&)     = delete;
    ir_leaf_collector            (ir_leaf_collector&&) noexcept = delete;
    ir_leaf_collector& operator= (const ir_leaf_collector&)     = delete;
    ir_leaf_collector& operator= (ir_leaf_collector&&) noexcept = delete;
    ~ir_leaf_collector           (void)                         = default;

    ir_link_set<ir_block>
    operator() (const ir_structure& s);

  private:
    void
    visit (const ir_block& block);

    void
    visit (const ir_component_fork& fork);

    void
    visit (const ir_component_loop& loop);

    void
    visit (const ir_component_sequence& seq);

    void
    visit (const ir_function& func);

    void
    append (const ir_subcomponent& sub);

    ir_link_set<ir_block> m_result;
  };

  ir_link_set<ir_block>
  collect_leaves (const ir_structure& s);

}

#endif // OCTAVE_IR_IR_LEAF_COLLECTOR_HPP

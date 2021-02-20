/** ir-entry-collector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_ENTRY_COLLECTOR_HPP
#define OCTAVE_IR_IR_ENTRY_COLLECTOR_HPP

#include "components/ir-component-fwd.hpp"
#include "visitors/ir-visitor.hpp"

namespace gch
{

  class ir_entry_collector
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    using result_type = const ir_subcomponent&;

    ir_entry_collector            (void)                          = default;
    ir_entry_collector            (const ir_entry_collector&)     = delete;
    ir_entry_collector            (ir_entry_collector&&) noexcept = delete;
    ir_entry_collector& operator= (const ir_entry_collector&)     = delete;
    ir_entry_collector& operator= (ir_entry_collector&&) noexcept = delete;
    ~ir_entry_collector           (void)                          = default;

    [[nodiscard]]
    result_type
    operator() (const ir_structure& s) const;

  private:
    [[nodiscard]]
    static
    result_type
    visit (const ir_component_fork& fork);

    [[nodiscard]]
    static
    result_type
    visit (const ir_component_loop& loop);

    [[nodiscard]]
    static
    result_type
    visit (const ir_component_sequence& seq);

    [[nodiscard]]
    static
    result_type
    visit (const ir_function& func);
  };

  ir_subcomponent&
  get_entry_component (ir_structure& s);

  const ir_subcomponent&
  get_entry_component (const ir_structure& s);

  ir_block&
  get_entry_block (ir_structure& s);

  const ir_block&
  get_entry_block (const ir_structure& s);

}

#endif // OCTAVE_IR_IR_ENTRY_COLLECTOR_HPP

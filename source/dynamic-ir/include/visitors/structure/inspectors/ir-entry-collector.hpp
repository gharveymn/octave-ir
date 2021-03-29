/** ir-entry-collector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_ENTRY_COLLECTOR_HPP
#define OCTAVE_IR_IR_ENTRY_COLLECTOR_HPP

#include "ir-structure-inspectors-fwd.hpp"

#include "visitors/ir-visitor.hpp"

namespace gch
{

  template <>
  auto
  acceptor<ir_component_fork, inspector_type<ir_entry_collector>>::
  accept (visitor_reference v) const
    -> result_type;

  template <>
  auto
  acceptor<ir_component_loop, inspector_type<ir_entry_collector>>::
  accept (visitor_reference v) const
    -> result_type;

  template <>
  auto
  acceptor<ir_component_sequence, inspector_type<ir_entry_collector>>::
  accept (visitor_reference v) const
    -> result_type;

  template <>
  auto
  acceptor<ir_function, inspector_type<ir_entry_collector>>::
  accept (visitor_reference v) const
    -> result_type;

  class ir_entry_collector
    : public visitor_traits<ir_entry_collector>
  {
  public:
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

    ir_entry_collector            (void)                          = default;
    ir_entry_collector            (const ir_entry_collector&)     = default;
    ir_entry_collector            (ir_entry_collector&&) noexcept = default;
    ir_entry_collector& operator= (const ir_entry_collector&)     = default;
    ir_entry_collector& operator= (ir_entry_collector&&) noexcept = default;
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

}

#endif // OCTAVE_IR_IR_ENTRY_COLLECTOR_HPP

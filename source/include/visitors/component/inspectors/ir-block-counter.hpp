/** ir-block-counter.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_BLOCK_COUNTER_HPP
#define OCTAVE_IR_IR_BLOCK_COUNTER_HPP

#include "ir-component-inspectors-fwd.hpp"

#include "visitors/ir-visitor.hpp"

namespace gch
{

  template <>
  auto
  acceptor<ir_block, inspector_type<ir_block_counter>>::
  accept (visitor_reference v) const
    -> result_type;

  template <>
  auto
  acceptor<ir_component_fork, inspector_type<ir_block_counter>>::
  accept (visitor_reference v) const
    -> result_type;

  template <>
  auto
  acceptor<ir_component_loop, inspector_type<ir_block_counter>>::
  accept (visitor_reference v) const
    -> result_type;

  template <>
  auto
  acceptor<ir_component_sequence, inspector_type<ir_block_counter>>::
  accept (visitor_reference v) const
    -> result_type;

  template <>
  auto
  acceptor<ir_function, inspector_type<ir_block_counter>>::
  accept (visitor_reference v) const
    -> result_type;

  class ir_block_counter
    : public visitor_traits<ir_block_counter>
  {
  public:
    friend acceptor_type<ir_block>;
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

    ir_block_counter            (void)                        = default;
    ir_block_counter            (const ir_block_counter&)     = default;
    ir_block_counter            (ir_block_counter&&) noexcept = default;
    ir_block_counter& operator= (const ir_block_counter&)     = default;
    ir_block_counter& operator= (ir_block_counter&&) noexcept = default;
    ~ir_block_counter           (void)                        = default;

    [[nodiscard]]
    result_type
    operator() (const ir_component& c) const;

  private:
    [[nodiscard]]
    result_type
    visit (const ir_block& block) const;

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
    result_type
    visit (const ir_function& func) const;
  };

}

#endif // OCTAVE_IR_IR_BLOCK_COUNTER_HPP

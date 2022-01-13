/** ir-block-counter.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_BLOCK_COUNTER_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_BLOCK_COUNTER_HPP

#include "ir-component-inspectors-fwd.hpp"

#include "ir-visitor.hpp"

namespace gch
{

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
    static
    result_type
    visit (const ir_block& block);

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

    [[nodiscard]]
    static
    result_type
    subcomponent_result (const ir_subcomponent& sub);

  };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_BLOCK_COUNTER_HPP

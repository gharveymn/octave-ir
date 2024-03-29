/** ir-component-inspector.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_LEAF_COLLECTOR_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_LEAF_COLLECTOR_HPP

#include "ir-component-inspectors-fwd.hpp"

#include "ir-link-set.hpp"
#include "ir-visitor.hpp"

namespace gch
{

  class ir_leaf_collector
    : public visitor_traits<ir_leaf_collector>
  {
  public:
    friend acceptor_type<ir_block>;
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

    ir_leaf_collector            (void)                         = default;
    ir_leaf_collector            (const ir_leaf_collector&)     = default;
    ir_leaf_collector            (ir_leaf_collector&&) noexcept = default;
    ir_leaf_collector& operator= (const ir_leaf_collector&)     = default;
    ir_leaf_collector& operator= (ir_leaf_collector&&) noexcept = default;
    ~ir_leaf_collector           (void)                         = default;

    [[nodiscard]]
    result_type
    operator() (const ir_structure& s) const;

  private:
    [[nodiscard]]
    static
    result_type
    visit (const ir_block& block);

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

    [[nodiscard]]
    result_type
    subcomponent_result (const ir_subcomponent& sub) const;
  };

  class ir_const_leaf_collector
    : public visitor_traits<ir_const_leaf_collector>
  {
  public:
    friend acceptor_type<ir_block>;
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

    ir_const_leaf_collector            (void)                               = default;
    ir_const_leaf_collector            (const ir_const_leaf_collector&)     = default;
    ir_const_leaf_collector            (ir_const_leaf_collector&&) noexcept = default;
    ir_const_leaf_collector& operator= (const ir_const_leaf_collector&)     = default;
    ir_const_leaf_collector& operator= (ir_const_leaf_collector&&) noexcept = default;
    ~ir_const_leaf_collector           (void)                               = default;

    [[nodiscard]]
    result_type
    operator() (const ir_structure& s) const;

  private:
    [[nodiscard]]
    static
    result_type
    visit (const ir_block& block);

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

    [[nodiscard]]
    result_type
    subcomponent_result (const ir_subcomponent& sub) const;
  };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_LEAF_COLLECTOR_HPP

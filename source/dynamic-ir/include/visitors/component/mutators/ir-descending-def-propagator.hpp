/** ir-descending-def-propagator.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_DESCENDING_DEF_PROPAGATOR_HPP
#define OCTAVE_IR_IR_DESCENDING_DEF_PROPAGATOR_HPP

#include "ir-component-mutators-fwd.hpp"

#include "gch/octave-ir-utilities/ir-link-set.hpp"
#include "visitors/ir-visitor.hpp"

namespace gch
{

  class ir_def_timeline;

  class ir_descending_def_propagator
    : visitor_traits<ir_descending_def_propagator>
  {
  public:
    friend acceptor_type<ir_block>;
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

    using result_type  = ir_link_set<ir_block>;

    ir_descending_def_propagator            (void)                                    = delete;
    ir_descending_def_propagator            (const ir_descending_def_propagator&)     = delete;
    ir_descending_def_propagator            (ir_descending_def_propagator&&) noexcept = delete;
    ir_descending_def_propagator& operator= (const ir_descending_def_propagator&)     = delete;
    ir_descending_def_propagator& operator= (ir_descending_def_propagator&&) noexcept = delete;
    ~ir_descending_def_propagator           (void)                                    = default;

    explicit
    ir_descending_def_propagator (ir_def_timeline& dominator, ir_link_set<ir_block>&& incoming);

    [[nodiscard]]
    result_type
    operator() (ir_component& c) const;

    [[nodiscard]]
    result_type
    operator() (ir_block& block) const;

  private:
    [[nodiscard]]
    result_type
    visit (ir_block& block) const;

    [[nodiscard]]
    result_type
    visit (ir_component_fork& fork) const;

    [[nodiscard]]
    result_type
    visit (ir_component_loop& loop) const;

    [[nodiscard]]
    result_type
    visit (ir_component_sequence& seq) const;

    [[nodiscard]]
    result_type
    visit (ir_function& func) const;

    [[nodiscard]]
    result_type
    dispatch_descender (ir_subcomponent& sub) const;

    [[nodiscard]]
    result_type
    dispatch_descender (ir_block& block) const;

    ir_def_timeline&              m_dominator;
    mutable ir_link_set<ir_block> m_incoming_blocks;
  };

}

#endif // OCTAVE_IR_IR_DESCENDING_DEF_PROPAGATOR_HPP

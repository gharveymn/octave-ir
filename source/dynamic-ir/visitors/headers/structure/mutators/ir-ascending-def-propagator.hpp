/** ir-def-propagator.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_ASCENDING_DEF_PROPAGATOR_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_ASCENDING_DEF_PROPAGATOR_HPP

#include "ir-structure-mutators-fwd.hpp"
#include "structure/mutators/utility/ir-subcomponent-mutator.hpp"

#include "ir-link-set.hpp"
#include "ir-visitor.hpp"

namespace gch
{

  class ir_def_timeline;

  class ir_ascending_def_propagator
    : public    visitor_traits<ir_ascending_def_propagator>,
      protected ir_subcomponent_mutator
  {
  public:
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

    using result_type = void;

    ir_ascending_def_propagator            (void)                                   = delete;
    ir_ascending_def_propagator            (const ir_ascending_def_propagator&)     = delete;
    ir_ascending_def_propagator            (ir_ascending_def_propagator&&) noexcept = delete;
    ir_ascending_def_propagator& operator= (const ir_ascending_def_propagator&)     = delete;
    ir_ascending_def_propagator& operator= (ir_ascending_def_propagator&&) noexcept = delete;
    ~ir_ascending_def_propagator           (void)                                   = default;

    explicit
    ir_ascending_def_propagator (ir_subcomponent& sub,
                                 ir_def_timeline& dominator,
                                 ir_link_set<ir_block>&& incoming);

    result_type
    operator() (void) const;

  private:
    result_type
    visit (ir_component_fork& fork) const;

    result_type
    visit (ir_component_loop& loop) const;

    result_type
    visit (ir_component_sequence& seq) const;

    static constexpr
    result_type
    visit (ir_function&) noexcept { }

    result_type
    ascend (ir_substructure& sub, ir_link_set<ir_block>&& sub_result) const;

    ir_link_set<ir_block>
    dispatch_descender (ir_subcomponent& sub) const;

    ir_link_set<ir_block>
    dispatch_descender (ir_block& block) const;

    static
    bool
    needs_stop (const ir_link_set<ir_block>& res) noexcept;

    ir_def_timeline&              m_dominator;
    mutable ir_link_set<ir_block> m_incoming_block_cache;
  };

  void
  propagate_def (ir_def_timeline& dt);

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_ASCENDING_DEF_PROPAGATOR_HPP

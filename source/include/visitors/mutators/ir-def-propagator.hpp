/** ir-def-propagator.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_DEF_PROPAGATOR_HPP
#define OCTAVE_IR_IR_DEF_PROPAGATOR_HPP

#include "components/ir-component-fwd.hpp"
#include "utilities/ir-link-set.hpp"
#include "visitors/mutators/ir-subcomponent-mutator.hpp"

namespace gch
{

  class ir_def_timeline;

  class ir_descending_def_propagator
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    using result_type  = ir_link_set<ir_block>;

    ir_descending_def_propagator            (void)                                    = delete;
    ir_descending_def_propagator            (const ir_descending_def_propagator&)     = delete;
    ir_descending_def_propagator            (ir_descending_def_propagator&&) noexcept = delete;
    ir_descending_def_propagator& operator= (const ir_descending_def_propagator&)     = delete;
    ir_descending_def_propagator& operator= (ir_descending_def_propagator&&) noexcept = delete;
    ~ir_descending_def_propagator           (void)                                    = default;

    explicit
    ir_descending_def_propagator (ir_def_timeline& dominator);

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

    ir_def_timeline& m_dominator;
  };

  class ir_ascending_def_propagator
    : protected ir_subcomponent_mutator
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    using result_type    = void;
    using descender_type = ir_descending_def_propagator;

    ir_ascending_def_propagator            (void)                                   = delete;
    ir_ascending_def_propagator            (const ir_ascending_def_propagator&)     = delete;
    ir_ascending_def_propagator            (ir_ascending_def_propagator&&) noexcept = delete;
    ir_ascending_def_propagator& operator= (const ir_ascending_def_propagator&)     = delete;
    ir_ascending_def_propagator& operator= (ir_ascending_def_propagator&&) noexcept = delete;
    ~ir_ascending_def_propagator           (void)                                   = default;

    explicit
    ir_ascending_def_propagator (ir_subcomponent& sub, ir_def_timeline& dominator,
                                 ir_link_set<ir_block>&& incoming_blocks);

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
    visit (ir_function& func) noexcept { }

    result_type
    ascend (ir_substructure& sub, descender_type::result_type&& sub_result) const;

    descender_type::result_type
    dispatch_descender (ir_subcomponent& sub) const;

    descender_type::result_type
    dispatch_descender (ir_block& block) const;

    static
    bool
    needs_stop (const descender_type::result_type& res) noexcept;

    ir_def_timeline&              m_dominator;
    mutable ir_link_set<ir_block> m_incoming_blocks;
  };

}

#endif // OCTAVE_IR_IR_DEF_PROPAGATOR_HPP

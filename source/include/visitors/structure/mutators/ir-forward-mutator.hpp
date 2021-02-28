/** ir-forward-mutator.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_FORWARD_MUTATOR_HPP
#define OCTAVE_IR_IR_FORWARD_MUTATOR_HPP

#include "ir-structure-mutators-fwd.hpp"

#include "visitors/component/mutators/ir-component-mutators-fwd.hpp"

#include "visitors/structure/mutators/utility/ir-subcomponent-mutator.hpp"

#include <functional>

namespace gch
{

  class ir_descending_forward_mutator
    : visitor_traits<ir_descending_forward_mutator>
  {
  public:
    friend acceptor_type<ir_block>;
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

    using result_type  = bool;
    using functor_type = std::function<result_type (ir_block&)>;

    ir_descending_forward_mutator            (void)                                     = default;
    ir_descending_forward_mutator            (const ir_descending_forward_mutator&)     = default;
    ir_descending_forward_mutator            (ir_descending_forward_mutator&&) noexcept = default;
    ir_descending_forward_mutator& operator= (const ir_descending_forward_mutator&)     = default;
    ir_descending_forward_mutator& operator= (ir_descending_forward_mutator&&) noexcept = default;
    ~ir_descending_forward_mutator           (void)                                     = default;

    explicit
    ir_descending_forward_mutator (const functor_type& functor);

    explicit
    ir_descending_forward_mutator (functor_type&& functor);

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

    functor_type m_functor;
  };

  class ir_ascending_forward_mutator
    : public    visitor_traits<ir_ascending_forward_mutator>,
      protected ir_subcomponent_mutator
  {
  public:
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

    using result_type    = void;
    using descender_type = ir_descending_forward_mutator;
    using functor_type   = descender_type::functor_type;

    ir_ascending_forward_mutator            (void)                                    = delete;
    ir_ascending_forward_mutator            (const ir_ascending_forward_mutator&)     = delete;
    ir_ascending_forward_mutator            (ir_ascending_forward_mutator&&) noexcept = delete;
    ir_ascending_forward_mutator& operator= (const ir_ascending_forward_mutator&)     = delete;
    ir_ascending_forward_mutator& operator= (ir_ascending_forward_mutator&&) noexcept = delete;
    ~ir_ascending_forward_mutator           (void)                                    = default;

    explicit
    ir_ascending_forward_mutator (ir_subcomponent& sub, const functor_type& functor);

    explicit
    ir_ascending_forward_mutator (ir_subcomponent& sub, functor_type&& functor);

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
    ascend (ir_substructure& sub) const;

    [[nodiscard]]
    descender_type::result_type
    dispatch_descender (ir_subcomponent& sub) const;

    [[nodiscard]]
    descender_type::result_type
    dispatch_descender (ir_block& block) const;

    functor_type m_functor;
  };

}

#endif // OCTAVE_IR_IR_FORWARD_MUTATOR_HPP

/** ir-loop-component.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_LOOP_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_LOOP_HPP

#include "ir-structure.hpp"
#include "ir-block.hpp"

namespace gch
{

  class ir_component_loop final
    : public ir_substructure,
      public visitable<ir_component_loop, consolidated_visitors_t<ir_substructure>>
  {
  public:
    enum class subcomponent_id
    {
      start    ,
      condition,
      body     ,
      update   ,
      after    ,
    };

    using ptr  = ir_component_ptr;
    using cptr = ir_component_cptr;

    ir_component_loop (void)                                    = delete;
    ir_component_loop (const ir_component_loop&)                = delete;
    ir_component_loop (ir_component_loop&&) noexcept            = delete;
    ir_component_loop& operator= (const ir_component_loop&)     = delete;
    ir_component_loop& operator= (ir_component_loop&&) noexcept = delete;
    ~ir_component_loop (void) noexcept override;

    ir_component_loop (ir_structure& parent, ir_variable& condition_var);

    ir_component_loop (ir_structure& parent, ir_variable& condition_var, ir_component_mover comp);

    [[nodiscard]]
    ir_subcomponent&
    get_start (void) noexcept;

    [[nodiscard]]
    const ir_subcomponent&
    get_start (void) const noexcept;

    [[nodiscard]]
    ir_block&
    get_condition (void) noexcept;

    [[nodiscard]]
    const ir_block&
    get_condition (void) const noexcept;

    [[nodiscard]]
    ir_subcomponent&
    get_body (void) noexcept;

    [[nodiscard]]
    const ir_subcomponent&
    get_body (void) const noexcept;

    [[nodiscard]]
    ir_subcomponent&
    get_update (void) noexcept;

    [[nodiscard]]
    const ir_subcomponent&
    get_update (void) const noexcept;

    [[nodiscard]]
    ir_subcomponent&
    get_after (void) noexcept;

    [[nodiscard]]
    const ir_subcomponent&
    get_after (void) const noexcept;

    [[nodiscard]]
    bool
    is_start (const ir_subcomponent& sub) const noexcept;

    [[nodiscard]]
    bool
    is_condition (const ir_subcomponent& sub) const noexcept;

    [[nodiscard]]
    bool
    is_body (const ir_subcomponent& sub) const noexcept;

    [[nodiscard]]
    bool
    is_update (const ir_subcomponent& sub) const noexcept;

    [[nodiscard]]
    bool
    is_after (const ir_subcomponent& sub) const noexcept;

    [[nodiscard]]
    subcomponent_id
    get_id (const ir_subcomponent& c) const;

  private:
    std::unique_ptr<ir_subcomponent> m_start;     // preds: [pred]        | succs: condition
    ir_block                         m_condition; // preds: start, update | succs: body, after
    std::unique_ptr<ir_subcomponent> m_body;      // preds: condition     | succs: update
    std::unique_ptr<ir_subcomponent> m_update;    // preds: body          | succs: condition
    std::unique_ptr<ir_subcomponent> m_after;     // preds: body          | succs: condition

    //          +-----+     +---------+     +-----+
    // [] +---> |start| +-> |condition| +-> |after| +---> []
    //          +-----+     +---------+     +-----+
    //
    //                      +         ^
    //                      |         |
    //                      v         +
    //
    //                   +----+     +------+
    //                   |body| +-> |update|
    //                   +----+     +------+

  };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_LOOP_HPP

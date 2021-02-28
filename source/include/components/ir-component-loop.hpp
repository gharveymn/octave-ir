/** ir-loop-component.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_COMPONENT_LOOP_HPP
#define OCTAVE_IR_IR_COMPONENT_LOOP_HPP

#include "ir-structure.hpp"
#include "ir-block.hpp"

namespace gch
{

  class ir_component_loop
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
    };

    using ptr  = ir_component_ptr;
    using cptr = ir_component_cptr;

    ir_component_loop (void)                                    = delete;
    ir_component_loop (const ir_component_loop&)                = delete;
    ir_component_loop (ir_component_loop&&) noexcept            = delete;
    ir_component_loop& operator= (const ir_component_loop&)     = delete;
    ir_component_loop& operator= (ir_component_loop&&) noexcept = delete;
    ~ir_component_loop (void) noexcept override;

    explicit
    ir_component_loop (ir_structure& parent);

    [[nodiscard]]
    ir_subcomponent&
    get_start (void) noexcept
    {
      return *m_start;
    }

    [[nodiscard]]
    const ir_subcomponent&
    get_start (void) const noexcept
    {
      return as_mutable (*this).get_start ();
    }

    [[nodiscard]]
    ir_block&
    get_condition (void) noexcept
    {
      return m_condition;
    }

    [[nodiscard]]
    const ir_block&
    get_condition (void) const noexcept
    {
      return as_mutable (*this).get_condition ();
    }

    [[nodiscard]]
    ir_subcomponent&
    get_body (void) noexcept
    {
      return *m_body;
    }

    [[nodiscard]]
    const ir_subcomponent&
    get_body (void) const noexcept
    {
      return as_mutable (*this).get_body ();
    }

    [[nodiscard]]
    ir_subcomponent&
    get_update (void) noexcept
    {
      return *m_update;
    }

    [[nodiscard]]
    const ir_subcomponent&
    get_update (void) const noexcept
    {
      return as_mutable (*this).get_update ();
    }

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
    subcomponent_id
    get_id (const ir_subcomponent& c) const;

  private:
    std::unique_ptr<ir_subcomponent> m_start;     // preds: [pred]        | succs: condition
    ir_condition_block               m_condition; // preds: start, update | succs: body, [succ]
    std::unique_ptr<ir_subcomponent> m_body;      // preds: condition     | succs: update
    std::unique_ptr<ir_subcomponent> m_update;    // preds: body          | succs: condition

    //          +-----+     +---------+
    // [] +---> |start| +-> |condition| +---> []
    //          +-----+     +---------+
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

#endif // OCTAVE_IR_IR_COMPONENT_LOOP_HPP

/** ir-loop-component.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_COMPONENT_LOOP_HPP
#define OCTAVE_IR_IR_COMPONENT_LOOP_HPP

#include "ir-structure.hpp"

namespace gch
{

  class ir_component_loop;

  template <>
  struct ir_subcomponent_type_t<ir_component_loop>
  {
    explicit ir_subcomponent_type_t (void) = default;
  };

  class ir_component_loop
    : public ir_substructure
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
    ptr
    get_start (void) noexcept
    {
      return ptr { &m_start };
    }

    [[nodiscard]]
    cptr
    get_start (void) const noexcept
    {
      return cptr { &m_start };
    }

    [[nodiscard]]
    ptr
    get_condition (void) noexcept
    {
      return ptr { &m_condition };
    }

    [[nodiscard]]
    cptr
    get_condition (void) const noexcept
    {
      return cptr { &m_condition };
    }

    [[nodiscard]]
    ptr
    get_body (void) noexcept
    {
      return ptr { &m_body };
    }

    [[nodiscard]]
    cptr
    get_body (void) const noexcept
    {
      return cptr { &m_body };
    }

    [[nodiscard]]
    ptr
    get_update (void) noexcept
    {
      return ptr { &m_update };
    }

    [[nodiscard]]
    cptr
    get_update (void) const noexcept
    {
      return cptr { &m_update };
    }

    [[nodiscard]]
    bool
    is_start (cptr comp) const noexcept
    {
      return comp == get_start ();
    }

    [[nodiscard]]
    bool
    is_start (const ir_component& c) const noexcept
    {
      return &c == get_start ();
    }

    [[nodiscard]]
    bool
    is_condition (cptr comp) const noexcept
    {
      return comp == get_condition ();
    }

    [[nodiscard]]
    bool
    is_condition (const ir_component& c) const noexcept
    {
      return &c == get_condition ();
    }

    [[nodiscard]]
    bool
    is_body (cptr comp) const noexcept
    {
      return comp == get_body ();
    }

    [[nodiscard]]
    bool
    is_body (const ir_component& c) const noexcept
    {
      return &c == get_body ();
    }

    [[nodiscard]]
    bool
    is_update (cptr comp) const noexcept
    {
      return comp == get_update ();
    }

    [[nodiscard]]
    bool
    is_update (const ir_component& c) const noexcept
    {
      return &c == get_update ();
    }

    [[nodiscard]]
    subcomponent_id
    get_id (const ir_component& c) const;

    [[nodiscard]]
    subcomponent_id
    get_id (cptr comp) const
    {
      return get_id (*comp);
    }

    //
    // virtual from ir_component
    //

    bool
    reassociate_timelines (const ir_link_set<ir_def_timeline>& old_dts, ir_def_timeline& new_dt,
                           std::vector<nonnull_ptr<ir_block>>& until) override;

    void
    reset (void) noexcept override;

    //
    // virtual from ir_structure
    //

    [[nodiscard]]
    ir_component_ptr
    get_ptr (ir_component& c) const override;

    [[nodiscard]]
    ir_component_ptr
    get_entry_ptr (void) override;

    [[nodiscard]]
    ir_link_set<ir_block>
    get_predecessors (ir_component_cptr comp) override;

    [[nodiscard]]
    ir_link_set<ir_block>
    get_successors (ir_component_cptr comp) override;

    [[nodiscard]]
    bool
    is_leaf (ir_component_cptr comp) noexcept override;

    void
    generate_leaf_cache (void) override;

    ir_use_timeline&
    join_incoming_at (ir_component_ptr pos, ir_def_timeline& dt) override;

    void
    recursive_flatten (void) override;

  private:
    ir_component_storage m_start;     // preds: [pred]        | succs: condition
    ir_component_storage m_condition; // preds: start, update | succs: body, [succ]
    ir_component_storage m_body;      // preds: condition     | succs: update
    ir_component_storage m_update;    // preds: body          | succs: condition

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

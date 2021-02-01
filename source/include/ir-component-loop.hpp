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
  class ir_component_loop : public ir_structure
  {
  public:

    ir_component_loop (void)                                    = delete;
    ir_component_loop (const ir_component_loop&)                = delete;
    ir_component_loop (ir_component_loop&&) noexcept            = delete;
    ir_component_loop& operator= (const ir_component_loop&)     = delete;
    ir_component_loop& operator= (ir_component_loop&&) noexcept = delete;
    ~ir_component_loop (void) noexcept override;

    explicit
    ir_component_loop (ir_structure& parent);

    // link_iter preds_begin (ir_component& c) override;
    // link_iter preds_end   (ir_component& c) override;
    // link_iter succs_begin (ir_component& c) override;
    // link_iter succs_end   (ir_component& c) override;

    [[nodiscard]] constexpr
    const ir_component_handle&
    get_start_component (void) const noexcept
    {
      return m_start;
    }

    [[nodiscard]] constexpr
    const ir_component_handle&
    get_condition_component (void) const noexcept
    {
      return m_condition;
    }

    [[nodiscard]] constexpr
    const ir_component_handle&
    get_body_component (void) const noexcept
    {
      return m_body;
    }

    [[nodiscard]] constexpr
    const ir_component_handle&
    get_update_component (void) const noexcept
    {
      return m_update;
    }

    [[nodiscard]] constexpr
    bool
    is_start_component (const ir_component_handle& comp) const noexcept
    {
      return comp == get_start_component ();
    }

    [[nodiscard]] constexpr
    bool
    is_condition_component (const ir_component_handle& comp) const noexcept
    {
      return comp == get_condition_component ();
    }

    [[nodiscard]] constexpr
    bool
    is_body_component (const ir_component_handle& comp) const noexcept
    {
      return comp == get_body_component ();
    }

    [[nodiscard]] constexpr
    bool
    is_update_component (const ir_component_handle& comp) const noexcept
    {
      return comp == get_update_component ();
    }

    [[nodiscard]]
    std::list<nonnull_ptr<ir_def>> get_latest_defs (ir_variable& var) noexcept override
    {

      if (auto opt_def = m_exit.get_latest_def (var))
        return { *opt_def };

      if (auto opt_def = m_condition.get_latest_def (var))
        return { *opt_def };

      std::list<nonnull_ptr<ir_def>> ret = m_body.get_latest_defs (var);
      ret.splice (ret.end (), m_entry.get_latest_defs (var));
      return ret;
    }

    [[nodiscard]]
    std::list<nonnull_ptr<ir_def>>
    get_latest_defs_before (ir_variable& var, component_handle comp) override
    {
      if (is_entry (comp->get ()))
      {
        return { };
      }
      else if (is_body (comp->get ()))
      {
        return m_entry.get_latest_defs (var);
      }
      else if (is_condition (comp->get ()))
      {
        std::list<nonnull_ptr<ir_def>> ret = m_body.get_latest_defs (var);
        ret.splice (ret.end (), m_entry.get_latest_defs (var));
        return ret;
      }
      else if (is_exit (comp->get ()))
      {
        if (auto opt_def = m_condition.get_latest_def (var))
          return { *opt_def };

        std::list<nonnull_ptr<ir_def>> ret = m_body.get_latest_defs (var);
        ret.splice (ret.end (), m_entry.get_latest_defs (var));
        return ret;
      }
      else
      {
        throw ir_exception ("unexpected component handle");
      }
    }

    //
    // virtual from ir_component
    //

    void
    reset (void) noexcept override;

    //
    // virtual from ir_structure
    //

    [[nodiscard]]
    const ir_component_handle&
    get_entry_component (void) override;

    [[nodiscard]]
    const ir_component_handle&
    get_handle (const ir_component& c) const override;

    link_vector
    get_preds (const ir_component_handle& comp) override;

    link_vector
    get_succs (const ir_component_handle& comp) override;

    ir_use_timeline&
    join_incoming_at (ir_component_handle& block_handle, ir_def_timeline& dt) override;

    void
    generate_leaf_cache (void) override;

    [[nodiscard]]
    bool
    is_leaf_component (const ir_component_handle& comp) noexcept override;

  private:
    link_iter cond_succ_begin (void);

    link_iter cond_succ_end (void);

    ir_component_handle m_start;     // preds: [pred]        | succs: condition
    ir_component_handle m_condition; // preds: start, update | succs: body, [succ]
    ir_component_handle m_body;      // preds: condition     | succs: update
    ir_component_handle m_update;    // preds: body          | succs: condition

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

    // does not change
    link_vector m_cond_preds;

    link_vector m_succ_cache;

  };
}

#endif // OCTAVE_IR_IR_COMPONENT_LOOP_HPP

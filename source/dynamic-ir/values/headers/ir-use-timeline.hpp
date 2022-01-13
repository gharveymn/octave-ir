/** ir-use-timeline.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_USE_TIMELINE_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_USE_TIMELINE_HPP

#include "ir-common.hpp"
#include "ir-instruction-fwd.hpp"

#include <gch/optional_ref.hpp>
#include <gch/tracker/tracker.hpp>

namespace gch
{

  class ir_use;
  class ir_variable;

  class ir_use_timeline
    : public intrusive_tracker<ir_use_timeline, remote::intrusive_reporter<ir_use>>
  {
  public:
    using use_tracker  = intrusive_tracker<ir_use_timeline, remote::intrusive_reporter<ir_use>>;

    ir_use_timeline (void)                                  = delete;
    ir_use_timeline (const ir_use_timeline&)                = delete;
    ir_use_timeline (ir_use_timeline&&) noexcept            = default;
    ir_use_timeline& operator= (const ir_use_timeline&)     = delete;
    ir_use_timeline& operator= (ir_use_timeline&&) noexcept = default;
    ~ir_use_timeline (void)                                 = default;

    template <typename ...TrackerArgs>
    GCH_IMPLICIT_CONVERSION
    ir_use_timeline (ir_instruction_iter origin_pos, TrackerArgs&&... args)
      : use_tracker (std::forward<TrackerArgs> (args)...),
        m_instruction_pos (origin_pos)
    { }

    [[nodiscard]]
    ir_def&
    get_def (void) noexcept;

    [[nodiscard]]
    const ir_def&
    get_def (void) const noexcept;

    [[nodiscard]]
    ir_instruction_iter
    get_def_pos (void) noexcept;

    [[nodiscard]]
    ir_instruction_citer
    get_def_pos (void) const noexcept;

    [[nodiscard]]
    ir_instruction&
    get_def_instruction (void) noexcept;

    [[nodiscard]]
    const ir_instruction&
    get_def_instruction (void) const noexcept;

    [[nodiscard]]
    ir_variable&
    get_variable (void) noexcept;

    [[nodiscard]]
    const ir_variable&
    get_variable (void) const noexcept;

    void
    set_instruction_pos (ir_instruction_iter instr) noexcept;

    [[nodiscard]]
    bool
    has_uses (void) const noexcept;

  private:
    ir_instruction_iter m_instruction_pos;
  };

  [[nodiscard]]
  bool
  has_same_def (const ir_use_timeline& l, const ir_use_timeline& r) noexcept;

  [[nodiscard]]
  bool
  has_same_def (optional_cref<ir_use_timeline> lhs, const ir_use_timeline& r) noexcept;

  [[nodiscard]]
  bool
  has_same_def (const ir_use_timeline& l, optional_cref<ir_use_timeline> rhs) noexcept;

  [[nodiscard]]
  bool
  has_same_def (optional_cref<ir_use_timeline> lhs, optional_cref<ir_use_timeline> rhs) noexcept;

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_USE_TIMELINE_HPP

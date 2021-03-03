/** ir-use.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_USE_HPP
#define OCTAVE_IR_IR_USE_HPP

#include <gch/nonnull_ptr.hpp>
#include <gch/tracker/tracker.hpp>

namespace gch
{

  class ir_def;
  class ir_instruction;
  class ir_type;
  class ir_use_info;
  class ir_use_timeline;
  class ir_variable;

  class ir_use
    : public intrusive_reporter<ir_use, remote::intrusive_tracker<ir_use_timeline>>
  {
    using base = intrusive_reporter<ir_use, remote::intrusive_tracker<ir_use_timeline>>;

  public:
    using reporter_type       = base;
    using remote_tracker_type = typename tracker_traits<reporter_type>::remote_interface_type;
    using remote_iterator     = remote_tracker_type::citer;

    ir_use            (void)              = delete;
    ir_use            (const ir_use&)     = delete;
    ir_use            (ir_use&&) noexcept = default;
    ir_use& operator= (const ir_use&)     = delete;
    ir_use& operator= (ir_use&&) noexcept = default;
    ~ir_use           (void)              = default;

    ir_use (ir_instruction& instr, ir_use_timeline& ut, remote_iterator pos);

    ir_use (ir_instruction& instr, const ir_use_info& info);

    [[nodiscard]]
    ir_use_timeline&
    get_timeline (void) noexcept;

    [[nodiscard]]
    const ir_use_timeline&
    get_timeline (void) const noexcept;

    [[nodiscard]]
    ir_def&
    get_def (void) noexcept;

    [[nodiscard]]
    const ir_def&
    get_def (void) const noexcept;

    [[nodiscard]]
    ir_variable&
    get_variable (void) noexcept;

    [[nodiscard]]
    const ir_variable&
    get_variable (void) const noexcept;

    [[nodiscard]]
    ir_type
    get_type (void) const noexcept;

    [[nodiscard]]
    const std::string&
    get_name (void) const;

    [[nodiscard]]
    std::size_t
    get_id (void);

    [[nodiscard]]
    ir_instruction&
    get_instruction (void) noexcept;

    [[nodiscard]]
    const ir_instruction&
    get_instruction (void) const noexcept;

    void
    set_instruction (ir_instruction& instr) noexcept;

  private:
    nonnull_ptr<ir_instruction> m_instr;
  };

  class ir_use_info
  {
  public:
    using remote_iterator = ir_use::remote_iterator;

    ir_use_info            (void)                   = delete;
    ir_use_info            (const ir_use_info&)     = default;
    ir_use_info            (ir_use_info&&) noexcept = default;
    ir_use_info& operator= (const ir_use_info&)     = default;
    ir_use_info& operator= (ir_use_info&&) noexcept = default;
    ~ir_use_info           (void)                   = default;

    ir_use_info (ir_use_timeline& ut, remote_iterator pos);

    [[nodiscard]]
    ir_use_timeline&
    get_timeline (void) const noexcept;

    [[nodiscard]]
    remote_iterator
    get_insertion_position (void) const noexcept;

  private:
    nonnull_ptr<ir_use_timeline> m_timeline;
    remote_iterator              m_insertion_position;
  };

}

#endif // OCTAVE_IR_IR_USE_HPP

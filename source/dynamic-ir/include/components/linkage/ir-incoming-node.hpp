/** ir-incoming-node.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_INCOMING_NODE_HPP
#define OCTAVE_IR_IR_INCOMING_NODE_HPP

#include "components/ir-block-common.hpp"
#include "gch/octave-ir-utilities/ir-allocator-util.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>
#include <gch/tracker/reporter.hpp>

namespace gch
{

  class ir_def_timeline;

  class ir_incoming_node
    : public intrusive_reporter<ir_incoming_node, remote::intrusive_tracker<ir_def_timeline>>
  {
    using base = intrusive_reporter<ir_incoming_node, remote::intrusive_tracker<ir_def_timeline>>;

  public:
    using element_allocator = strongly_linked_allocator<ir_incoming_node, ir_def_timeline>;

    ir_incoming_node            (void)                        = delete;
    ir_incoming_node            (const ir_incoming_node&)     = delete;
    ir_incoming_node            (ir_incoming_node&&) noexcept = delete;
    ir_incoming_node& operator= (const ir_incoming_node&)     = delete;
    ir_incoming_node& operator= (ir_incoming_node&&) noexcept;
    ~ir_incoming_node           (void)                        = default;

    ir_incoming_node (ir_def_timeline& parent, ir_incoming_node&& other) noexcept;

    ir_incoming_node (ir_def_timeline& parent, nullopt_t);
    ir_incoming_node (ir_def_timeline& parent, ir_def_timeline& incoming);
    ir_incoming_node (ir_def_timeline& parent, optional_ref<ir_def_timeline> opt_incoming);

    void
    set_parent_timeline (ir_def_timeline& dt) noexcept;

    [[nodiscard]]
    ir_def_timeline&
    get_parent_timeline (void) noexcept;

    [[nodiscard]]
    const ir_def_timeline&
    get_parent_timeline (void) const noexcept;

    [[nodiscard]]
    ir_block&
    get_parent_block (void) noexcept;

    [[nodiscard]]
    const ir_block&
    get_parent_block (void) const noexcept;

    [[nodiscard]]
    bool
    has_incoming_timeline (void) const noexcept;

    [[nodiscard]]
    ir_def_timeline&
    get_incoming_timeline (void) noexcept;

    [[nodiscard]]
    const ir_def_timeline&
    get_incoming_timeline (void) const noexcept;

    void
    swap (ir_incoming_node& other) noexcept;

  private:
    nonnull_ptr<ir_def_timeline> m_parent_timeline;
  };

  void
  swap (ir_incoming_node& lhs, ir_incoming_node& rhs) noexcept;

}

#endif // OCTAVE_IR_IR_INCOMING_NODE_HPP

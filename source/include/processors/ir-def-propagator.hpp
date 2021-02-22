/** ir-def-propagator.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_DEF_PROPAGATOR_HPP
#define OCTAVE_IR_IR_DEF_PROPAGATOR_HPP

#include "utilities/ir-link-set.hpp"

#include <gch/small_vector.hpp>

#include <stack>

namespace gch
{

  class ir_block;

  class ir_def_propagator_frame
  {
  public:
    ir_def_propagator_frame            (void)                               = delete;
    ir_def_propagator_frame            (const ir_def_propagator_frame&)     = default;
    ir_def_propagator_frame            (ir_def_propagator_frame&&) noexcept = default;
    ir_def_propagator_frame& operator= (const ir_def_propagator_frame&)     = default;
    ir_def_propagator_frame& operator= (ir_def_propagator_frame&&) noexcept = default;
    ~ir_def_propagator_frame           (void)                               = default;

    ir_def_propagator_frame (ir_block& join_block, ir_link_set<ir_block>&& propagated_incoming);

  private:
    nonnull_ptr<ir_block> m_join_block;
    ir_link_set<ir_block> m_propagated_incoming;
    ir_link_set<ir_block> m_merged_incoming;
  };

  class ir_def_propagator
  {
  public:


  private:
    std::stack<ir_def_propagator_frame, small_vector<ir_def_propagator_frame>> m_stack;
  };

}

#endif // OCTAVE_IR_IR_DEF_PROPAGATOR_HPP

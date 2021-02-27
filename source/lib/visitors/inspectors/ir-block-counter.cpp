/** ir-block-counter.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/inspectors/ir-block-counter.hpp"

namespace gch
{

  template <>
  auto
  acceptor<ir_block, ir_block_counter>::
  accept (visitor_reference v) const
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<ir_component_fork, ir_block_counter>::
  accept (visitor_reference v) const
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<ir_component_loop, ir_block_counter>::
  accept (visitor_reference v) const
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<ir_component_sequence, ir_block_counter>::
  accept (visitor_reference v) const
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<ir_function, ir_block_counter>::
  accept (visitor_reference v) const
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  auto
  ir_block_counter::
  operator() (const ir_structure& s) const
  -> result_type
  {
    return s.accept (*this);
  }

  auto
  ir_block_counter::
  visit (const ir_block& block)
  -> result_type
  {
    return { nonnull_ptr (as_mutable (block)) };
  }

  auto
  ir_block_counter::
  visit (const ir_component_fork& fork) const
  -> result_type
  {
    result_type ret;
    std::for_each (fork.cases_begin (), fork.cases_end (),
                   [&](const ir_subcomponent& sub) { ret += subcomponent_result (sub); });
    return ret;
  }

  auto
  ir_block_counter::
  visit (const ir_component_loop& loop) const
  -> result_type
  {
    return subcomponent_result (loop.get_condition ());
  }

  auto
  ir_block_counter::
  visit (const ir_component_sequence& seq) const
  -> result_type
  {
    return subcomponent_result (*seq.last ());
  }

  auto
  ir_block_counter::
  visit (const ir_function& func) const
  -> result_type
  {
    return subcomponent_result (func.get_body ());
  }

  auto
  ir_block_counter::
  subcomponent_result (const ir_subcomponent& sub) const
  -> result_type
  {
    return sub.accept (*this);
  }

}

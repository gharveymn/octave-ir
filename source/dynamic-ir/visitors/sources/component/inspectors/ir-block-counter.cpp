/** ir-block-counter.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "component/inspectors/ir-block-counter.hpp"

#include "ir-block.hpp"
#include "ir-component-fork.hpp"
#include "ir-component-loop.hpp"
#include "ir-component-sequence.hpp"
#include "ir-function.hpp"

#include <numeric>

namespace gch
{

  template class acceptor<ir_block,              inspector_type<ir_block_counter>>;
  template class acceptor<ir_component_fork,     inspector_type<ir_block_counter>>;
  template class acceptor<ir_component_loop,     inspector_type<ir_block_counter>>;
  template class acceptor<ir_component_sequence, inspector_type<ir_block_counter>>;
  template class acceptor<ir_function,           inspector_type<ir_block_counter>>;

  template <>
  auto
  ir_block_counter::acceptor_type<ir_block>::
  accept (visitor_reference_t<ir_block_counter> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_block_counter::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_block_counter> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_block_counter::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_block_counter> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_block_counter::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_block_counter> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_block_counter::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_block_counter> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  auto
  ir_block_counter::
  operator() (const ir_component& c) const
  -> result_type
  {
    return c.accept (*this);
  }

  auto
  ir_block_counter::
  visit (const ir_block&)
    -> result_type
  {
    return 1;
  }

  auto
  ir_block_counter::
  visit (const ir_component_fork& fork)
  -> result_type
  {
    return std::accumulate (fork.cases_begin (), fork.cases_end (), result_type (1),
                            [](result_type res, const ir_subcomponent& sub)
                            {
                              return res + subcomponent_result (sub);
                            });
  }

  auto
  ir_block_counter::
  visit (const ir_component_loop& loop)
    -> result_type
  {
    return subcomponent_result (loop.get_start ())
         + result_type (1) // condition block
         + subcomponent_result (loop.get_body ())
         + subcomponent_result (loop.get_update ());
  }

  auto
  ir_block_counter::
  visit (const ir_component_sequence& seq)
    -> result_type
  {
    return std::accumulate (seq.begin (), seq.end (), result_type (0),
                            [](result_type res, const ir_subcomponent& sub)
                            {
                              return res + subcomponent_result (sub);
                            });
  }

  auto
  ir_block_counter::
  visit (const ir_function& func)
  -> result_type
  {
    return subcomponent_result (func.get_body ());
  }

  auto
  ir_block_counter::
  subcomponent_result (const ir_subcomponent& sub)
    -> result_type
  {
    return sub.accept (ir_block_counter { });
  }

}

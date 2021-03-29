/** ir-successor-collector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/structure/inspectors/ir-successor-collector.hpp"

#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"
#include "gch/octave-ir-utilities/ir-error.hpp"
#include "gch/octave-ir-utilities/ir-iterator.hpp"

namespace gch
{

  template <>
  auto
  ir_successor_collector::acceptor_type<ir_component_fork>::
  accept (visitor_reference_t<ir_successor_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_successor_collector::acceptor_type<ir_component_loop>::
  accept (visitor_reference_t<ir_successor_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_successor_collector::acceptor_type<ir_component_sequence>::
  accept (visitor_reference_t<ir_successor_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  ir_successor_collector::acceptor_type<ir_function>::
  accept (visitor_reference_t<ir_successor_collector> v) const
    -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  auto
  ir_successor_collector::
  operator() (void) const
    -> result_type
  {
    return get_parent ().accept (*this);
  }

  auto
  ir_successor_collector::
  visit (const ir_component_fork& fork) const
    -> result_type
  {
    if (! fork.is_condition (get_subcomponent ()))
      return get_successors (fork);

    ir_link_set<ir_block> ret;
    std::transform (fork.cases_begin (), fork.cases_end (), set_inserter { ret },
                    [&](const ir_component& c)
                    {
                      return nonnull_ptr { as_mutable (get_entry_block (c)) };
                    });
    return ret;
  }

  auto
  ir_successor_collector::
  visit (const ir_component_loop& loop) const
    -> result_type
  {
    using id = ir_component_loop::subcomponent_id;
    switch (loop.get_id (get_subcomponent ()))
    {
      case id::start:
        return { nonnull_ptr { as_mutable (loop.get_condition ()) } };
      case id::condition:
      {
        ir_link_set res { get_successors (loop) };
        res.emplace (as_mutable (get_entry_block (loop.get_body ())));
        return res;
      }
      case id::body:
        return { nonnull_ptr { as_mutable (get_entry_block (loop.get_update ())) } };
      case id::update:
        return { nonnull_ptr { as_mutable (loop.get_condition ()) } };
      default:
        abort<reason::impossible> ();
    }
  }

  auto
  ir_successor_collector::
  visit (const ir_component_sequence& seq) const
    -> result_type
  {
    auto found = seq.find (get_subcomponent ());
    assert (found != seq.end ());
    if (found == seq.last ())
      return (get_successors (seq));
    else
      return { nonnull_ptr { as_mutable (get_entry_block (*std::next (found))) } };
  }

}

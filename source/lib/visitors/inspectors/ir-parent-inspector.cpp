/** ir-parent-inspector.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/inspectors/ir-parent-inspector.hpp"

#include "components/ir-component.hpp"
#include "components/ir-structure.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"

namespace gch
{

  //
  // ir_predecessor_collector
  //

  ir_link_set<ir_block>
  ir_predecessor_collector::
  operator() (const ir_subcomponent& sub)
  {
    set_subcomponent (sub);
    sub.get_parent ().accept (*this);
    return release_result ();
  }

  void
  ir_predecessor_collector::
  visit (const ir_component_fork& fork)
  {
    if (fork.is_condition (get_subcomponent ()))
      set_result (get_predecessors (fork));
    else
      set_result (copy_leaves (fork.get_condition ()));
  }

  void
  ir_predecessor_collector::
  visit (const ir_component_loop& loop)
  {
    using id = ir_component_loop::subcomponent_id;
    switch (loop.get_id (get_subcomponent ()))
    {
      case id::start     : set_result (get_predecessors (loop));                             break;
      case id::condition : set_result (copy_leaves (loop.get_start (), loop.get_update ())); break;
      case id::body      : set_result (copy_leaves (loop.get_condition ()));                 break;
      case id::update    : set_result (copy_leaves (loop.get_body ()));                      break;
    }
  }

  void
  ir_predecessor_collector::
  visit (const ir_component_sequence& seq)
  {
    auto found = seq.find (get_subcomponent ());
    assert (found != seq.end ());
    if (found == seq.begin ())
      set_result (get_predecessors (seq));
    else
      set_result (copy_leaves (*std::prev (found)));
  }

  void
  ir_predecessor_collector::
  visit (const ir_function& func)
  { }

  ir_link_set<ir_block>
  get_predecessors (const ir_subcomponent& sub)
  {
    return ir_predecessor_collector { } (sub);
  }

  //
  // ir_successor_collector
  //

  ir_link_set<ir_block>
  ir_successor_collector::
  operator() (const ir_subcomponent& sub)
  {
    set_subcomponent (sub);
    sub.get_parent ().accept (*this);
    return release_result ();
  }

  void
  ir_successor_collector::
  visit (const ir_component_fork& fork)
  {
    if (! fork.is_condition (get_subcomponent ()))
      set_result (get_successors (fork));

    ir_link_set<ir_block> res;
    std::transform (fork.cases_begin (), fork.cases_end (), set_inserter { res },
                    [&](const ir_component& c)
                    {
                      return nonnull_ptr { as_mutable (get_entry_block (c)) };
                    });
    set_result (std::move (res));
  }

  void
  ir_successor_collector::
  visit (const ir_component_loop& loop)
  {
    using id = ir_component_loop::subcomponent_id;
    switch (loop.get_id (get_subcomponent ()))
    {
      case id::start:
      {
        set_result ({ nonnull_ptr { as_mutable (loop.get_condition ()) } });
        break;
      }
      case id::condition:
      {
        ir_link_set res { get_successors (loop) };
        res.emplace (as_mutable (get_entry_block (loop.get_body ())));
        set_result (std::move (res));
        break;
      }
      case id::body:
      {
        set_result ({ nonnull_ptr { as_mutable (get_entry_block (loop.get_update ())) } });
        break;
      }
      case id::update:
      {
        set_result ({ nonnull_ptr { as_mutable (loop.get_condition ()) } });
        break;
      }
    }
  }

  void
  ir_successor_collector::
  visit (const ir_component_sequence& seq)
  {
    auto found = seq.find (get_subcomponent ());
    assert (found != seq.end ());
    if (found == seq.last ())
      set_result (get_successors (seq));
    else
      set_result ({ nonnull_ptr { as_mutable (get_entry_block (*std::next (found))) } });
  }

  void
  ir_successor_collector::
  visit (const ir_function& func)
  {
    set_result ({ });
  }

  ir_link_set<ir_block>
  get_successors (const ir_subcomponent& sub)
  {
    return ir_successor_collector { } (sub);
  }

  //
  // ir_leaf_inspector
  //

  bool
  ir_leaf_inspector::
  operator() (const ir_subcomponent& sub)
  {
    set_subcomponent (sub);
    sub.get_parent ().accept (*this);
    return release_result ();
  }

  void
  ir_leaf_inspector::
  visit (const ir_component_fork& fork)
  {
    set_result (&get_subcomponent () != &fork.get_condition ());
  }

  void
  ir_leaf_inspector::
  visit (const ir_component_loop& loop)
  {
    set_result (loop.is_condition (get_subcomponent ()));
  }

  void
  ir_leaf_inspector::
  visit (const ir_component_sequence& seq)
  {
    auto found = seq.find (get_subcomponent ());
    assert (found != seq.end ());
    set_result (found == seq.last ());
  }

  void
  ir_leaf_inspector::
  visit (const ir_function& func)
  {
    set_result (true);
  }

  bool
  is_leaf (const ir_subcomponent& sub)
  {
    return ir_leaf_inspector { } (sub);
  }

}

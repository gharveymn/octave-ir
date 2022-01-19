/** ir-component.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-component.hpp"
#include "ir-function.hpp"
#include "ir-structure.hpp"
#include "ir-block.hpp"
#include "structure/inspectors/ir-predecessor-collector.hpp"
#include "structure/inspectors/ir-successor-collector.hpp"
#include "structure/inspectors/ir-leaf-inspector.hpp"

namespace gch
{

  ir_component::
  ~ir_component (void) = default;

  ir_subcomponent::
  ~ir_subcomponent (void) = default;

  ir_block&
  get_entry_block (ir_component& c)
  {
    if (optional_ref s { maybe_cast<ir_structure> (c) })
      return get_entry_block (*s);
    assert (is_a<ir_block> (c));
    return static_cast<ir_block&> (c);
  }

  const ir_block&
  get_entry_block (const ir_component& c)
  {
    return get_entry_block (as_mutable (c));
  }

  ir_function&
  get_function (ir_subcomponent& sub)
  {
    // Note: this is too simple to need to create a new visitor.
    nonnull_ptr<ir_structure> parent { sub.get_parent () };
    while (optional_ref s { maybe_cast<ir_substructure> (*parent) })
      parent.emplace (s->get_parent ());
    assert (is_a<ir_function> (*parent));
    return static_cast<ir_function&> (*parent);
  }

  const ir_function&
  get_function (const ir_subcomponent& sub)
  {
    return get_function (as_mutable (sub));
  }

  std::string
  get_name (const ir_component& c)
  {
    // FIXME: temporary
    return "component" + std::to_string (reinterpret_cast<std::size_t> (&c));
  }

  ir_link_set<ir_block>
  get_predecessors (const ir_subcomponent& sub)
  {
    return ir_predecessor_collector { sub } ();
  }

  ir_link_set<ir_block>
  get_successors (const ir_subcomponent& sub)
  {
    return ir_successor_collector { sub } ();
  }

  bool
  is_leaf (const ir_subcomponent& sub)
  {
    return ir_leaf_inspector { sub } ();
  }

  bool
  is_subcomponent_of (const ir_component& parent, const ir_subcomponent& sub)
  {
    optional_ref curr { sub };
    do
    {
      if (curr.refers_to (parent))
        return true;
    } while ((curr = maybe_cast<ir_subcomponent> (curr->get_parent ())));

    return is_a<ir_function> (parent);
  }

  bool
  is_leaf_of (const ir_component& parent, const ir_subcomponent& sub)
  {
    optional_ref curr { sub };
    do
    {
      if (curr.refers_to (parent))
        return true;
    } while (is_leaf (*curr) && (curr = maybe_cast<ir_subcomponent> (curr->get_parent ())));

    return is_a<ir_function> (parent);
  }

  bool
  is_entry (const ir_subcomponent& sub)
  {
    return &sub == &get_entry_component (sub.get_parent ());
  }

  ir_link_set<ir_block>
  copy_leaves (const ir_component& c)
  {
    // delegate if the component is an ir_structure
    if (optional_ref s { maybe_cast<ir_structure> (c) })
      return copy_leaves (*s);

    // else it should just be a block
    assert (is_a<ir_block> (c));
    return copy_leaves (static_cast<const ir_block&> (c));
  }

  ir_link_set<ir_block>
  copy_leaves (const ir_structure& s)
  {
    return s.get_leaves ();
  }

  ir_link_set<ir_block>
  copy_leaves (const ir_block& b)
  {
    return { nonnull_ptr { as_mutable (b) } };
  }

}

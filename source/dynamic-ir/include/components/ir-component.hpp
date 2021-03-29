/*

Copyright (C) 2019 Gene Harvey

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#ifndef OCTAVE_IR_IR_COMPONENT_HPP
#define OCTAVE_IR_IR_COMPONENT_HPP

#include "components/utility/ir-component-handle.hpp"
#include "gch/octave-ir-utilities/ir-utility.hpp"
#include "gch/octave-ir-utilities/ir-link-set.hpp"

#include "visitors/ir-visitor.hpp"
#include "visitors/component/ir-component-visitors-fwd.hpp"
#include "visitors/subcomponent/ir-subcomponent-visitors-fwd.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>
#include <gch/small_vector.hpp>

#include <list>
#include <memory>

namespace gch
{

  class ir_block;
  class ir_function;
  class ir_def;
  class ir_variable;
  class ir_structure;
  class ir_def_timeline;

  template <typename T>
  using is_ir_component = std::is_base_of<ir_component, T>;

  // abstract
  class ir_component
    : public abstract_visitable<exclusive_visitors_t<ir_component>>
  {
  public:
    ir_component            (void)                    = default;
    ir_component            (const ir_component&)     = default;
    ir_component            (ir_component&&) noexcept = default;
    ir_component& operator= (const ir_component&)     = default;
    ir_component& operator= (ir_component&&) noexcept = default;
    ~ir_component           (void) override;

  private:
  };

  // a subcomponent is any component that isn't an function
  class ir_subcomponent
    : public ir_component,
      public abstract_visitable<exclusive_visitors_t<ir_subcomponent>>
  {
  public:
    ir_subcomponent            (void)                       = delete;
    ir_subcomponent            (const ir_subcomponent&)     = default;
    ir_subcomponent            (ir_subcomponent&&) noexcept = default;
    ir_subcomponent& operator= (const ir_subcomponent&)     = default;
    ir_subcomponent& operator= (ir_subcomponent&&) noexcept = default;
    ~ir_subcomponent           (void) override;

    using ir_component::accept;
    using abstract_visitable<exclusive_visitors_t<ir_subcomponent>>::accept;

    explicit
    ir_subcomponent (ir_structure& parent)
      : m_parent (parent)
    { }

    [[nodiscard]]
    ir_structure&
    get_parent (void) noexcept
    {
      return *m_parent;
    }

    [[nodiscard]]
    const ir_structure&
    get_parent (void) const noexcept
    {
      return as_mutable (*this).get_parent ();
    }

    void
    set_parent (ir_structure& s) noexcept
    {
      m_parent.emplace (s);
    }

  private:
    nonnull_ptr<ir_structure> m_parent;
  };

  ir_block&
  get_entry_block (ir_component& c);

  const ir_block&
  get_entry_block (const ir_component& c);

  [[nodiscard]]
  ir_function&
  get_function (ir_subcomponent& sub);

  [[nodiscard]]
  const ir_function&
  get_function (const ir_subcomponent& sub);

  [[nodiscard]]
  std::string
  get_name (const ir_component& c);

  ir_link_set<ir_block>
  get_predecessors (const ir_subcomponent& sub);

  ir_link_set<ir_block>
  get_successors (const ir_subcomponent& sub);

  bool
  is_leaf (const ir_subcomponent& sub);

  bool
  is_subcomponent_of (const ir_component& parent, const ir_subcomponent& sub);

  bool
  is_leaf_of (const ir_component& parent, const ir_subcomponent& sub);

  bool
  is_entry (const ir_subcomponent& sub);

  [[nodiscard]]
  ir_link_set<ir_block>
  copy_leaves (const ir_component& c);

  [[nodiscard]]
  ir_link_set<ir_block>
  copy_leaves (const ir_structure& s);

  [[nodiscard]]
  ir_link_set<ir_block>
  copy_leaves (const ir_block& b);

  template <typename ...Components>
  [[nodiscard]] inline
  ir_link_set<ir_block>
  copy_leaves (Components&&... components)
  {
    return (copy_leaves (std::forward<Components> (components)) | ...);
  }

}

#endif

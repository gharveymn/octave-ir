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

#include "ir-component-handle.hpp"
#include "utilities/ir-common-util.hpp"
#include "utilities/ir-link-set.hpp"

#include "visitors/ir-visitor.hpp"
#include "visitors/ir-visitor-fwd.hpp"

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

  template <typename BlockVisitor>
  class ir_forward_descender;

  template <typename BlockVisitor>
  class ir_forward_ascender;

  template <typename BlockVisitor>
  class ir_backward_descender;

  template <typename BlockVisitor>
  class ir_backward_ascender;

  class ir_block_visitor_prototype;

  // abstract
  class ir_component
    : public abstract_visitable<ir_component_visitors>
  {
  public:
    ir_component            (void)                    = default;
    ir_component            (const ir_component&)     = default;
    ir_component            (ir_component&&) noexcept = default;
    ir_component& operator= (const ir_component&)     = default;
    ir_component& operator= (ir_component&&) noexcept = default;
    virtual ~ir_component   (void)           noexcept = 0;

    template <typename T>
    using is_component = std::is_base_of<ir_component, T>;

  private:
  };

  // a subcomponent is any component that isn't an function
  class ir_subcomponent
    : public ir_component
  {
  public:
    ir_subcomponent            (void)                       = delete;
    ir_subcomponent            (const ir_subcomponent&)     = default;
    ir_subcomponent            (ir_subcomponent&&) noexcept = default;
    ir_subcomponent& operator= (const ir_subcomponent&)     = default;
    ir_subcomponent& operator= (ir_subcomponent&&) noexcept = default;
    ~ir_subcomponent           (void) override              = 0;

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

  template <typename Component>
  struct ir_subcomponent_type_t;

  template <typename Component>
  inline constexpr
  ir_subcomponent_type_t<Component>
  ir_subcomponent_type { };

  [[nodiscard]]
  ir_function&
  get_function (ir_component& c);

  [[nodiscard]] inline
  const ir_function&
  get_function (const ir_component& c)
  {
    return get_function (as_mutable (c));
  }

  [[nodiscard]]
  ir_block&
  get_entry_block (ir_component& c);

  [[nodiscard]] inline
  const ir_block&
  get_entry_block (const ir_component& c)
  {
    return get_entry_block (as_mutable (c));
  }

  [[nodiscard]] inline
  ir_block&
  get_entry_block (ir_component_ptr comp)
  {
    return get_entry_block (*comp);
  }

  [[nodiscard]] inline
  const ir_block&
  get_entry_block (ir_component_cptr comp)
  {
    return get_entry_block (*comp);
  }

  ir_link_set<ir_block>
  get_predecessors (const ir_subcomponent& sub);

  ir_link_set<ir_block>
  get_successors (const ir_subcomponent& sub);

  bool
  is_leaf (const ir_subcomponent& sub);

  [[nodiscard]]
  ir_link_set<ir_block>
  copy_leaves (const ir_component& c);

  [[nodiscard]]
  ir_link_set<ir_block>
  copy_leaves (const ir_structure& s);

  [[nodiscard]]
  ir_link_set<ir_block>
  copy_leaves (const ir_block& b);

  template <typename ...Args>
  [[nodiscard]] inline
  ir_link_set<ir_block>
  copy_leaves (const ir_component& c, Args&&... args)
  {
    return (copy_leaves (c) | ... | copy_leaves (std::forward<Args> (args)));
  }

  template <typename ...Args>
  [[nodiscard]] inline
  ir_link_set<ir_block>
  copy_leaves (const ir_structure& s, Args&&... args)
  {
    return (copy_leaves (s) | ... | copy_leaves (std::forward<Args> (args)));
  }

  template <typename ...Args>
  [[nodiscard]] inline
  ir_link_set<ir_block>
  copy_leaves (const ir_block& b, Args&&... args)
  {
    return (copy_leaves (b) | ... | copy_leaves (std::forward<Args> (args)));
  }

}

#endif

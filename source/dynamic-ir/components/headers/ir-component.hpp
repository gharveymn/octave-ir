/** ir-component.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_HPP

#include "utility/ir-component-handle.hpp"
#include "ir-utility.hpp"
#include "ir-link-set.hpp"

#include "ir-visitor.hpp"
#include "component/ir-component-visitors-fwd.hpp"
#include "subcomponent/ir-subcomponent-visitors-fwd.hpp"

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

  template <typename T>
  inline constexpr
  bool
  is_ir_component_v = is_ir_component<T>::value;

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

  template <typename ...Components,
            std::enable_if_t<(1 < sizeof...(Components))> * = nullptr>
  [[nodiscard]] inline
  ir_link_set<ir_block>
  copy_leaves (Components&&... components)
  {
    return (copy_leaves (std::forward<Components> (components)) | ...);
  }

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_HPP

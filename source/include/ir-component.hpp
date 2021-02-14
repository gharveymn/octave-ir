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

#if ! defined (ir_component_h)
#define ir_component_h 1

#include "ir-component-handle.hpp"
#include "ir-common-util.hpp"
#include "ir-link-set.hpp"

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
  class ir_substructure_descender;

  template <typename BlockVisitor>
  class ir_superstructure_descender;

  template <typename BlockVisitor>
  class ir_substructure_ascender;

  template <typename BlockVisitor>
  class ir_superstructure_ascender;

  class ir_block_visitor_prototype;

  using traverser_types = visitor_types<ir_substructure_descender<ir_block_visitor_prototype>,
                                        ir_superstructure_descender<ir_block_visitor_prototype>,
                                        ir_substructure_ascender<ir_block_visitor_prototype>,
                                        ir_superstructure_ascender<ir_block_visitor_prototype>>;

  // abstract
  class ir_component
    : public visitable<ir_component, traverser_types>
  {
  public:
    ir_component            (void)                    = delete;
    ir_component            (const ir_component&)     = default;
    ir_component            (ir_component&&) noexcept = default;
    ir_component& operator= (const ir_component&)     = default;
    ir_component& operator= (ir_component&&) noexcept = default;
    virtual ~ir_component (void) noexcept             = 0;

    explicit
    ir_component (ir_structure& parent)
      : m_parent (parent)
    { }

    explicit
    ir_component (nullopt_t)
      : m_parent (nullopt)
    { }

    [[nodiscard]]
    bool
    has_parent (void) const noexcept
    {
      return m_parent.has_value ();
    }

    [[nodiscard]]
    optional_ref<ir_structure>
    maybe_get_parent (void) noexcept
    {
      return m_parent;
    }

    [[nodiscard]]
    optional_cref<ir_structure>
    maybe_get_parent (void) const noexcept
    {
      return m_parent;
    }

    void
    set_parent (ir_structure& s) noexcept
    {
      m_parent.emplace (s);
    }

    template <typename T>
    using is_component = std::is_base_of<ir_component, T>;

    //
    // virtual functions
    //

    // returns whether the caller should stop executing
    virtual
    bool
    reassociate_timelines (const ir_link_set<ir_def_timeline>& old_dts, ir_def_timeline& new_dt,
                           std::vector<nonnull_ptr<ir_block>>& until) = 0;

    virtual
    void
    reset (void) noexcept = 0;

    //
    // virtual function accessories
    //

  private:
    optional_ref<ir_structure> m_parent;
  };

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

  template <typename Component>
  struct ir_subcomponent_type_t;

  template <typename Component>
  inline constexpr ir_subcomponent_type_t<Component> ir_subcomponent_type { };

}

#endif

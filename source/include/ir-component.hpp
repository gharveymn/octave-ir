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

  // abstract
  class ir_component
  {
  public:
    using link_vector = small_vector<nonnull_ptr<ir_block>, 1>;
    using link_iter   = link_vector::iterator;
    using link_citer  = link_vector::const_iterator;

    ir_component            (void)                    = delete;
    ir_component            (const ir_component&)     = default;
    ir_component            (ir_component&&) noexcept = default;
    ir_component& operator= (const ir_component&)     = default;
    ir_component& operator= (ir_component&&) noexcept = default;
    virtual ~ir_component (void) noexcept             = 0;

    explicit
    ir_component (ir_structure& s)
      : m_parent (s)
    { }

    explicit
    ir_component (nullopt_t)
      : m_parent (nullopt)
    { }

    [[nodiscard]] constexpr
    bool
    has_parent (void) const noexcept
    {
      return m_parent.has_value ();
    }

    [[nodiscard]] constexpr
    ir_structure&
    get_parent (void) noexcept
    {
      return *m_parent;
    }

    [[nodiscard]] constexpr
    const ir_structure&
    get_parent (void) const noexcept
    {
      return *m_parent;
    }

    [[nodiscard]] constexpr
    optional_ref<ir_structure>
    maybe_get_parent (void) noexcept
    {
      return m_parent;
    }

    [[nodiscard]] constexpr
    optional_cref<ir_structure>
    maybe_get_parent (void) const noexcept
    {
      return m_parent;
    }

    template <typename T>
    using is_component = std::is_base_of<ir_component, T>;

    //
    // virtual functions
    //

    [[nodiscard]]
    virtual
    ir_block&
    get_entry_block (void) noexcept = 0;

    virtual
    void
    reset (void) noexcept = 0;

    virtual
    std::vector<std::pair<nonnull_ptr<ir_block>, optional_ref<ir_def_timeline>>>
    collect_outgoing (ir_variable& var);

    [[nodiscard]]
    virtual
    std::list<nonnull_ptr<ir_def>>
    get_latest_defs (ir_variable& var) noexcept;

    //
    // virtual function accessories
    //

    [[nodiscard]]
    const ir_block&
    get_entry_block (void) const noexcept
    {
      return const_cast<ir_component *> (this)->get_entry_block ();
    }

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
    return get_function (const_cast<ir_component&> (c));
  }

}

#endif

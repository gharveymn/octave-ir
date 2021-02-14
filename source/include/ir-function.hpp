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

#if !defined(ir_function_h)
#define ir_function_h 1

#include "ir-structure.hpp"
#include "ir-component-sequence.hpp"

namespace gch
{

  class ir_function
    : public ir_structure
  {
  public:

    ir_function            (void);
    ir_function            (const ir_function&)     = delete;
    ir_function            (ir_function&&) noexcept = default;
    ir_function& operator= (const ir_function&)     = delete;
    ir_function& operator= (ir_function&&) noexcept = delete;
    ~ir_function           (void) override          = default;

    [[nodiscard]]
    ptr
    get_body (void) noexcept
    {
      return ptr { &m_body };
    }

    [[nodiscard]]
    cptr
    get_body (void) const noexcept
    {
      return as_mutable (*this).get_body ();
    }

    [[nodiscard]]
    bool
    is_body (cptr comp) const noexcept
    {
      return comp == get_body ();
    }

    [[nodiscard]]
    ir_component_sequence&
    get_body_component (void) noexcept
    {
      return get_as<ir_component_sequence> (get_body ());
    }

    [[nodiscard]]
    const ir_component_sequence&
    get_body_component (void) const noexcept
    {
      return as_mutable (*this).get_body_component ();
    }

    //
    // virtual from ir_component
    //

    bool
    reassociate_timelines (const ir_link_set<ir_def_timeline>& old_dts,
                           ir_def_timeline& new_dt,
                           std::vector<nonnull_ptr<ir_block>>& until) override;

    void
    reset (void) noexcept override;

    //
    // virtual from ir_structure
    //

    [[nodiscard]]
    ir_component_ptr
    get_ptr (ir_component&) const noexcept override;

    [[nodiscard]]
    ir_component_ptr
    get_entry_ptr (void) noexcept override;

    [[nodiscard]]
    ir_link_set<ir_block>
    get_predecessors (ir_component_cptr) noexcept override;

    [[nodiscard]]
    ir_link_set<ir_block>
    get_successors (ir_component_cptr) noexcept override;

    [[nodiscard]]
    bool
    is_leaf (ir_component_cptr) noexcept override;

    void
    generate_leaf_cache (void) override;

    ir_use_timeline&
    join_incoming_at (ir_component_ptr pos, ir_def_timeline& dt) override;

    void
    recursive_flatten (void) override;

  private:
    ir_component_storage m_body;
  };

}

#endif

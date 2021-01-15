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

#include "ir-common-util.hpp"

#include <gch/optional_ref.hpp>
#include <gch/nonnull_ptr.hpp>
#include <gch/variant-iterator.hpp>

#include <list>
#include <type_traits>
#include <memory>
#include <vector>

namespace gch
{

  class ir_basic_block;
  class ir_function;
  class ir_def;
  class ir_variable;
  class ir_structure;
  class ir_def_timeline;

  // abstract
  class ir_component
  {
  public:
  
    using component_list  = std::list<std::unique_ptr<ir_component>>;
    using comp_iter       = component_list::iterator;
    using comp_citer      = component_list::const_iterator;
    using comp_riter      = component_list::reverse_iterator;
    using comp_criter     = component_list::const_reverse_iterator;
    using comp_ref        = component_list::reference;
    using comp_cref       = component_list::const_reference;
    
    using component_handle = variant_iterator<comp_iter, value_iter<std::unique_ptr<ir_component>>>;

    using link_cache_vect  = std::vector<nonnull_ptr<ir_basic_block>>;
    using link_cache_iter  = link_cache_vect::iterator;
    using link_cache_citer = link_cache_vect::const_iterator;

    using link_iter  = variant_iterator<link_cache_iter,
                                        value_iter<nonnull_ptr<ir_basic_block>>,
                                        void_iter<nonnull_ptr<ir_basic_block>>>;
    
    
    using link_citer = variant_iterator<link_cache_citer,
                                        value_citer<nonnull_ptr<ir_basic_block>>,
                                        void_citer<nonnull_ptr<ir_basic_block>>>;

    ir_component            (void)                    = delete;
    ir_component            (const ir_component&)     = default;
    ir_component            (ir_component&&) noexcept = default;
    ir_component& operator= (const ir_component&)     = default;
    ir_component& operator= (ir_component&&) noexcept = default;
    virtual ~ir_component (void) noexcept             = 0;
    
    constexpr explicit ir_component (ir_structure& s)
      : m_parent (s)
    { }
  
    constexpr explicit ir_component (nullopt_t)
      : m_parent (nullopt)
    { }
  
    virtual void reset (void) noexcept = 0;

    [[nodiscard]] virtual link_iter leaf_begin (void) = 0;
    [[nodiscard]] virtual link_iter leaf_end   (void) = 0;

    [[nodiscard]] virtual ir_function& get_function (void) noexcept = 0;
    
    [[nodiscard]]
    const ir_function& get_function (void) const noexcept
    {
      return const_cast<ir_component *> (this)->get_function ();
    }

    [[nodiscard]] virtual ir_basic_block& get_entry_block (void) noexcept = 0;
  
    [[nodiscard]]
    const ir_basic_block& get_entry_block (void) const noexcept
    {
      return const_cast<ir_component *> (this)->get_entry_block ();
    }
    
    virtual std::vector<std::pair<nonnull_ptr<ir_basic_block>, optional_ref<ir_def_timeline>>>
    collect_outgoing (ir_variable& var);
    
    [[nodiscard]]
    virtual std::list<nonnull_ptr<ir_def>> get_latest_defs (ir_variable& var) noexcept = 0;
    
    [[nodiscard]] constexpr       bool  has_parent (void) const noexcept { return bool (m_parent); }
    [[nodiscard]] constexpr       auto& get_parent (void)       noexcept { return *m_parent;       }
    [[nodiscard]] constexpr const auto& get_parent (void) const noexcept { return *m_parent;       }

    template <typename T>
    using is_component = std::is_base_of<ir_component, T>;

  private:
    optional_ref<ir_structure> m_parent;
  };

}

#endif

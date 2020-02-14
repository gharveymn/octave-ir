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

#include <optional_ref.hpp>
#include <nonnull_ptr.hpp>
#include <variant-iterator.hpp>

#include <type_traits>
#include <vector>

#include "ir-common.hpp"

namespace gch
{

  class ir_basic_block;
  class ir_function;

  // abstract
  class ir_component
  {
  public:

    using link_cache_vect  = std::vector<nonnull_ptr<ir_basic_block>>;
    using link_cache_iter  = link_cache_vect::iterator;
    using link_cache_citer = link_cache_vect::const_iterator;

    using link_iter  = variant_iterator<link_cache_iter,
                                        value_iter<nonnull_ptr<ir_basic_block>>,
                                        void_iter<nonnull_ptr<ir_basic_block>>>;
    
    
    using link_citer = variant_iterator<link_cache_citer,
                                        value_citer<nonnull_ptr<ir_basic_block>>,
                                        void_citer<nonnull_ptr<ir_basic_block>>>;

    ir_component            (void)                    = default;
    ir_component            (const ir_component&)     = default;
    ir_component            (ir_component&&) noexcept = default;
    ir_component& operator= (const ir_component&)     = default;
    ir_component& operator= (ir_component&&) noexcept = default;
    
    virtual ~ir_component (void) noexcept = 0;
  
    virtual void reset (void) noexcept = 0;

    [[nodiscard]] virtual link_iter          leaf_begin      (void)                = 0;
    [[nodiscard]] virtual link_iter          leaf_end        (void)                = 0;

    [[nodiscard]] virtual ir_function&       get_function    (void)       noexcept = 0;
    [[nodiscard]] virtual const ir_function& get_function    (void) const noexcept = 0;

    [[nodiscard]] virtual ir_basic_block&    get_entry_block (void)       noexcept = 0;

    template <typename T>
    using is_component = std::is_base_of<ir_component, T>;

  private:

  };

}

#endif

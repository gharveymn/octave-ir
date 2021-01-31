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
#include <optional>

namespace gch
{

  class ir_function : public ir_sequence
  {
  public:

//  ir_function            (void)                   = impl;
    ir_function            (const ir_function&)     = delete;
    ir_function            (ir_function&&) noexcept = default;
    ir_function& operator= (const ir_function&)     = delete;
    ir_function& operator= (ir_function&&) noexcept = delete;
    ~ir_function           (void) override          = default;

    ir_function (void)
      : ir_sequence (nullopt)
    { }

    void invalidate_leaf_cache (void) noexcept override
    {
      clear_leaf_cache ();
    }

    [[nodiscard]]
    ir_function& get_function (void) noexcept override
    {
      return *this;
    }

  private:
  };

}

#endif

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

#if !defined(ir_module_h)
#define ir_module_h 1

#include "octave-config.h"
#include "ir-structure.h"

namespace octave
{
  class ir_module
  {
  public:

    ir_module (void);

    ir_basic_block& get_entry_block (void);

  private:
    ir_component_sequence m_super_sequence;
    ir_basic_block& m_entry_block;
  };

}

#endif

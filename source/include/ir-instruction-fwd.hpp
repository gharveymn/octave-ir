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

#if ! defined (ir_instruction_fwd_h)
#define ir_instruction_fwd_h 1

#include "ir-common.hpp"
#include "ir-type-base.hpp"
#include <plf_list.h>
#include <list>

namespace gch
{
  // fwds

  class ir_basic_block;
  class ir_instruction;
  class ir_def;
  class ir_use;

  using ir_instruction_container = std::list<ir_instruction>;
  using ir_instruction_iter      = ir_instruction_container::iterator;
  using ir_instruction_citer     = ir_instruction_container::const_iterator;
  using ir_instruction_riter     = ir_instruction_container::reverse_iterator;
  using ir_instruction_criter    = ir_instruction_container::const_reverse_iterator;

}

#endif

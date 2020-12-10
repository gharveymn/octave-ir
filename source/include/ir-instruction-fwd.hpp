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

namespace plf
{
  template <typename T, typename A>
  class list;
}

namespace gch
{
  // fwds

  class ir_basic_block;
  class ir_instruction;
  class ir_def;
  class ir_use;

}

#endif

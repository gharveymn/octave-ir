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

  class ir_instruction;
    class ir_def_instruction;
      class ir_assign; // assign some variable (x = y)
      class ir_fetch;
      class ir_convert;
      class ir_phi;
    class ir_call;   // call a function
    class ir_branch;
    class ir_cbranch;
    class ir_define; // define a variable (used on entry)
    class ir_operator;
    class ir_unop;
    class ir_biop;
    class ir_phi;
    class ir_relation;
    class ir_rel_eq;
    class ir_rel_ne;
    class ir_rel_lt;
    class ir_rel_le;
    class ir_rel_gt;
    class ir_rel_ge;
    class ir_operation;
    class ir_biop;
    class ir_add;

  class ir_basic_block;

  class ir_def;
  class ir_use;

  // typedefs

  using ir_block_ref = ir_constant<ir_basic_block *>;
  using ir_def_ref   = ir_constant<ir_def *>;

  using instr_list_type = std::unique_ptr<ir_instruction>;
  using instr_list      = std::list<instr_list_type>;

}

#endif

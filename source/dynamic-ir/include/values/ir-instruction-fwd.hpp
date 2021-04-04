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

#include "gch/octave-ir-utilities/ir-common.hpp"
#include <list>

namespace gch
{

  class ir_block;
  class ir_instruction;
  class ir_def;
  class ir_use;

  using ir_instruction_container               = std::list<ir_instruction>;
  using ir_instruction_value_type              = ir_instruction_container::value_type;
  using ir_instruction_allocator_type          = ir_instruction_container::allocator_type;
  using ir_instruction_size_type               = ir_instruction_container::size_type;
  using ir_instruction_difference_type         = ir_instruction_container::difference_type;
  using ir_instruction_reference               = ir_instruction_container::reference;
  using ir_instruction_const_reference         = ir_instruction_container::const_reference;
  using ir_instruction_pointer                 = ir_instruction_container::pointer;
  using ir_instruction_const_pointer           = ir_instruction_container::const_pointer;

  using ir_instruction_iterator                = ir_instruction_container::iterator;
  using ir_instruction_const_iterator          = ir_instruction_container::const_iterator;
  using ir_instruction_reverse_iterator        = ir_instruction_container::reverse_iterator;
  using ir_instruction_const_reverse_iterator  = ir_instruction_container::const_reverse_iterator;

  using ir_instruction_val_t   = ir_instruction_value_type;
  using ir_instruction_alloc_t = ir_instruction_allocator_type;
  using ir_instruction_size_ty = ir_instruction_size_type;
  using ir_instruction_diff_ty = ir_instruction_difference_type;
  using ir_instruction_ref     = ir_instruction_reference;
  using ir_instruction_cref    = ir_instruction_const_reference;
  using ir_instruction_ptr     = ir_instruction_pointer;
  using ir_instruction_cptr    = ir_instruction_const_pointer;

  using ir_instruction_iter    = ir_instruction_iterator;
  using ir_instruction_citer   = ir_instruction_const_iterator;
  using ir_instruction_riter   = ir_instruction_reverse_iterator;
  using ir_instruction_criter  = ir_instruction_const_reverse_iterator;

}

#endif

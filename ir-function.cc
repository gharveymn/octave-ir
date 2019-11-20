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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "ir-function.h"

namespace octave
{
  ir_component::link_iter
  ir_function::pred_begin (ir_component *c)
  {
    comp_citer cit = must_find (c);
    if (cit == begin ())
      return link_iter (nullptr);
    return link_iter ((*--cit)->leaf_begin ());
  }
  
  ir_component::link_iter
  ir_function::pred_end (ir_component *c)
  {
    comp_citer cit = must_find (c);
    if (cit == begin ())
      return link_iter (nullptr);
    return ++link_iter ((*--cit)->leaf_end ());
  }
  
  ir_component::link_iter
  ir_function::succ_begin (ir_component *c)
  {
    comp_citer cit = must_find (c);
    if (cit == last ())
      return link_iter (nullptr);
    return link_iter ((*++cit)->get_entry_block ());
  }
  
  ir_component::link_iter
  ir_function::succ_end (ir_component *c)
  {
    comp_citer cit = must_find (c);
    if (cit == last ())
      return link_iter (nullptr);
    return ++link_iter ((*++cit)->get_entry_block ());
  }
  
}

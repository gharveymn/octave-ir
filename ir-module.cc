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

#include "ir-module.h"

namespace octave
{
  ir_module::ir_module (void)
    : m_super_sequence (*this),
      m_entry_block (m_super_sequence.emplace_back<ir_basic_block> ())
  {
    m_super_sequence.emplace_back<ir_basic_block> ();
  }
  
  ir_basic_block&
  ir_module::get_entry_block (void)
  {
    return m_entry_block;
  }
  
}

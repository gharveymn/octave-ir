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

#include "ir-block.hpp"
#include "ir-instruction.hpp"
#include "ir-variable.hpp"
#include <algorithm>
#include <list>

namespace gch
{
  
  //
  // ir_operand_precursor
  //
  
  ir_operand
  ir_operand_pre::construct (ir_instruction& instr)
  {
    return std::visit (overloaded
                       {
                         [&instr] (const use_pair& ut) -> ir_operand
                         {
                           return { instr, *std::get<nonnull_ptr<ir_use_timeline>> (ut),
                                    std::get<ir_use_timeline::citer> (ut) };
                         },
                         [](ir_constant&& c) -> ir_operand { return { std::move (c) }; }
                       }, std::move (m_data));
  }
  
  //
  // ir_instruction
  //

}

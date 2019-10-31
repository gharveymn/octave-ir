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

#if ! defined (ir_walker_h)
#define ir_walker_h 1

#include "octave-config.h"

namespace octave
{

  class ir_instruction;

  class ir_visitor
  {
  public:
    virtual void visit (ir_block&) = 0;
    virtual void visit (ir_assign&) = 0;
    virtual void visit (ir_extract_argument&) = 0;
    virtual void visit (ir_phi&) = 0;
    virtual void visit (ir_call&) = 0;
    virtual void visit (ir_magic_end&) = 0;
    virtual void visit (ir_store_argument&) = 0;
    virtual void visit (ir_jump&) = 0;
    virtual void visit (ir_cond_jump&) = 0;
    virtual void visit (ir_error_check&) = 0;

// linear walk over the blocks and instructions
    void walk (ir_module& mod)
    {
      for (ir_block& bl : mod)
        {
          visit (bl);
          for (ir_block::const_reference instr : bl)
            {
              instr.get ()->accept (*this);
            }
        }
    }

// linear walk over blocks
    void walk_blocks (ir_module& mod)
    {
      for (ir_block& bl : mod)
        {
          visit (bl);
        }
    }

// linear walk over instructions
    void walk_instrs (ir_module& mod)
    {
      for (ir_block& bl : mod)
        {
          for (const std::unique_ptr<ir_instruction>& instr : bl)
            {
              instr->accept (*this);
            }
        }
    }

// depth-first walk over blocks and instructions

// depth-first walk over blocks

// depth-first walk over instructions

// breadth-first walk over blocks and instructions

// breadth-first walk over blocks

// breadth-first walk over instructions

  };

}

#endif

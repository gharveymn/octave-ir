/** test-lor.cpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "test-templates.hpp"

using namespace gch;

int
main (void)
{
  try
  {
    test_binary<ir_opcode::lor> (false, 0, 0);
    test_binary<ir_opcode::lor> (true,  1, 0);
    test_binary<ir_opcode::lor> (true,  0, 1);
    test_binary<ir_opcode::lor> (true,  1, 1);

    test_binary<ir_opcode::lor> (false, 0b00, 0b00);
    test_binary<ir_opcode::lor> (true,  0b10, 0b00);
    test_binary<ir_opcode::lor> (true,  0b00, 0b10);
    test_binary<ir_opcode::lor> (true,  0b10, 0b10);

    test_binary<ir_opcode::lor> (false, 0.F, 0.F);
    test_binary<ir_opcode::lor> (true,  1.F, 0.F);
    test_binary<ir_opcode::lor> (true,  0.F, 1.F);
    test_binary<ir_opcode::lor> (true,  1.F, 1.F);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what () << std::endl;
    return 1;
  }

  return 0;
}

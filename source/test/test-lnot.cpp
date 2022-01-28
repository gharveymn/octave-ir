/** test-lnot.cpp
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
    test_unary<ir_opcode::lnot> (true, 0);
    test_unary<ir_opcode::lnot> (false, 1);

    test_unary<ir_opcode::lnot> (true, 0b00);
    test_unary<ir_opcode::lnot> (false, 0b10);

    test_unary<ir_opcode::lnot> (true, 0.F);
    test_unary<ir_opcode::lnot> (false, 1.F);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what () << std::endl;
    return 1;
  }

  return 0;
}

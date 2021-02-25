/** ir-error.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "utilities/ir-error.hpp"

#include <cstring>
#include <iostream>

namespace gch
{

  ir_exception::
  ~ir_exception (void) = default;

  namespace abort
  {

    void
    ir_impossible (const char *message)
    {
      std::cerr << message << std::endl;
      std::abort ();
    }

    void
    ir_logic_error (const char *message)
    {
      if (std::strlen (message) != 0)
        std::cerr << message << "\n\n";
      std::cerr << ir_logic_error_message << std::endl;
      std::abort ();
    }

  }

}

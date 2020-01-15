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


#if ! defined (ir_type_std_h)
#define ir_type_std_h 1

#include "ir-type-base.hpp"
#include <string>

namespace gch
{

  template <>
  struct ir_type::instance<std::string>
  {
    using type = std::string;
    static constexpr
    impl m_impl = create_type<type> ("string");
  };

}

#endif

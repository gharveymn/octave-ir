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

#include "gch/octave-static-ir/ir-type-base.hpp"

#include <complex>

namespace gch
{

  template struct ir_type::instance<long double *>;
  template struct ir_type::instance<double *>;
  template struct ir_type::instance<single *>;
  template struct ir_type::instance<int64_t *>;
  template struct ir_type::instance<int32_t *>;
  template struct ir_type::instance<int16_t *>;
  template struct ir_type::instance<int8_t *>;
  template struct ir_type::instance<uint64_t *>;
  template struct ir_type::instance<uint32_t *>;
  template struct ir_type::instance<uint16_t *>;
  template struct ir_type::instance<uint8_t *>;
  template struct ir_type::instance<char *>;
  template struct ir_type::instance<const char *>;
  template struct ir_type::instance<wchar_t *>;
  template struct ir_type::instance<char32_t *>;
  template struct ir_type::instance<char16_t *>;
#if defined (__cpp_char8_t) && __cpp_char8_t >= 201811L
  template struct ir_type::instance<char8_t *>;
#endif
  template struct ir_type::instance<bool *>;
  template struct ir_type::instance<std::complex<double> *>;
  template struct ir_type::instance<std::complex<single> *>;
}

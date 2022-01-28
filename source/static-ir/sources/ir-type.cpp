/** ir-type.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-type.hpp"

namespace gch
{
  /////////////////////////
  // type instantiations //
  /////////////////////////

  template struct ir_type_base::instance<long double *>;
  template struct ir_type_base::instance<double *>;
  template struct ir_type_base::instance<single *>;
  template struct ir_type_base::instance<int64_t *>;
  template struct ir_type_base::instance<int32_t *>;
  template struct ir_type_base::instance<int16_t *>;
  template struct ir_type_base::instance<int8_t *>;
  template struct ir_type_base::instance<uint64_t *>;
  template struct ir_type_base::instance<uint32_t *>;
  template struct ir_type_base::instance<uint16_t *>;
  template struct ir_type_base::instance<uint8_t *>;
  template struct ir_type_base::instance<char *>;
  template struct ir_type_base::instance<wchar_t *>;
  template struct ir_type_base::instance<char32_t *>;
  template struct ir_type_base::instance<char16_t *>;
  template struct ir_type_base::instance<bool *>;
  template struct ir_type_base::instance<std::complex<double> *>;
  template struct ir_type_base::instance<std::complex<single> *>;

#if 0

  // prints a pointer like "double**"
  template <>
  std::ostream&
  ir_printer<ir_type>::short_print (std::ostream& os, const ir_type& ty)
  {
    return os << ty.get_name_base () << std::string (indirection_level (ty), '*');
  }

  template <>
  std::ostream&
  ir_printer<ir_type>::long_print (std::ostream& os, const ir_type& ty)
  {
    const ir_type_array& members = ty.get_members ();
    short_print (os, ty);
    if (members.get_numel () > 0)
    {
      short_print (os << " {", members[0]);
      for (std::size_t i = 1; i < members.get_numel (); i++)
        short_print (os << ", ", members[i]);
      short_print (os << "}", members[0]);
    }
    return os;
  }

  std::ostream& operator<< (std::ostream& os, const ir_type& ty)
  {
    return ir_printer<ir_type>::short_print (os, ty);
  }

#endif

}

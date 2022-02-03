/** ir-external-function-info.hpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_EXTERNAL_FUNCTION_INFO_HPP
#define OCTAVE_IR_IR_EXTERNAL_FUNCTION_INFO_HPP

#include "ir-utility.hpp"

#include <iosfwd>
#include <string>
#include <string_view>

namespace gch
{

  class ir_external_function_info
  {
  public:
    struct variadic_type
      : transparent_named_type<variadic_type, bool>
    { };

    template <bool B>
    static constexpr variadic_type variadic { B };

    ir_external_function_info            (void)                                 = delete;
    ir_external_function_info            (const ir_external_function_info&)     = default;
    ir_external_function_info            (ir_external_function_info&&) noexcept = default;
    ir_external_function_info& operator= (const ir_external_function_info&)     = default;
    ir_external_function_info& operator= (ir_external_function_info&&) noexcept = default;
    ~ir_external_function_info           (void)                                 = default;

    ir_external_function_info (std::string_view name, variadic_type is_variadic);
    ir_external_function_info (std::string_view name);

    [[nodiscard]]
    std::string_view
    get_name (void) const noexcept;

    [[nodiscard]]
    bool
    is_variadic (void) const noexcept;

  private:
    std::string   m_name;
    variadic_type m_is_variadic = variadic<false>;
  };

  std::ostream&
  operator<< (std::ostream& out, const ir_external_function_info& info);

}

#endif // OCTAVE_IR_IR_EXTERNAL_FUNCTION_INFO_HPP

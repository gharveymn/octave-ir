/** ir-constant.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-constant.hpp"

#include "ir-type-util.hpp"

#include <ostream>
#include <iostream>
#include <iomanip>

namespace gch
{

  template <typename T>
  inline constexpr
  bool
  is_wide_char_v = std::is_same_v<T, wchar_t>
               ||  std::is_same_v<T, char32_t>
               ||  std::is_same_v<T, char16_t>;

  template <typename T>
  inline constexpr
  bool
  is_char_v = is_wide_char_v<T> || std::is_same_v<T, char>;

  template <typename CharT, typename Traits, typename CharIter>
  std::basic_ostream<CharT, Traits>&
  print_escaped (std::basic_ostream<CharT, Traits>& out,
                 CharIter first,
                 CharT surround = CharT ('"'))
  {
    constexpr std::pair<CharT, const CharT *> escaped[] {
      { '\a', R"(\a)" },
      { '\b', R"(\b)" },
      { '\t', R"(\t)" },
      { '\n', R"(\n)" },
      { '\v', R"(\v)" },
      { '\f', R"(\f)" },
      { '\r', R"(\r)" },
      { '\"', R"(\")" },
      { '\\', R"(\\)" },
    };

    std::basic_string<CharT, Traits> str;
    str.push_back (surround);
    while (auto c = static_cast<CharT> (*first++))
    {
      auto found = std::lower_bound (std::begin (escaped), std::end (escaped), c,
                                     [](const std::pair<CharT, const CharT *>& lhs, CharT rhs) {
                                       return Traits::lt (lhs.first, rhs);
                                     });
      if (found != std::end (escaped) && Traits::eq (found->first, c))
        str.append (found->second);
      else
      {
        if (Traits::eq (surround, c))
          str.push_back ('\\');
        str.push_back (c);
      }
    }
    str.push_back (surround);
    return out << str;
  }

  // FIXME: We probably shouldn't just be casting wide character types to char.

  template <typename T>
  struct constant_printer
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_constant& c)
    {
      if constexpr (is_char_v<T>)
      {
        std::array<T, 2> str { as<T> (c), 0 };
        return print_escaped (out, str.begin (), '\'');
      }
      else
        return out << as<T> (c);
    }
  };

  template <typename T>
  struct constant_printer<T *>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_constant& c)
    {
      if constexpr (is_char_v<T>)
        return print_escaped (out, as<T *> (c));
      else
        return out << as<T *> (c);
    }
  };

  template <>
  struct constant_printer<std::string>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_constant& c)
    {
      return print_escaped (out, as<std::string> (c).c_str ());
    }
  };

  template <>
  struct constant_printer<void>
  {
    static
    std::ostream&
    print (std::ostream&, const ir_constant&)
    {
      throw std::ios_base::failure { "Cannot print a constant of type `void`." };
    }
  };

  template <>
  struct constant_printer<ir_block_id>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_constant& c)
    {
      return out << "BLOCK" << static_cast<std::size_t> (as<ir_block_id> (c));
    }
  };

  template <typename T>
  struct printer_mapper
  {
    constexpr
    auto
    operator() (void) const noexcept
    {
      return constant_printer<T>::print;
    }
  };

  std::ostream&
  operator<< (std::ostream& out, const ir_constant& c)
  {
    constexpr auto map = generate_ir_type_map<printer_mapper> ();
    return std::invoke (map[c.get_type ()], out, c);
  }

}

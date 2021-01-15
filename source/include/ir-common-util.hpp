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

#if ! defined (ir_common_util_h)
#define ir_common_util_h 1

#include "ir-common.hpp"

#include <gch/optional_ref.hpp>
#include <gch/nonnull_ptr.hpp>

#include <iosfwd>
#include <memory>
#include <functional>
#include <unordered_set>
#include <type_traits>
#include <optional>

namespace gch
{
  class ir_type;

  template <typename T>
  struct ir_printer
  {
    using ir_class = T;

    static std::ostream& short_print (std::ostream& os, const ir_class&);

    static std::ostream& long_print (std::ostream& os, const ir_class&);

  };

  std::ostream& operator<< (std::ostream& os, const ir_type& ty);

  template <typename T>
  using is_pointer_ref = std::is_pointer<typename std::remove_reference<T>::type>;

  template <typename T, typename S>
  constexpr bool is_a (const S* x)
  {
    return dynamic_cast<const T*> (x) != nullptr;
  }

  template <typename T, typename S,
        typename std::enable_if<! std::is_pointer<S>::value>::type * = nullptr>
  constexpr bool is_a (const S& x)
  {
    return dynamic_cast<const T*> (&x) != nullptr;
  }

  template <typename T, typename S,
          typename std::enable_if<std::is_pointer<S>::value>::type * = nullptr>
  constexpr bool is_a (const S& x)
  {
    return dynamic_cast<const T*> (x) != nullptr;
  }

  template <typename ...Ts>
  struct overloaded: Ts...
  {
    using Ts::operator()...;
  };
  template <typename ...Ts> overloaded (Ts...) -> overloaded<Ts...>;

  template <typename R, typename Overloaded>
  struct overloaded_ret;

  template <typename R, typename ...Ts>
  struct overloaded_ret<R, overloaded<Ts...>>
    : overloaded<Ts...>
  {
    constexpr explicit overloaded_ret (Ts&&... ts)
      : overloaded<Ts...> (std::forward<Ts> (ts)...)
    { }

    template <typename T>
    R operator() (T&& t)
    {
      return overloaded<Ts...>::operator() (std::forward<T> (t));
    }
  };

  template <typename R, typename ...Ts>
  constexpr overloaded_ret<R, overloaded<Ts...>> make_overloaded (Ts&&... ts)
  {
    return overloaded_ret<R, overloaded<Ts...>> { std::forward<Ts> (ts)... };
  }

  template <typename Optional, typename Function>
  constexpr auto operator>>= (Optional&& opt, Function&& f)
    -> std::enable_if_t<std::is_invocable_v<Function, decltype (*opt)>,
                        decltype (std::invoke (std::forward<Function> (f), *opt))>
  {
    using ret_type = decltype (std::invoke (std::forward<Function> (f), *opt));
    if (opt)
      return std::invoke (std::forward<Function> (f), *opt);
    return ret_type ();
  }

  // nonconst member function
  template <typename Optional, typename Ret>
  constexpr auto
  operator>>= (Optional&& opt,
               Ret (std::decay_t<decltype (*std::declval<Optional> ())>::*f) (void))
  -> std::enable_if_t<std::is_invocable_v<decltype (f), decltype (*opt)>
                  &&! std::is_const_v<decltype (*opt)>,
                      decltype (std::invoke (f, *opt))>
  {
    using ret_type = decltype (std::invoke (f, *opt));
    if (opt)
      return std::invoke (f, *opt);
    return ret_type ();
  }

  // const member function
  template <typename Optional, typename Ret>
  constexpr auto
  operator>>= (Optional&& opt,
               Ret (std::decay_t<decltype (*std::declval<Optional> ())>::*f) (void) const)
    -> std::enable_if_t<std::is_invocable_v<decltype (f), decltype (*opt)>
                    &&  std::is_const_v<decltype (*opt)>,
                        decltype (std::invoke (f, *opt))>
  {
    using ret_type = decltype (std::invoke (f, *opt));
    if (opt)
      return std::invoke (f, *opt);
    return ret_type ();
  }

  template <typename Function>
  struct maybe
  {
    template <typename ...Args>
    decltype (auto) operator() (Args&&... args)
    {
      return std::make_optional (std::invoke (m_func, std::forward<Args> (args)...));
    }
    Function m_func;
  };

  template <typename Function> maybe (Function&&) -> maybe<Function>;

  template <typename T, typename Function>
  struct selected
  {
    template <typename ...Args>
    decltype (auto) operator() (Args&&... args)
    {
      return std::invoke (m_func, std::get<T> (std::forward<Args> (args))...);
    }
    Function m_func;
  };

  template <typename T, typename Function>
  selected<T, std::decay_t<Function>> make_selected (Function&& f)
  {
    return selected<T, std::decay_t<Function>> { std::forward<Function> (f) };
  }

  template <typename Function>
  struct applied
  {
    template <typename Tuple>
    decltype (auto) operator() (Tuple&& tup)
    {
      return std::apply (m_func, std::forward<Tuple> (tup));
    }

    Function m_func;
  };

  template <typename Function> applied (Function&&) -> applied<Function>;

  template <typename From, typename To>
  struct match_cv
  {
    using type = typename std::conditional<std::is_const<From>::value,
                   typename std::conditional<std::is_volatile<From>::value,
                                             typename std::add_cv<To>::type,
                                             typename std::add_const<To>::type>::type,
                   typename std::conditional<std::is_volatile<From>::value,
                                             typename std::add_volatile<To>::type,
                                             To>::type>::type;
  };

  template <typename From, typename To>
  using match_cv_t = typename match_cv<From, To>::type;

  template <typename ...Ts>
  using ref_tuple = std::tuple<nonnull_ptr<Ts>...>;

}

#endif

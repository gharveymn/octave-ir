/** ir-optional-util.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_OPTIONAL_UTIL_HPP
#define OCTAVE_IR_IR_OPTIONAL_UTIL_HPP

#include <gch/optional_ref.hpp>

#include <optional>
#include <type_traits>

namespace gch
{
  namespace detail
  {

    template <typename Optional>
    using just_t = decltype (*std::declval<Optional> ());

    template <typename Optional, typename Functor, typename ...Args>
    constexpr
    auto
    maybe_invoke_impl (Optional&& opt, Functor&& f, Args&&... args)
      -> std::enable_if_t<
               std::is_object_v<
                 std::invoke_result_t<Functor, just_t<Optional>, Args...>>
           &&  std::is_default_constructible_v<
                 std::invoke_result_t<Functor, just_t<Optional>, Args...>>,
               std::invoke_result_t<Functor, just_t<Optional>, Args...>>
    {
      using ret_type = std::invoke_result_t<Functor, just_t<Optional>, Args...>;
      if (opt)
        return std::invoke (std::forward<Functor> (f), *std::forward<Optional> (opt),
                            std::forward<Args> (args)...);
      return ret_type ();
    }

    template <typename Optional, typename Functor, typename ...Args>
    constexpr
    auto
    maybe_invoke_impl (Optional&& opt, Functor&& f, Args&&... args)
      -> std::enable_if_t<
             ! std::is_object_v<
                 std::invoke_result_t<Functor, just_t<Optional>, Args...>>
           &&  std::is_lvalue_reference_v<
                 std::invoke_result_t<Functor, just_t<Optional>, Args...>>,
           optional_ref<std::remove_reference_t<
             std::invoke_result_t<Functor, just_t<Optional>, Args...>>>>
    {
      using ret_type = optional_ref<std::remove_reference_t<
        std::invoke_result_t<Functor, just_t<Optional>, Args...>>>;

      if (opt)
        return ret_type { std::invoke (std::forward<Functor> (f), *std::forward<Optional> (opt),
                                       std::forward<Args> (args)...) };
      return nullopt;
    }

    template <typename Optional, typename Functor, typename ...Args>
    constexpr
    auto
    maybe_invoke_impl (Optional&& opt, Functor&& f, Args&&... args)
      -> std::enable_if_t<
             ! std::is_object_v<
                 std::invoke_result_t<Functor, just_t<Optional>, Args...>>
           &&! std::is_lvalue_reference_v<
                 std::invoke_result_t<Functor, just_t<Optional>, Args...>>,
               void>
    {
      if (opt)
        std::invoke (std::forward<Functor> (f), *std::forward<Optional> (opt),
                     std::forward<Args> (args)...);
    }

    template <typename Void, typename Optional, typename Functor, typename ...Args>
    struct maybe_invoke_result_impl
    { };

    template <typename Optional, typename Functor, typename ...Args>
    struct maybe_invoke_result_impl<
      decltype (static_cast<void> (maybe_invoke_impl (std::declval<Optional> (),
                                                      std::declval<Functor> (),
                                                      std::declval<Args> ()...))),
      Optional, Functor, Args...>
    {
      using type = decltype (maybe_invoke_impl (std::declval<Optional> (),
                                                std::declval<Functor> (),
                                                std::declval<Args> ()...));
    };

    template <typename Void, typename Optional, typename Functor, typename ...Args>
    struct is_maybe_invocable_impl
      : std::false_type
    { };

    template <typename Optional, typename Functor, typename ...Args>
    struct is_maybe_invocable_impl<
          std::void_t<typename maybe_invoke_result_impl<void, Optional, Functor, Args...>::type>,
          Optional, Functor, Args...>
      : std::true_type
    { };

  }

  template <typename Optional, typename Function, typename ...Args>
  struct maybe_invoke_result
    : detail::maybe_invoke_result_impl<void, Optional, Function, Args...>
  { };

  template <typename Optional, typename Function, typename ...Args>
  using maybe_invoke_result_t = typename maybe_invoke_result<Optional, Function, Args...>::type;

  template <typename Optional, typename Function, typename ...Args>
  struct is_maybe_invocable;

  template <typename Optional, typename Function, typename ...Args>
  struct is_maybe_invocable
    : detail::is_maybe_invocable_impl<void, Optional, Function, Args...>
  { };

  template <typename Optional, typename Functor, typename ...Args>
  std::enable_if_t<is_maybe_invocable_v<Optional, Functor, Args...>,
                   maybe_invoke_result_t<Optional, Functor, Args...>>
  maybe_invoke (Optional&& opt, Functor&& f, Args&&... args)
  {
    return detail::maybe_invoke_impl (std::forward<Optional> (opt),
                                      std::forward<Functor> (f),
                                      std::forward<Args> (args)...);
  }

  template <typename T, typename Function,
            typename = std::enable_if_t<is_maybe_invocable_v<std::optional<T>&, Function>>>
  constexpr
  maybe_invoke_result_t<std::optional<T>&, Function>
  operator>>= (std::optional<T>& opt, Function&& f)
  {
    return maybe_invoke (opt, std::forward<Function> (f));
  }

  template <typename T, typename Function,
            typename = std::enable_if_t<is_maybe_invocable_v<const std::optional<T>&, Function>>>
  constexpr
  maybe_invoke_result_t<const std::optional<T>&, Function>
  operator>>= (const std::optional<T>& opt, Function&& f)
  {
    return maybe_invoke (opt, std::forward<Function> (f));
  }

  template <typename T, typename Function,
            typename = std::enable_if_t<is_maybe_invocable_v<std::optional<T>&&, Function>>>
  constexpr
  maybe_invoke_result_t<std::optional<T>&&, Function>
  operator>>= (std::optional<T>&& opt, Function&& f)
  {
    return maybe_invoke (std::move (opt), std::forward<Function> (f));
  }

  template <typename T, typename Function,
            typename = std::enable_if_t<is_maybe_invocable_v<const std::optional<T>&&, Function>>>
  constexpr
  maybe_invoke_result_t<const std::optional<T>&&, Function>
  operator>>= (const std::optional<T>&& opt, Function&& f)
  {
    return maybe_invoke (std::move (opt), std::forward<Function> (f));
  }

  template <typename T>
  constexpr
  auto
  operator>>= (std::optional<T>& opt, void (* f) (T&))
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                        maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (opt, f);
  }

  template <typename T, typename Return>
  constexpr
  auto
  operator>>= (std::optional<T>& opt, Return (* f) (T&))
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                        maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (opt, f);
  }

  template <typename T, typename Base, typename Return,
            typename std::enable_if<
              std::is_base_of<Base, typename std::decay<T>::type>::value>::type * = nullptr>
  constexpr
  auto
  operator>>= (std::optional<T>& opt, Return (Base::* f) (void))
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                        maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (opt, f);
  }

  template <typename T, typename Base, typename Return,
            typename std::enable_if<
              std::is_base_of<Base, typename std::decay<T>::type>::value>::type * = nullptr>
  constexpr
  auto
  operator>>= (std::optional<T>& opt, Return (Base::* f) (void) &)
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                      maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (opt, f);
  }

  template <typename T>
  constexpr
  auto
  operator>>= (const std::optional<T>& opt, void (* f) (const T&))
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                        maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (opt, f);
  }

  template <typename T, typename Return>
  constexpr
  auto
  operator>>= (const std::optional<T>& opt, Return (* f) (const T&))
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                        maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (opt, f);
  }

  template <typename T, typename Base, typename Return,
            typename std::enable_if<
              std::is_base_of<Base, typename std::decay<T>::type>::value>::type * = nullptr>
  constexpr
  auto
  operator>>= (const std::optional<T>& opt, Return (Base::* f) (void) const)
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                      maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (opt, f);
  }

  template <typename T, typename Base, typename Return,
            typename std::enable_if<
              std::is_base_of<Base, typename std::decay<T>::type>::value>::type * = nullptr>
  constexpr
  auto
  operator>>= (const std::optional<T>& opt, Return (Base::* f) (void) const &)
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                    maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (opt, f);
  }

  template <typename T>
  constexpr
  auto
  operator>>= (std::optional<T>&& opt, void (* f) (T&&))
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                        maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (opt, f);
  }

  template <typename T, typename Return>
  constexpr
  auto
  operator>>= (std::optional<T>&& opt, Return (* f) (T&&))
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                      maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (std::move (opt), f);
  }

  template <typename T, typename Base, typename Return,
            typename std::enable_if<
              std::is_base_of<Base, typename std::decay<T>::type>::value>::type * = nullptr>
  constexpr
  auto
  operator>>= (std::optional<T>&& opt, Return (Base::* f) (void))
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                      maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (std::move (opt), f);
  }

  template <typename T, typename Base, typename Return,
            typename std::enable_if<
              std::is_base_of<Base, typename std::decay<T>::type>::value>::type * = nullptr>
  constexpr
  auto
  operator>>= (std::optional<T>&& opt, Return (Base::* f) (void) &&)
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                      maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (std::move (opt), f);
  }

  template <typename T>
  constexpr
  auto
  operator>>= (const std::optional<T>&& opt, void (* f) (const T&&))
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                        maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (opt, f);
  }

  template <typename T, typename Return>
  constexpr
  auto
  operator>>= (const std::optional<T>&& opt, Return (* f) (const T&&))
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                      maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (std::move (opt), f);
  }

  template <typename T, typename Base, typename Return,
            typename std::enable_if<
              std::is_base_of<Base, typename std::decay<T>::type>::value>::type * = nullptr>
  constexpr
  auto
  operator>>= (const std::optional<T>&& opt, Return (Base::* f) (void) const)
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                      maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (std::move (opt), f);
  }

  template <typename T, typename Base, typename Return,
            typename std::enable_if<
              std::is_base_of<Base, typename std::decay<T>::type>::value>::type * = nullptr>
  constexpr
  auto
  operator>>= (const std::optional<T>&& opt, Return (Base::* f) (void) const &&)
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                      maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (std::move (opt), f);
  }

}

#endif // OCTAVE_IR_IR_OPTIONAL_UTIL_HPP

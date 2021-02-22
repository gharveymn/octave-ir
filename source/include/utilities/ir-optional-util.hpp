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
    struct is_optional_impl
      : std::false_type
    { };

    template <typename T>
    struct is_optional_impl<std::optional<T>>
      : std::true_type
    { };

    template <typename T>
    struct is_optional_impl<gch::optional_ref<T>>
      : std::true_type
    { };

  }

  template <typename Optional>
  struct is_optional
    : detail::is_optional_impl<std::decay_t<Optional>>
  { };

  template <typename Optional>
  inline constexpr bool is_optional_v = is_optional<Optional>::value;

  namespace detail
  {

    template <typename Optional, typename Enable = void>
    struct just_impl
    { };

    template <typename Optional>
    struct just_impl<Optional, std::void_t<decltype (*std::declval<Optional> ())>>
    {
      using type = decltype (*std::declval<Optional> ());
    };

  }

  template <typename Optional>
  struct just
    : detail::just_impl<Optional>
  { };

  template <typename Optional>
  using just_t = typename just<Optional>::type;

  template <typename Functor>
  class maybe
  {
  public:
    maybe            (void)             = default;
    maybe            (const maybe&)     = default;
    maybe            (maybe&&) noexcept = default;
    maybe& operator= (const maybe&)     = default;
    maybe& operator= (maybe&&) noexcept = default;
    ~maybe           (void)             = default;

    constexpr explicit
    maybe (const Functor& functor)
      : m_functor (functor)
    { }

    constexpr explicit
    maybe (Functor&& functor)
      : m_functor (std::move (functor))
    { }

    template <typename ...Args,
              std::enable_if_t<std::is_lvalue_reference_v<
                std::invoke_result_t<Functor, Args...>>> * = nullptr>
    constexpr
    decltype (auto)
    operator() (Args&&... args)
    {
      return make_optional_ref (std::invoke (m_functor, std::forward<Args> (args)...));
    }

    template <typename ...Args,
              std::enable_if_t<! std::is_lvalue_reference_v<
                std::invoke_result_t<Functor, Args...>>> * = nullptr>
    constexpr
    decltype (auto)
    operator() (Args&&... args)
    {
      return std::make_optional (std::invoke (m_functor, std::forward<Args> (args)...));
    }

  private:
    Functor m_functor;
  };

  template <typename Functor>
  maybe (Functor&&) -> maybe<std::decay_t<Functor>>;

  namespace detail
  {

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
      return opt ? std::invoke (std::forward<Functor> (f), *std::forward<Optional> (opt),
                                std::forward<Args> (args)...)
                 : ret_type ();
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
      return opt ? ret_type (std::invoke (std::forward<Functor> (f), *std::forward<Optional> (opt),
                                          std::forward<Args> (args)...))
                 : ret_type ();
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
    {
      using nothrow = std::false_type;
    };

    template <typename Optional, typename Functor, typename ...Args>
    struct is_maybe_invocable_impl<
          std::void_t<typename maybe_invoke_result_impl<void, Optional, Functor, Args...>::type>,
          Optional, Functor, Args...>
      : std::true_type
    {
      using nothrow = std::bool_constant<noexcept (maybe_invoke_impl (std::declval<Optional> (),
                                                                      std::declval<Functor> (),
                                                                      std::declval<Args> ()...))>;
    };

  }

  template <typename Optional, typename Function, typename ...Args>
  struct maybe_invoke_result
    : detail::maybe_invoke_result_impl<void, Optional, Function, Args...>
  { };

  template <typename Optional, typename Function, typename ...Args>
  using maybe_invoke_result_t = typename maybe_invoke_result<Optional, Function, Args...>::type;

  template <typename Optional, typename Function, typename ...Args>
  struct is_maybe_invocable
    : detail::is_maybe_invocable_impl<void, Optional, Function, Args...>
  { };

  template <typename Optional, typename Function, typename ...Args>
  struct is_nothrow_maybe_invocable
    : detail::is_maybe_invocable_impl<void, Optional, Function, Args...>::nothrow
  { };

  template <typename Optional, typename Functor, typename ...Args>
  constexpr
  std::enable_if_t<is_maybe_invocable_v<Optional, Functor, Args...>,
                   maybe_invoke_result_t<Optional, Functor, Args...>>
  maybe_invoke (Optional&& opt, Functor&& f, Args&&... args)
    noexcept (is_nothrow_maybe_invocable_v<Optional, Functor, Args...>)
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
    noexcept (is_nothrow_maybe_invocable_v<decltype (opt), decltype (f)>)
  {
    return maybe_invoke (opt, std::forward<Function> (f));
  }

  template <typename T, typename Function,
            typename = std::enable_if_t<is_maybe_invocable_v<const std::optional<T>&, Function>>>
  constexpr
  maybe_invoke_result_t<const std::optional<T>&, Function>
  operator>>= (const std::optional<T>& opt, Function&& f)
    noexcept (is_nothrow_maybe_invocable_v<decltype (opt), decltype (f)>)
  {
    return maybe_invoke (opt, std::forward<Function> (f));
  }

  template <typename T, typename Function,
            typename = std::enable_if_t<is_maybe_invocable_v<std::optional<T>&&, Function>>>
  constexpr
  maybe_invoke_result_t<std::optional<T>&&, Function>
  operator>>= (std::optional<T>&& opt, Function&& f)
    noexcept (is_nothrow_maybe_invocable_v<decltype (opt), decltype (f)>)
  {
    return maybe_invoke (std::move (opt), std::forward<Function> (f));
  }

  template <typename T, typename Function,
            typename = std::enable_if_t<is_maybe_invocable_v<const std::optional<T>&&, Function>>>
  constexpr
  maybe_invoke_result_t<const std::optional<T>&&, Function>
  operator>>= (const std::optional<T>&& opt, Function&& f)
    noexcept (is_nothrow_maybe_invocable_v<decltype (opt), decltype (f)>)
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

  template <typename T>
  constexpr
  auto
  operator>>= (const std::optional<T>& opt, void (* f) (const T&))
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
  operator>>= (std::optional<T>& opt, Return (* f) (T&))
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

  template <typename T, typename Return>
  constexpr
  auto
  operator>>= (std::optional<T>&& opt, Return (* f) (T&&))
    -> std::enable_if_t<is_maybe_invocable_v<decltype (opt), decltype (f)>,
                        maybe_invoke_result_t<decltype (opt), decltype (f)>>
  {
    return maybe_invoke (std::move (opt), f);
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

  template <typename T, typename Functor,
    std::enable_if_t<
      is_maybe_invocable_v<
        std::optional<T>&, decltype (std::declval<Functor> () ()) (T&)>> * = nullptr>
  constexpr
  decltype (auto)
  operator>> (std::optional<T>& opt, Functor&& f)
  {
    return opt >>= [&f](T&) { return std::forward<Functor> (f); };
  }

 template <typename T, typename Functor,
    std::enable_if_t<
      is_maybe_invocable_v<
        const std::optional<T>&, decltype (std::declval<Functor> () ()) (const T&)>> * = nullptr>
  constexpr
  decltype (auto)
  operator>> (const std::optional<T>& opt, Functor&& f)
  {
    return opt >>= [&f](const T&) { return std::forward<Functor> (f); };
  }

  template <typename T, typename Functor,
    std::enable_if_t<
      is_maybe_invocable_v<
        std::optional<T>&&, decltype (std::declval<Functor> () ()) (T&&)>> * = nullptr>
  constexpr
  decltype (auto)
  operator>> (std::optional<T>&& opt, Functor&& f)
  {
    return opt >>= [&f](T&&) { return std::forward<Functor> (f); };
  }

  template <typename T, typename Functor,
    std::enable_if_t<
      is_maybe_invocable_v<
        const std::optional<T>&&, decltype (std::declval<Functor> () ()) (const T&&)>> * = nullptr>
  constexpr
  decltype (auto)
  operator>> (const std::optional<T>&& opt, Functor&& f)
  {
    return opt >>= [&f](const T&&) { return std::forward<Functor> (f); };
  }

  template <typename NAryOp, typename OptionalLHS, typename ...Optionals,
            std::enable_if_t<is_maybe_invocable_v<
              OptionalLHS, NAryOp, just_t<Optionals>...>> * = nullptr>
  constexpr
  maybe_invoke_result_t<OptionalLHS, NAryOp, just_t<Optionals>...>
  maybe_invoke_with (NAryOp&& n_ary_op, OptionalLHS&& lhs, Optionals&&... optionals)
  {
    using ret_type = maybe_invoke_result_t<OptionalLHS, NAryOp, just_t<Optionals>...>;
    if ((optionals && ...))
      return maybe_invoke (std::forward<OptionalLHS> (lhs),
                           std::forward<NAryOp> (n_ary_op),
                           *std::forward<Optionals> (optionals)...);
    return ret_type ();
  }

  /* assignment */

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS&>> ()
                               += std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator+= (OptionalLHS& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with ([](auto& l, auto&& r) -> auto& { return l += r; },
                              lhs, std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS&>> ()
                               -= std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator-= (OptionalLHS& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with ([](auto& l, auto&& r) -> auto& { return l -= r; },
                              lhs, std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS&>> ()
                               *= std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator*= (OptionalLHS& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with ([](auto& l, auto&& r) -> auto& { return l *= r; },
                              lhs, std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS&>> ()
                               /= std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator/= (OptionalLHS& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with ([](auto& l, auto&& r) -> auto& { return l /= r; },
                              lhs, std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS&>> ()
                               %= std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator%= (OptionalLHS& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with ([](auto& l, auto&& r) -> auto& { return l %= r; },
                              lhs, std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS&>> ()
                               &= std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator&= (OptionalLHS& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with ([](auto& l, auto&& r) -> auto& { return l &= r; },
                              lhs, std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS&>> ()
                               |= std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator|= (OptionalLHS& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with ([](auto& l, auto&& r) -> auto& { return l |= r; },
                              lhs, std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS&>> ()
                               ^= std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator^= (OptionalLHS& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with ([](auto& l, auto&& r) -> auto& { return l ^= r; },
                              lhs, std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS&>> ()
                               <<= std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator<<= (OptionalLHS& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with ([](auto& l, auto&& r) -> auto& { return l <<= r; },
                              lhs, std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS&>> ()
                                >>= std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator>>= (OptionalLHS& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with ([](auto& l, auto&& r) -> auto& { return l >>= r; },
                              lhs, std::forward<OptionalRHS> (rhs));
  }

  /* increment and decrement */

  template <typename Optional,
            std::void_t<std::enable_if_t<is_optional_v<Optional>>,
                        decltype (++std::declval<just_t<Optional&>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator++ (Optional& optional)
  {
    return optional >>= [](auto& x) -> auto& { return ++x; };
  }

  template <typename Optional,
            std::void_t<std::enable_if_t<is_optional_v<Optional>>,
                        decltype (--std::declval<just_t<Optional&>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator-- (Optional& optional)
  {
    return optional >>= [](auto& x) -> auto& { return --x; };
  }

  template <typename Optional,
            std::void_t<std::enable_if_t<is_optional_v<Optional>>,
                        decltype (std::declval<just_t<Optional&>> ()++)> * = nullptr>
  constexpr
  decltype (auto)
  operator++ (Optional& optional, int)
  {
    return optional >>= maybe { [](auto& x) { return x++; } };
  }

  template <typename Optional,
            std::void_t<std::enable_if_t<is_optional_v<Optional>>,
                        decltype (std::declval<just_t<Optional&>> ()--)> * = nullptr>
  constexpr
  decltype (auto)
  operator-- (Optional& optional, int)
  {
    return optional >>= maybe { [](auto& x) { return x--; } };
  }

  /* arithmetic */

  template <typename Optional,
            std::void_t<std::enable_if_t<is_optional_v<Optional>>,
                        decltype (+std::declval<just_t<Optional>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator+ (Optional&& optional)
  {
    return std::forward<Optional> (optional) >>= [](auto&& x) { return std::optional { +x }; };
  }

  template <typename Optional,
            std::void_t<std::enable_if_t<is_optional_v<Optional>>,
                        decltype (-std::declval<just_t<Optional>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator- (Optional&& optional)
  {
    return std::forward<Optional> (optional) >>= maybe { std::negate<void> { } };
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS>> ()
                                + std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator+ (OptionalLHS&& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with (maybe { std::plus<void> { } },
                              std::forward<OptionalLHS> (lhs),
                              std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS>> ()
                                - std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator- (OptionalLHS&& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with (maybe { std::minus<void> { } },
                              std::forward<OptionalLHS> (lhs),
                              std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS>> ()
                                * std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator* (OptionalLHS&& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with (maybe { std::multiplies<void> { } },
                              std::forward<OptionalLHS> (lhs),
                              std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS>> ()
                                / std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator/ (OptionalLHS&& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with (maybe { std::divides<void> { } },
                              std::forward<OptionalLHS> (lhs),
                              std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS>> ()
                                % std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator% (OptionalLHS&& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with (maybe { std::modulus<void> { } },
                              std::forward<OptionalLHS> (lhs),
                              std::forward<OptionalRHS> (rhs));
  }

  template <typename Optional,
            std::void_t<std::enable_if_t<is_optional_v<Optional>>,
                        decltype (~std::declval<just_t<Optional>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator~ (Optional&& optional)
  {
    return std::forward<Optional> (optional) >>= maybe { std::bit_not<void> { } };
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS>> ()
                                & std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator& (OptionalLHS&& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with (maybe { std::bit_and<void> { } },
                              std::forward<OptionalLHS> (lhs),
                              std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS>> ()
                                | std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator| (OptionalLHS&& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with (maybe { std::bit_or<void> { } },
                              std::forward<OptionalLHS> (lhs),
                              std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS>> ()
                                ^ std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator^ (OptionalLHS&& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with (maybe { std::bit_xor<void> { } },
                              std::forward<OptionalLHS> (lhs),
                              std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS>> ()
                               << std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator<< (OptionalLHS&& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with (maybe { [](auto&& l, auto&& r) { return l << r; } },
                              std::forward<OptionalLHS> (lhs),
                              std::forward<OptionalRHS> (rhs));
  }

  template <typename OptionalLHS, typename OptionalRHS,
            std::void_t<std::enable_if_t<is_optional_v<OptionalLHS>
                                     &&  is_optional_v<OptionalRHS>>,
                        decltype (std::declval<just_t<OptionalLHS>> ()
                               >> std::declval<just_t<OptionalRHS>> ())> * = nullptr>
  constexpr
  decltype (auto)
  operator>> (OptionalLHS&& lhs, OptionalRHS&& rhs)
  {
    return maybe_invoke_with (maybe { [](auto&& l, auto&& r) { return l >> r; } },
                              std::forward<OptionalLHS> (lhs),
                              std::forward<OptionalRHS> (rhs));
  }

  constexpr inline
  void
  ffff (void)
  {
    constexpr std::optional<int> x { 4 };
    constexpr std::optional<long> y { 5 };
    static_assert (*(x + y) == 9);

    constexpr std::optional<short> n { std::nullopt };
    static_assert (! (x + n));
    static_assert (! (n + x));
    static_assert (! (n + n));

    static_assert (*(-x) == -4);
    static_assert (*(+(-x)) == -4);
  }

}

#endif // OCTAVE_IR_IR_OPTIONAL_UTIL_HPP

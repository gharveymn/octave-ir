/** ir-function-traits.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_UTILITIES_IR_FUNCTION_TRAITS_HPP
#define OCTAVE_IR_UTILITIES_IR_FUNCTION_TRAITS_HPP

#include "ir-type-pack.hpp"

namespace gch
{

  template <typename Callable>
  struct function_traits;

  template <typename Callable>
  struct function_args;

  template <typename Callable>
  using function_args_t = typename function_args<Callable>::type;

  template <typename Callable>
  struct function_result;

  template <typename Callable>
  using function_result_t = typename function_result<Callable>::type;

  template <typename Callable>
  struct is_lvref_qualified;

  template <typename Callable>
  inline constexpr
  bool
  is_lvref_qualified_v = is_lvref_qualified<Callable>::value;

  template <typename Callable>
  struct is_rvref_qualified;

  template <typename Callable>
  inline constexpr
  bool
  is_rvref_qualified_v = is_rvref_qualified<Callable>::value;

  template <typename Callable>
  struct is_ref_qualified;

  template <typename Callable>
  inline constexpr
  bool
  is_ref_qualified_v = is_rvref_qualified<Callable>::value;

  template <typename Callable>
  struct is_const_qualified;

  template <typename Callable>
  inline constexpr
  bool
  is_const_qualified_v = is_const_qualified<Callable>::value;

  template <typename Callable>
  struct is_volatile_qualified;

  template <typename Callable>
  inline constexpr
  bool
  is_volatile_qualified_v = is_volatile_qualified<Callable>::value;

  template <typename Callable>
  struct is_noexcept_qualified;

  template <typename Callable>
  inline constexpr
  bool
  is_noexcept_qualified_v = is_noexcept_qualified<Callable>::value;

  template <typename FromCallable, typename ToObject>
  struct match_function_cv;

  template <typename FromCallable, typename ToObject>
  using match_function_cv_t = typename match_function_cv<FromCallable, ToObject>::type;

  template <typename FromCallable, typename ToObject>
  struct match_function_ref;

  template <typename FromCallable, typename ToObject>
  using match_function_ref_t = typename match_function_ref<FromCallable, ToObject>::type;

  template <typename FromCallable, typename ToObject>
  struct match_function_cvref;

  template <typename FromCallable, typename ToObject>
  using match_function_cvref_t = typename match_function_cvref<FromCallable, ToObject>::type;

  template <typename Function>
  struct remove_cvref_qualifiers;

  template <typename Function>
  using remove_cvref_qualifiers_t = typename remove_cvref_qualifiers<Function>::type;

  template <typename Member>
  struct unified_equivalent_function;

  template <typename Member>
  using unified_equivalent_function_t = typename unified_equivalent_function<Member>::type;

  namespace detail
  {

    template <typename Function>
    struct function_traits_helper
    { };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...)>
    {
      using function_type         = Result (Args...);
      using unbound_type          = Result (Args...);
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::false_type;
      using is_volatile_qualified = std::false_type;
      using is_noexcept_qualified = std::false_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) const>
    {
      using function_type         = Result (Args...) const;
      using unbound_type          = Result (Args...);
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::true_type;
      using is_volatile_qualified = std::false_type;
      using is_noexcept_qualified = std::false_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) volatile>
    {
      using function_type         = Result (Args...) volatile;
      using unbound_type          = Result (Args...);
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::false_type;
      using is_volatile_qualified = std::true_type;
      using is_noexcept_qualified = std::false_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) const volatile>
    {
      using function_type         = Result (Args...) const volatile;
      using unbound_type          = Result (Args...);
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::true_type;
      using is_volatile_qualified = std::true_type;
      using is_noexcept_qualified = std::false_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) &>
    {
      using function_type         = Result (Args...) &;
      using unbound_type          = Result (Args...);
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::true_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::false_type;
      using is_volatile_qualified = std::false_type;
      using is_noexcept_qualified = std::false_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) const &>
    {
      using function_type         = Result (Args...) const &;
      using unbound_type          = Result (Args...);
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::true_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::true_type;
      using is_volatile_qualified = std::false_type;
      using is_noexcept_qualified = std::false_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) volatile &>
    {
      using function_type         = Result (Args...) volatile &;
      using unbound_type          = Result (Args...);
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::true_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::false_type;
      using is_volatile_qualified = std::true_type;
      using is_noexcept_qualified = std::false_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) const volatile &>
    {
      using function_type         = Result (Args...) const volatile &;
      using unbound_type          = Result (Args...);
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::true_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::true_type;
      using is_volatile_qualified = std::true_type;
      using is_noexcept_qualified = std::false_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) &&>
    {
      using function_type         = Result (Args...) &&;
      using unbound_type          = Result (Args...);
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::true_type;
      using is_const_qualified    = std::false_type;
      using is_volatile_qualified = std::false_type;
      using is_noexcept_qualified = std::false_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) const &&>
    {
      using function_type         = Result (Args...) const &&;
      using unbound_type          = Result (Args...);
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::true_type;
      using is_const_qualified    = std::true_type;
      using is_volatile_qualified = std::false_type;
      using is_noexcept_qualified = std::false_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) volatile &&>
    {
      using function_type         = Result (Args...) volatile &&;
      using unbound_type          = Result (Args...);
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::true_type;
      using is_const_qualified    = std::false_type;
      using is_volatile_qualified = std::true_type;
      using is_noexcept_qualified = std::false_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) const volatile &&>
    {
      using function_type         = Result (Args...) const volatile &&;
      using unbound_type          = Result (Args...);
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::true_type;
      using is_const_qualified    = std::true_type;
      using is_volatile_qualified = std::true_type;
      using is_noexcept_qualified = std::false_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) noexcept>
    {
      using function_type         = Result (Args...) noexcept;
      using unbound_type          = Result (Args...) noexcept;
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::false_type;
      using is_volatile_qualified = std::false_type;
      using is_noexcept_qualified = std::true_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) const noexcept>
    {
      using function_type         = Result (Args...) const noexcept;
      using unbound_type          = Result (Args...) noexcept;
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::true_type;
      using is_volatile_qualified = std::false_type;
      using is_noexcept_qualified = std::true_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) volatile noexcept>
    {
      using function_type         = Result (Args...) volatile noexcept;
      using unbound_type          = Result (Args...) noexcept;
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::false_type;
      using is_volatile_qualified = std::true_type;
      using is_noexcept_qualified = std::true_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) const volatile noexcept>
    {
      using function_type         = Result (Args...) const volatile noexcept;
      using unbound_type          = Result (Args...) noexcept;
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::true_type;
      using is_volatile_qualified = std::true_type;
      using is_noexcept_qualified = std::true_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) & noexcept>
    {
      using function_type         = Result (Args...) & noexcept;
      using unbound_type          = Result (Args...) noexcept;
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::true_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::false_type;
      using is_volatile_qualified = std::false_type;
      using is_noexcept_qualified = std::true_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) const & noexcept>
    {
      using function_type         = Result (Args...) const & noexcept;
      using unbound_type          = Result (Args...) noexcept;
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::true_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::true_type;
      using is_volatile_qualified = std::false_type;
      using is_noexcept_qualified = std::true_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) volatile & noexcept>
    {
      using function_type         = Result (Args...) volatile & noexcept;
      using unbound_type          = Result (Args...) noexcept;
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::true_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::false_type;
      using is_volatile_qualified = std::true_type;
      using is_noexcept_qualified = std::true_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) const volatile & noexcept>
    {
      using function_type         = Result (Args...) const volatile & noexcept;
      using unbound_type          = Result (Args...) noexcept;
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::true_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::true_type;
      using is_volatile_qualified = std::true_type;
      using is_noexcept_qualified = std::true_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) && noexcept>
    {
      using function_type         = Result (Args...) && noexcept;
      using unbound_type          = Result (Args...) noexcept;
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::true_type;
      using is_const_qualified    = std::false_type;
      using is_volatile_qualified = std::false_type;
      using is_noexcept_qualified = std::true_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) const && noexcept>
    {
      using function_type         = Result (Args...) const && noexcept;
      using unbound_type          = Result (Args...) noexcept;
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::true_type;
      using is_const_qualified    = std::true_type;
      using is_volatile_qualified = std::false_type;
      using is_noexcept_qualified = std::true_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) volatile && noexcept>
    {
      using function_type         = Result (Args...) volatile && noexcept;
      using unbound_type          = Result (Args...) noexcept;
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::true_type;
      using is_const_qualified    = std::false_type;
      using is_volatile_qualified = std::true_type;
      using is_noexcept_qualified = std::true_type;
    };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...) const volatile && noexcept>
    {
      using function_type         = Result (Args...) const volatile && noexcept;
      using unbound_type          = Result (Args...) noexcept;
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::true_type;
      using is_const_qualified    = std::true_type;
      using is_volatile_qualified = std::true_type;
      using is_noexcept_qualified = std::true_type;
    };

    template <typename Callable, typename Enable = void>
    struct function_traits_impl
    {
      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::false_type;
      using is_const_qualified    = std::false_type;
      using is_volatile_qualified = std::false_type;
      using is_noexcept_qualified = std::false_type;
    };

    template <typename Function>
    struct function_traits_impl<Function, std::enable_if_t<std::is_function_v<Function>>>
      : function_traits_helper<Function>
    {
      using callable_type = Function;
    };

    template <typename Function>
    struct function_traits_impl<Function *, std::enable_if_t<std::is_function_v<Function>>>
      : function_traits_helper<Function>
    {
      using callable_type = Function *;
    };

    template <typename M, typename T>
    struct function_traits_impl<M T::*, std::enable_if_t<std::is_function_v<M>>>
      : function_traits_helper<M>
    {
      using callable_type = M T::*;
    };

    template <typename F, typename Enable = void>
    struct function_args_impl
    { };

    template <typename F>
    struct function_args_impl<F, std::void_t<typename function_traits<F>::args_pack>>
    {
      using type = typename function_traits<F>::args_pack;
    };

    template <typename F, typename Enable = void>
    struct function_result_impl
    { };

    template <typename F>
    struct function_result_impl<F, std::void_t<typename function_traits<F>::result_type>>
    {
      using type = typename function_traits<F>::result_type;
    };

    template <typename Function, typename Enable = void>
    struct remove_cvref_qualifiers_impl
    { };

    template <typename Function>
    struct remove_cvref_qualifiers_impl<
      Function,
      std::void_t<std::enable_if_t<std::is_function_v<Function>>,
                  typename function_traits<Function>::unbound_type>>
    {
      using type = typename function_traits<Function>::unbound_type;
    };

    template <typename Object, typename Result, typename ArgsPack, bool IsNoexcept>
    struct unified_equivalent_function_helper
    { };

    template <typename Object, typename Result, typename ...Args, bool IsNoexcept>
    struct unified_equivalent_function_helper<Object, Result, type_pack<Args...>, IsNoexcept>
      : std::conditional<std::is_reference_v<Object>,
                         std::conditional_t<IsNoexcept,
                                            Result (Object, Args...) noexcept,
                                            Result (Object, Args...)>,
                         std::conditional_t<IsNoexcept,
                                            Result (Object&, Args...) noexcept,
                                            Result (Object&, Args...)>>
    { };

    template <typename Member, typename Enable = void>
    struct unified_equivalent_function_impl
    { };

    template <typename Result, typename ...Args>
    struct unified_equivalent_function_impl<Result (Args...)>
    {
      using type = Result (Args...);
    };

    template <typename Result, typename ...Args>
    struct unified_equivalent_function_impl<Result (Args...) noexcept>
    {
      using type = Result (Args...) noexcept;
    };

    template <typename M, typename T>
    struct unified_equivalent_function_impl<M T::*, std::enable_if_t<std::is_function_v<M>>>
      : unified_equivalent_function_helper<match_function_cvref_t<M, T>,
                                           function_result_t<M>,
                                           function_args_t<M>,
                                           is_noexcept_qualified_v<M>>
    { };

  } // namespace gch::detail

  template <typename Callable>
  struct function_traits
    : detail::function_traits_impl<Callable>
  { };

  template <typename Callable>
  struct function_args
  {
    using type = typename function_traits<Callable>::args_pack;
  };

  template <typename Callable>
  struct function_result
  {
    using type = typename function_traits<Callable>::result_type;
  };

  template <typename Callable>
  struct is_lvref_qualified
    : function_traits<Callable>::is_lvref_qualified
  { };

  template <typename Callable>
  struct is_rvref_qualified
    : function_traits<Callable>::is_rvref_qualified
  { };

  template <typename Callable>
  struct is_ref_qualified
    : std::disjunction<is_lvref_qualified<Callable>, is_rvref_qualified<Callable>>
  { };

  template <typename Callable>
  struct is_const_qualified
    : function_traits<Callable>::is_const_qualified
  { };

  template <typename Callable>
  struct is_volatile_qualified
    : function_traits<Callable>::is_volatile_qualified
  { };

  template <typename Callable>
  struct is_noexcept_qualified
    : function_traits<Callable>::is_noexcept_qualified
  { };

  template <typename FromCallable, typename ToObject>
  struct match_function_cv
    : std::conditional<is_const_qualified_v<FromCallable>,
                       std::conditional_t<is_volatile_qualified_v<FromCallable>,
                                          std::add_cv_t<ToObject>,
                                          std::add_const_t<ToObject>>,
                       std::conditional_t<is_volatile_qualified_v<FromCallable>,
                                          std::add_volatile_t<ToObject>,
                                          ToObject>>
  { };

  template <typename FromCallable, typename ToObject>
  struct match_function_ref
    : std::conditional<is_lvref_qualified_v<FromCallable>,
                       std::add_lvalue_reference_t<ToObject>,
                       std::conditional_t<is_rvref_qualified_v<FromCallable>,
                                          std::add_rvalue_reference_t<ToObject>,
                                          ToObject>>
  { };

  template <typename FromCallable, typename ToObject>
  struct match_function_cvref
    : match_function_ref<FromCallable, match_function_cv_t<FromCallable, ToObject>>
  { };

  template <typename Function>
  struct remove_cvref_qualifiers
    : detail::remove_cvref_qualifiers_impl<Function>
  { };

  template <typename Member>
  struct unified_equivalent_function
    : detail::unified_equivalent_function_impl<Member>
  { };

} // namespace gch

#endif // OCTAVE_IR_UTILITIES_IR_FUNCTION_TRAITS_HPP

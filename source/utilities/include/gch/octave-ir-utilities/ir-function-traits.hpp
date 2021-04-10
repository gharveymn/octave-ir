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

  template <typename Function>
  struct function_traits;

  template <typename Function>
  struct function_args;

  template <typename Function>
  using function_args_t = typename function_args<Function>::type;

  template <typename Function>
  struct function_result;

  template <typename Function>
  using function_result_t = typename function_result<Function>::type;

  template <typename Function>
  struct is_lvref_qualified;

  template <typename Function>
  inline constexpr
  bool
  is_lvref_qualified_v = is_lvref_qualified<Function>::value;

  template <typename Function>
  struct is_rvref_qualified;

  template <typename Function>
  inline constexpr
  bool
  is_rvref_qualified_v = is_rvref_qualified<Function>::value;

  template <typename Function>
  struct is_ref_qualified;

  template <typename Function>
  inline constexpr
  bool
  is_ref_qualified_v = is_rvref_qualified<Function>::value;

  template <typename Function>
  struct is_const_qualified;

  template <typename Function>
  inline constexpr
  bool
  is_const_qualified_v = is_const_qualified<Function>::value;

  template <typename Function>
  struct is_volatile_qualified;

  template <typename Function>
  inline constexpr
  bool
  is_volatile_qualified_v = is_volatile_qualified<Function>::value;

  template <typename Function>
  struct is_noexcept_qualified;

  template <typename Function>
  inline constexpr
  bool
  is_noexcept_qualified_v = is_noexcept_qualified<Function>::value;

  template <typename FromFunction, typename ToObject>
  struct match_function_cv;

  template <typename FromFunction, typename ToObject>
  using match_function_cv_t = typename match_function_cv<FromFunction, ToObject>::type;

  template <typename FromFunction, typename ToObject>
  struct match_function_ref;

  template <typename FromFunction, typename ToObject>
  using match_function_ref_t = typename match_function_ref<FromFunction, ToObject>::type;

  template <typename FromFunction, typename ToObject>
  struct match_function_cvref;

  template <typename FromFunction, typename ToObject>
  using match_function_cvref_t = typename match_function_cvref<FromFunction, ToObject>::type;

  template <typename Function>
  struct remove_cvref_qualifiers;

  template <typename Function>
  using remove_cvref_qualifiers_t = typename remove_cvref_qualifiers<Function>::type;

  namespace detail
  {

    template <typename Function>
    struct function_traits_helper
    { };

    template<typename Result, typename... Args>
    struct function_traits_helper<Result (Args...)>
    {
      using function_type         = Result (Args...);
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
      using reduced_type          = Result (Args...);
      using result_type           = Result;
      using args_pack             = type_pack<Args...>;

      using is_lvref_qualified    = std::false_type;
      using is_rvref_qualified    = std::true_type;
      using is_const_qualified    = std::true_type;
      using is_volatile_qualified = std::true_type;
      using is_noexcept_qualified = std::true_type;
    };

    template <typename Function, typename Enable = void>
    struct function_traits_impl
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
    struct function_traits_impl<M T::*>
      : function_traits_helper<M>
    {
      using callable_type = M T::*;
    };

    template <typename Function>
    struct function_args_helper
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...)>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) const>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) volatile>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) const volatile>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) &>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) const &>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) volatile &>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) const volatile &>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) &&>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) const &&>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) volatile &&>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) const volatile &&>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) noexcept>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) const noexcept>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) volatile noexcept>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) const volatile noexcept>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) & noexcept>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) const & noexcept>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) volatile & noexcept>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) const volatile & noexcept>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) && noexcept>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) const && noexcept>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) volatile && noexcept>
      : type_pack<Args...>
    { };

    template<typename Result, typename... Args>
    struct function_args_helper<Result (Args...) const volatile && noexcept>
      : type_pack<Args...>
    { };

    template <typename Function>
    struct function_args_impl
      : function_args_helper<Function>
    { };

    template <typename M, typename T>
    struct function_args_impl<M T::*>
      : function_args_helper<M>
    { };

    template <typename Function>
    struct function_result_helper
    { };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...)>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) const>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) volatile>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) const volatile>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) &>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) const &>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) volatile &>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) const volatile &>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) &&>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) const &&>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) volatile &&>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) const volatile &&>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) noexcept>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) const noexcept>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) volatile noexcept>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) const volatile noexcept>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) & noexcept>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) const & noexcept>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) volatile & noexcept>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) const volatile & noexcept>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) && noexcept>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) const && noexcept>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) volatile && noexcept>
    {
      using type = Result;
    };

    template<typename Result, typename... Args>
    struct function_result_helper<Result (Args...) const volatile && noexcept>
    {
      using type = Result;
    };

    template <typename Function>
    struct function_result_impl
      : function_result_helper<Function>
    { };

    template <typename M, typename T>
    struct function_result_impl<M T::*>
      : function_result_impl<M>
    { };

    template <typename Function>
    struct is_lvref_qualified_helper
      : std::false_type
    { };

    template<typename Result, typename... Args>
    struct is_lvref_qualified_helper<Result (Args...) &>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_lvref_qualified_helper<Result (Args...) const &>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_lvref_qualified_helper<Result (Args...) volatile &>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_lvref_qualified_helper<Result (Args...) const volatile &>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_lvref_qualified_helper<Result (Args...) & noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_lvref_qualified_helper<Result (Args...) const & noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_lvref_qualified_helper<Result (Args...) volatile & noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_lvref_qualified_helper<Result (Args...) const volatile & noexcept>
      : std::true_type
    { };

    template <typename Function>
    struct is_lvref_qualified_impl
      : is_lvref_qualified_helper<Function>
    { };

    template <typename M, typename T>
    struct is_lvref_qualified_impl<M T::*>
      : is_lvref_qualified_helper<M>
    { };

    template <typename Function>
    struct is_rvref_qualified_helper
      : std::false_type
    { };

    template<typename Result, typename... Args>
    struct is_rvref_qualified_helper<Result (Args...) &&>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_rvref_qualified_helper<Result (Args...) const &&>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_rvref_qualified_helper<Result (Args...) volatile &&>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_rvref_qualified_helper<Result (Args...) const volatile &&>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_rvref_qualified_helper<Result (Args...) && noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_rvref_qualified_helper<Result (Args...) const && noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_rvref_qualified_helper<Result (Args...) volatile && noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_rvref_qualified_helper<Result (Args...) const volatile && noexcept>
      : std::true_type
    { };

    template <typename Function>
    struct is_rvref_qualified_impl
      : is_rvref_qualified_helper<Function>
    { };

    template <typename M, typename T>
    struct is_rvref_qualified_impl<M T::*>
      : is_rvref_qualified_helper<M>
    { };

    template <typename Function>
    struct is_const_qualified_helper
      : std::false_type
    { };

    template<typename Result, typename... Args>
    struct is_const_qualified_helper<Result (Args...) const>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_const_qualified_helper<Result (Args...) const volatile>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_const_qualified_helper<Result (Args...) const &>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_const_qualified_helper<Result (Args...) const volatile &>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_const_qualified_helper<Result (Args...) const &&>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_const_qualified_helper<Result (Args...) const volatile &&>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_const_qualified_helper<Result (Args...) const noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_const_qualified_helper<Result (Args...) const volatile noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_const_qualified_helper<Result (Args...) const & noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_const_qualified_helper<Result (Args...) const volatile & noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_const_qualified_helper<Result (Args...) const && noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_const_qualified_helper<Result (Args...) const volatile && noexcept>
      : std::true_type
    { };

    template <typename Function>
    struct is_const_qualified_impl
      : is_const_qualified_helper<Function>
    { };

    template <typename M, typename T>
    struct is_const_qualified_impl<M T::*>
      : is_const_qualified_helper<M>
    { };

    template <typename Function>
    struct is_volatile_qualified_helper
      : std::false_type
    { };

    template<typename Result, typename... Args>
    struct is_volatile_qualified_helper<Result (Args...) volatile>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_volatile_qualified_helper<Result (Args...) const volatile>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_volatile_qualified_helper<Result (Args...) volatile &>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_volatile_qualified_helper<Result (Args...) const volatile &>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_volatile_qualified_helper<Result (Args...) volatile &&>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_volatile_qualified_helper<Result (Args...) const volatile &&>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_volatile_qualified_helper<Result (Args...) volatile noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_volatile_qualified_helper<Result (Args...) const volatile noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_volatile_qualified_helper<Result (Args...) volatile & noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_volatile_qualified_helper<Result (Args...) const volatile & noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_volatile_qualified_helper<Result (Args...) volatile && noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_volatile_qualified_helper<Result (Args...) const volatile && noexcept>
      : std::true_type
    { };

    template <typename Function>
    struct is_volatile_qualified_impl
      : is_volatile_qualified_helper<Function>
    { };

    template <typename M, typename T>
    struct is_volatile_qualified_impl<M T::*>
      : is_volatile_qualified_helper<M>
    { };

    template <typename Function>
    struct is_noexcept_qualified_helper
      : std::false_type
    { };

    template<typename Result, typename... Args>
    struct is_noexcept_qualified_helper<Result (Args...) noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_noexcept_qualified_helper<Result (Args...) const noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_noexcept_qualified_helper<Result (Args...) volatile noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_noexcept_qualified_helper<Result (Args...) const volatile noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_noexcept_qualified_helper<Result (Args...) & noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_noexcept_qualified_helper<Result (Args...) const & noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_noexcept_qualified_helper<Result (Args...) volatile & noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_noexcept_qualified_helper<Result (Args...) const volatile & noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_noexcept_qualified_helper<Result (Args...) && noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_noexcept_qualified_helper<Result (Args...) const && noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_noexcept_qualified_helper<Result (Args...) volatile && noexcept>
      : std::true_type
    { };

    template<typename Result, typename... Args>
    struct is_noexcept_qualified_helper<Result (Args...) const volatile && noexcept>
      : std::true_type
    { };

    template <typename Function>
    struct is_noexcept_qualified_impl
      : is_noexcept_qualified_helper<Function>
    { };

    template <typename M, typename T>
    struct is_noexcept_qualified_impl<M T::*>
      : is_noexcept_qualified_helper<M>
    { };

    template <typename Function>
    struct remove_cvref_qualifiers_helper
    { };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...)>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) const>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) volatile>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) const volatile>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) &>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) const &>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) volatile &>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) const volatile &>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) &&>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) const &&>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) volatile &&>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) const volatile &&>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) noexcept>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) const noexcept>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) volatile noexcept>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) const volatile noexcept>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) & noexcept>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) const & noexcept>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) volatile & noexcept>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) const volatile & noexcept>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) && noexcept>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) const && noexcept>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) volatile && noexcept>
    {
      using type = Result (Args...);
    };

    template<typename Result, typename... Args>
    struct remove_cvref_qualifiers_helper<Result (Args...) const volatile && noexcept>
    {
      using type = Result (Args...);
    };

    template <typename Function>
    struct remove_cvref_qualifiers_impl
      : remove_cvref_qualifiers_helper<Function>
    { };

    template <typename M, typename T>
    struct remove_cvref_qualifiers_impl<M T::*>
      : remove_cvref_qualifiers_impl<M>
    { };

  } // namespace gch::detail

  template <typename Function>
  struct function_traits
    : detail::function_traits_impl<Function>
  { };

  template <typename Function>
  struct function_args
  {
    using type = typename function_traits<Function>::args_pack;
  };

  template <typename Function>
  struct function_result
  {
    using type = typename function_traits<Function>::result_type;
  };

  template <typename Function>
  struct is_lvref_qualified
    : function_traits<Function>::is_lvref_qualified
  { };

  template <typename Function>
  struct is_rvref_qualified
    : function_traits<Function>::is_rvref_qualified
  { };

  template <typename Function>
  struct is_ref_qualified
    : std::disjunction<is_lvref_qualified<Function>, is_rvref_qualified<Function>>
  { };

  template <typename Function>
  struct is_const_qualified
    : function_traits<Function>::is_const_qualified
  { };

  template <typename Function>
  struct is_volatile_qualified
    : function_traits<Function>::is_volatile_qualified
  { };

  template <typename Function>
  struct is_noexcept_qualified
    : function_traits<Function>::is_noexcept_qualified
  { };

  template <typename FromFunction, typename ToObject>
  struct match_function_cv
    : std::conditional<is_const_qualified_v<FromFunction>,
                       std::conditional_t<is_volatile_qualified_v<FromFunction>,
                                          std::add_cv_t<ToObject>,
                                          std::add_const_t<ToObject>>,
                       std::conditional_t<is_volatile_qualified_v<FromFunction>,
                                          std::add_volatile_t<ToObject>,
                                          ToObject>>
  { };

  template <typename FromFunction, typename ToObject>
  struct match_function_ref
    : std::conditional<is_lvref_qualified_v<FromFunction>,
                       std::add_lvalue_reference_t<ToObject>,
                       std::conditional_t<is_rvref_qualified_v<FromFunction>,
                                          std::add_rvalue_reference_t<ToObject>,
                                          ToObject>>
  { };

  template <typename FromFunction, typename ToObject>
  struct match_function_cvref
    : match_function_ref<FromFunction, match_function_cv_t<FromFunction, ToObject>>
  { };

  template <typename Function>
  struct remove_cvref_qualifiers
  { };

} // namespace gch

#endif // OCTAVE_IR_UTILITIES_IR_FUNCTION_TRAITS_HPP

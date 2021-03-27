/** ir-functional.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_FUNCTIONAL_HPP
#define OCTAVE_IR_IR_FUNCTIONAL_HPP

#include "utilities/ir-type-traits.hpp"

#include <utility>
#include <tuple>
#include <type_traits>

namespace gch
{

  namespace detail
  {

    template <typename Functor, typename ...Args>
    constexpr
    auto
    invoke_impl (Functor&& f, Args&&... args)
      noexcept (std::is_nothrow_invocable_v<Functor, Args...>)
      -> std::invoke_result_t<Functor, Args...>
    {
      return std::forward<Functor> (f) (std::forward<Args>(args)...);
    }

    template <typename M, typename T, typename Object, typename... Args>
    constexpr
    auto
    invoke_impl (M T::* f, Object&& obj, Args&&... args)
      noexcept (std::is_nothrow_invocable_v<decltype (f), Object, Args...>)
      -> std::invoke_result_t<decltype (f), Object, Args...>
    {
      if constexpr (std::is_member_function_pointer_v<decltype (f)>)
      {
        if constexpr (std::is_base_of_v<T, std::decay_t<Object>>)
          return (std::forward<Object> (obj).*f) (std::forward<Args> (args)...);
        else if constexpr (is_reference_wrapper_v<std::decay_t<Object>>)
          return (obj.get ().*f) (std::forward<Args>(args)...);
        else
          return ((*std::forward<Object> (obj)).*f)(std::forward<Args> (args)...);
      }
      else
      {
        static_assert (std::is_member_object_pointer_v<decltype (f)>);
        static_assert (sizeof...(args) == 0);

        if constexpr (std::is_base_of_v<T, std::decay_t<Object>>)
          return std::forward<Object> (obj).*f;
        else if constexpr (is_reference_wrapper_v<std::decay_t<Object>>)
          return obj.get ().*f;
        else
          return (*std::forward<Object> (obj)).*f;
      }
    }

  }

  template<typename Functor, typename ...Args>
  constexpr
  std::invoke_result_t<Functor, Args...>
  invoke (Functor&& f, Args&&... args)
    noexcept (std::is_nothrow_invocable_v<Functor, Args...>)
  {
    return detail::invoke_impl (std::forward<Functor> (f), std::forward<Args> (args)...);
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
    constexpr explicit
    overloaded_ret (Ts&&... ts)
      : overloaded<Ts...> (std::forward<Ts> (ts)...)
    { }

    template <typename T>
    R
    operator() (T&& t)
    {
      return overloaded<Ts...>::operator() (std::forward<T> (t));
    }
  };

  template <typename R, typename ...Ts>
  constexpr
  overloaded_ret<R, overloaded<Ts...>>
  make_overloaded (Ts&&... ts)
  {
    return overloaded_ret<R, overloaded<Ts...>> { std::forward<Ts> (ts)... };
  }

  template <typename Functor>
  struct applied
  {
    template <typename Tuple>
    decltype (auto)
    operator() (Tuple&& t)
    {
      return std::apply (m_func, std::forward<Tuple> (t));
    }

    Functor m_func;
  };

  template <typename Functor> applied (Functor) -> applied<Functor>;

  namespace detail
  {

    template <typename T, typename MemFn>
    class bound_mem_fn_object;

    template <typename T, typename MemFn>
    class bound_mem_fn_object<T&, MemFn>
    {
    public:
      template <typename F>
      constexpr
      bound_mem_fn_object (T& object, F&& f)
        : m_object_ptr (&object),
          m_mem_fn (std::forward<F> (f))
      { }

      template <typename ...Args>
      constexpr
      std::enable_if_t<std::is_invocable_v<MemFn, T&, Args...>,
                       std::invoke_result_t<MemFn, T&, Args...>>
      operator() (Args&&... args) const
        noexcept (std::is_nothrow_invocable_v<MemFn, T&, Args...>)
      {
        return invoke (m_mem_fn, *m_object_ptr, std::forward<Args> (args)...);
      }

    private:
      T *   m_object_ptr;
      MemFn m_mem_fn;
    };

    template <typename T, typename MemFn>
    class bound_mem_fn_object<T&&, MemFn>
    {
    public:
      template <typename F>
      constexpr
      bound_mem_fn_object (T&& object, F&& f)
        : m_object_ptr (&object),
          m_mem_fn (std::forward<F> (f))
      { }

      template <typename ...Args>
      constexpr
      std::enable_if_t<std::is_invocable_v<MemFn, T&&, Args...>,
                       std::invoke_result_t<MemFn, T&&, Args...>>
      operator() (Args&&... args) const
        noexcept (std::is_nothrow_invocable_v<MemFn, T&&, Args...>)
      {
        return invoke (m_mem_fn, std::move (*m_object_ptr), std::forward<Args> (args)...);
      }

    private:
      T *   m_object_ptr;
      MemFn m_mem_fn;
    };

    template <typename T, typename MemFn>
    bound_mem_fn_object (T&, MemFn) -> bound_mem_fn_object<T&, MemFn>;

    template <typename T, typename MemFn>
    bound_mem_fn_object (T&&, MemFn) -> bound_mem_fn_object<T&&, MemFn>;

  }

  template <typename ...Args, typename T, typename Base, typename Return,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>> * = nullptr>
  constexpr
  decltype (auto)
  bound_mem_fn (T& object, Return (Base::* f) (Args...))
  {
    return detail::bound_mem_fn_object { object, f };
  }

  template <typename ...Args, typename T, typename Base, typename Return,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>> * = nullptr>
  constexpr
  decltype (auto)
  bound_mem_fn (T& object, Return (Base::* f) (Args...) &)
  {
    return detail::bound_mem_fn_object { object, f };
  }

  template <typename ...Args, typename T, typename Base, typename Return,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>> * = nullptr>
  constexpr
  decltype (auto)
  bound_mem_fn (const T& object, Return (Base::* f) (Args...) const)
  {
    return detail::bound_mem_fn_object { object, f };
  }

  template <typename ...Args, typename T, typename Base, typename Return,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>> * = nullptr>
  constexpr
  decltype (auto)
  bound_mem_fn (const T& object, Return (Base::* f) (Args...) const &)
  {
    return detail::bound_mem_fn_object { object, f };
  }

  template <typename ...Args, typename T, typename Base, typename Return,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>> * = nullptr>
  constexpr
  decltype (auto)
  bound_mem_fn (T&& object, Return (Base::* f) (Args...))
  {
    return detail::bound_mem_fn_object { std::move (object), f };
  }

  template <typename ...Args, typename T, typename Base, typename Return,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>> * = nullptr>
  constexpr
  decltype (auto)
  bound_mem_fn (T&& object, Return (Base::* f) (Args...) &&)
  {
    return detail::bound_mem_fn_object { std::move (object), f };
  }

  template <typename T, typename U>
  void
  bound_mem_fn (const T&&, U&&) = delete;

  template <typename T, typename Base, typename M,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>
                         &&  std::is_member_object_pointer_v<M Base::*>> * = nullptr>
  constexpr
  decltype (auto)
  bound_mem_fn (T& object, M Base::* m)
  {
    return detail::bound_mem_fn_object { object, m };
  }

  template <typename T>
  struct value_projection
  {
    template <typename ...Args>
    T
    operator() (Args&&...) const
      noexcept (std::is_nothrow_copy_constructible_v<T>)
    {
      return m_value;
    }

    T m_value;
  };

  template <typename T>
  value_projection (T) -> value_projection<T>;

  struct identity
  {
    template <typename T>
    decltype (auto)
    operator() (T&& t) const noexcept
    {
      return std::forward<T> (t);
    }
  };

}

#endif // OCTAVE_IR_IR_FUNCTIONAL_HPP

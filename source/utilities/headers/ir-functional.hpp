/** ir-functional.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_UTILITIES_IR_FUNCTIONAL_HPP
#define OCTAVE_IR_UTILITIES_IR_FUNCTIONAL_HPP

#include "ir-type-traits.hpp"

#include <array>
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

  } // namespace gch::detail

  template<typename Functor, typename ...Args>
  constexpr
  std::invoke_result_t<Functor, Args...>
  invoke (Functor&& f, Args&&... args)
    noexcept (std::is_nothrow_invocable_v<Functor, Args...>)
  {
    return detail::invoke_impl (std::forward<Functor> (f), std::forward<Args> (args)...);
  }

  template<typename Ret, typename Functor, typename ...Args,
           std::enable_if_t<std::is_invocable_r_v<Ret, Functor, Args...>> * = nullptr>
  constexpr
  Ret
  invoke_r (Functor&& f, Args&&... args)
    noexcept (std::is_nothrow_invocable_r_v<Ret, Functor, Args...>)
  {
    if constexpr (std::is_void_v<Ret>)
      gch::invoke (std::forward<Functor> (f), std::forward<Args> (args)...);
    else
      return gch::invoke (std::forward<Functor> (f), std::forward<Args> (args)...);
  }

  struct identity
  {
    template <typename T>
    decltype (auto)
    operator() (T&& t) const noexcept
    {
      return std::forward<T> (t);
    }
  };

  struct tuple_forwarder
  {
    template <typename ...Args>
    std::tuple<Args...>
    operator() (Args&&... args) const noexcept
    {
      return std::forward_as_tuple (std::forward<Args> (args)...);
    }
  };

  template <typename ...Ts>
  struct overloaded : Ts...
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

  template <typename Functor = void>
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

  template <>
  struct applied<void>
  {
    static constexpr gch::tuple_forwarder m_func { };
  };

  template <typename FunctorLHS, typename FunctorRHS>
  auto
  operator| (applied<FunctorLHS> lhs, FunctorRHS rhs)
  {
    return applied {
      [=](auto&&... ts)
      {
        return applied { rhs } (lhs.m_func (std::forward<decltype (ts)> (ts)...));
      }
    };
  }

  inline constexpr applied<void> applied_v { };

  template <typename Functor> applied (Functor) -> applied<Functor>;

  namespace detail
  {

    struct bind_front_create_tag { };
    static constexpr bind_front_create_tag bind_front_create { };

    template <typename Functor, typename ...BoundArgs>
    class bind_front_object
    {
      using functor_type  = Functor;
      using bound_tuple   = std::tuple<BoundArgs...>;
      using bound_indices = std::index_sequence_for<BoundArgs...>;

    public:
      struct create_tag { };
      static constexpr create_tag create { };

      bind_front_object            (void)                         = default;
      bind_front_object            (const bind_front_object&)     = default;
      bind_front_object            (bind_front_object&&) noexcept = default;
      bind_front_object& operator= (const bind_front_object&)     = default;
      bind_front_object& operator= (bind_front_object&&) noexcept = default;
      ~bind_front_object           (void)                         = default;

      template <typename FunctorIn, typename ...Ts>
      constexpr explicit
      bind_front_object (bind_front_create_tag, FunctorIn&& f, Ts&&... ts)
        noexcept (std::is_nothrow_constructible_v<functor_type, FunctorIn>
              &&  std::is_nothrow_constructible_v<bound_tuple, Ts...>)
        : m_functor    (std::forward<FunctorIn> (f)),
          m_bound_args (std::forward<Ts> (ts)...)
      { }

      template <typename ...Args>
      constexpr
      std::invoke_result_t<Functor&, BoundArgs&..., Args...>
      operator() (Args&&... args) &
        noexcept (std::is_nothrow_invocable_v<Functor&, BoundArgs&..., Args...>)
      {
        return invoke (bound_indices { }, m_functor, m_bound_args, std::forward<Args> (args)...);
      }

      template <typename ...Args>
      constexpr
      std::invoke_result_t<const Functor&, const BoundArgs&..., Args...>
      operator() (Args&&... args) const &
        noexcept (std::is_nothrow_invocable_v<const Functor&, const BoundArgs&..., Args...>)
      {
        return invoke (bound_indices { }, m_functor, m_bound_args, std::forward<Args> (args)...);
      }

      template <typename ...Args>
      constexpr
      std::invoke_result_t<Functor, BoundArgs..., Args...>
      operator() (Args&&... args) &&
        noexcept (std::is_nothrow_invocable_v<Functor, BoundArgs..., Args...>)
      {
        return invoke (bound_indices { }, std::move (m_functor), std::move (m_bound_args),
                       std::forward<Args> (args)...);
      }

      template <typename ...Args>
      constexpr
      std::invoke_result_t<const Functor, const BoundArgs..., Args...>
      operator() (Args&&... args) const &&
        noexcept (std::is_nothrow_invocable_v<const Functor, const BoundArgs..., Args...>)
      {
        return invoke (bound_indices { }, std::move (m_functor), std::move (m_bound_args),
                       std::forward<Args> (args)...);
      }

    private:
      template <typename F, typename Tup, std::size_t ...Indices, typename ...Args>
      static constexpr
      decltype (auto)
      invoke (std::index_sequence<Indices...>, F&& f, Tup&& bound_tup, Args&&... args)
      {
        return gch::invoke (std::forward<F> (f),
                            std::get<Indices> (std::forward<Tup> (bound_tup))...,
                            std::forward<Args> (args)...);
      }

      functor_type m_functor;
      bound_tuple  m_bound_args;
    };

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
        return gch::invoke (m_mem_fn, *m_object_ptr, std::forward<Args> (args)...);
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
        return gch::invoke (m_mem_fn, std::move (*m_object_ptr), std::forward<Args> (args)...);
      }

    private:
      T *   m_object_ptr;
      MemFn m_mem_fn;
    };

    template <typename T, typename MemFn>
    bound_mem_fn_object (T&, MemFn) -> bound_mem_fn_object<T&, MemFn>;

    template <typename T, typename MemFn>
    bound_mem_fn_object (T&&, MemFn) -> bound_mem_fn_object<T&&, MemFn>;

  } // namespace gch::detail

  template <typename Functor, typename ...Args>
  using bind_front_t = detail::bind_front_object<std::decay_t<Functor>, std::decay_t<Args>...>;

  template <typename Functor, typename ...Args>
  constexpr
  bind_front_t<Functor, Args...>
  bind_front (Functor&& f, Args&&... args)
    noexcept (std::is_nothrow_constructible_v<detail::bind_front_create_tag,
                                              bind_front_t<Functor, Args...>,
                                              Functor, Args...>)
  {
    return bind_front_t<Functor, Args...> (detail::bind_front_create, std::forward<Functor> (f),
                                           std::forward<Args> (args)...);
  }

  template <typename ...Args, typename T, typename Base, typename Return,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>> * = nullptr>
  constexpr
  auto
  bound_mem_fn (T& object, Return (Base::* f) (Args...))
  {
    return detail::bound_mem_fn_object { object, f };
  }

  template <typename ...Args, typename T, typename Base, typename Return,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>> * = nullptr>
  constexpr
  auto
  bound_mem_fn (T& object, Return (Base::* f) (Args...) &)
  {
    return detail::bound_mem_fn_object { object, f };
  }

  template <typename ...Args, typename T, typename Base, typename Return,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>> * = nullptr>
  constexpr
  auto
  bound_mem_fn (const T& object, Return (Base::* f) (Args...) const)
  {
    return detail::bound_mem_fn_object { object, f };
  }

  template <typename ...Args, typename T, typename Base, typename Return,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>> * = nullptr>
  constexpr
  auto
  bound_mem_fn (const T& object, Return (Base::* f) (Args...) const &)
  {
    return detail::bound_mem_fn_object { object, f };
  }

  template <typename ...Args, typename T, typename Base, typename Return,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>> * = nullptr>
  constexpr
  auto
  bound_mem_fn (T&& object, Return (Base::* f) (Args...))
  {
    return detail::bound_mem_fn_object { std::move (object), f };
  }

  template <typename ...Args, typename T, typename Base, typename Return,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>> * = nullptr>
  constexpr
  auto
  bound_mem_fn (T&& object, Return (Base::* f) (Args...) &&)
  {
    return detail::bound_mem_fn_object { std::move (object), f };
  }

  template <typename T, typename U>
  constexpr
  void
  bound_mem_fn (const T&& object, U&&) = delete;

  template <typename T, typename Base, typename M,
            std::enable_if_t<std::is_base_of_v<Base, std::decay_t<T>>
                         &&  std::is_member_object_pointer_v<M Base::*>> * = nullptr>
  constexpr
  decltype (auto)
  bound_mem_fn (T& object, M Base::* m)
  {
    return detail::bound_mem_fn_object { object, m };
  }

  namespace detail
  {

    template <typename FunctionType, FunctionType Function, typename Enable = void>
    struct static_function_impl
    {
      static_function_impl (void) = delete;
    };

    template <typename FunctionType, FunctionType Function>
    struct static_function_impl<FunctionType,
                                Function,
                                std::enable_if_t<std::is_function_v<FunctionType>>>
      : std::integral_constant<FunctionType, Function>
    { };

    template <typename M, typename T, M T::* Function>
    struct static_function_impl<M T::*,
                                Function,
                                std::enable_if_t<std::is_function_v<M>>>
      : std::integral_constant<M T::*, Function>
    { };

  } // namespace gch::detail

  template <auto Function>
  struct static_function
    : detail::static_function_impl<decltype (Function), Function>
  { };

  template <auto Function>
  inline constexpr
  auto
  static_function_v = static_function<Function> { };

  template <typename Function>
  class unbound_function;

  namespace detail
  {

    template <typename Function, typename ArgsPack>
    struct unbound_function_base;

    template <typename Function, typename ...Args>
    struct unbound_function_base<Function, type_pack<Args...>>
    {
      using function_type  = Function;
      using result_type    = function_result_t<Function>;
      using args_pack_type = type_pack<Args...>;

      static constexpr
      bool
      is_nothrow = std::is_nothrow_invocable_v<function_type, Args...>;

      template <typename F>
      struct is_compatible_function_type
        : std::bool_constant<std::conditional_t<
            is_nothrow,
            std::is_nothrow_invocable_r<result_type, F, Args...>,
            std::is_invocable_r<result_type, F, Args...>>::value>
      { };

      unbound_function_base            (void)                             = default;
      unbound_function_base            (const unbound_function_base&)     = default;
      unbound_function_base            (unbound_function_base&&) noexcept = default;
      unbound_function_base& operator= (const unbound_function_base&)     = default;
      unbound_function_base& operator= (unbound_function_base&&) noexcept = default;
      ~unbound_function_base           (void)                             = default;

      constexpr explicit
      unbound_function_base (function_type *function) noexcept
        : m_function (function)
      { }

      constexpr
      unbound_function_base&
      operator= (function_type *function) noexcept
      {
        m_function = function;
        return *this;
      }

      constexpr
      result_type
      call (Args... args) const
        noexcept (is_nothrow)
      {
        return gch::invoke (m_function, std::forward<Args> (args)...);
      }

      constexpr
      result_type
      operator() (Args... args) const
        noexcept (is_nothrow)
      {
        return call (std::forward<Args> (args)...);
      }

      template <auto F,
                std::enable_if_t<is_compatible_function_type<decltype (F)>::value> * = nullptr>
      static constexpr
      function_type *
      create_invoker_impl (void) noexcept
      {
        return [](Args... args) noexcept (is_nothrow) -> result_type
               {
                 return gch::invoke (F, std::forward<Args> (args)...);
               };
      }

      [[nodiscard]]
      constexpr
      function_type *
      get_target (void) const noexcept
      {
        return m_function;
      }

      constexpr
      void
      swap_functions (unbound_function_base& other) noexcept
      {
        using std::swap;
        swap (m_function, other.m_function);
      }

    private:
      function_type *m_function;
    };

  } // namespace gch::detail

  template <typename Function>
  class unbound_function
    : detail::unbound_function_base<Function, function_args_t<Function>>
  {
    using base = detail::unbound_function_base<Function, function_args_t<Function>>;

  public:
    using function_type  = typename base::function_type;
    using result_type    = typename base::result_type;
    using args_pack_type = typename base::args_pack_type;

    template <auto F>
    static constexpr
    bool
    is_compatible_function = base::template is_compatible_function_type<decltype (F)>::value;

    using base::is_nothrow;
    using base::operator();
    using base::call;

    unbound_function            (void)                        = default;
    unbound_function            (const unbound_function&)     = default;
    unbound_function            (unbound_function&&) noexcept = default;
    unbound_function& operator= (const unbound_function&)     = default;
    unbound_function& operator= (unbound_function&&) noexcept = default;
    ~unbound_function           (void)                        = default;

    constexpr
    unbound_function (std::nullptr_t) noexcept
      : base (nullptr)
    { }

    constexpr
    unbound_function (function_type f) noexcept
      : base (f)
    { }

    template <auto F, std::enable_if_t<is_compatible_function<F>> * = nullptr>
    constexpr
    unbound_function (static_function<F>) noexcept
      : base (create_invoker<F> ())
    { }

    constexpr
    unbound_function&
    operator= (std::nullptr_t) noexcept
    {
      base::operator= (nullptr);
      return *this;
    }

    constexpr
    unbound_function&
    operator= (function_type f) noexcept
    {
      base::operator= (f);
      return *this;
    }

    constexpr
    unbound_function&
    operator= (std::reference_wrapper<function_type> ref) noexcept
    {
      base::operator= (ref.get ());
      return *this;
    }

    template <auto F, std::enable_if_t<is_compatible_function<F>> * = nullptr>
    constexpr
    unbound_function&
    operator= (static_function<F>) noexcept
    {
      base::operator= (create_invoker<F> ());
      return *this;
    }

    constexpr
    void
    assign (std::nullptr_t) noexcept
    {
      *this = nullptr;
    }

    constexpr
    void
    assign (function_type f) noexcept
    {
      *this = f;
    }

    template <auto F, std::enable_if_t<is_compatible_function<F>> * = nullptr>
    constexpr
    void
    assign (void) noexcept
    {
      *this = (create_invoker<F> ());
    }

    constexpr
    explicit
    operator bool (void) const noexcept
    {
      return nullptr != target ();
    }

    [[nodiscard]]
    const std::type_info&
    target_type (void) const noexcept
    {
      return typeid (function_type *);
    }

    [[nodiscard]]
    constexpr
    function_type *
    target (void) const noexcept
    {
      return base::get_target ();
    }

    constexpr
    void
    swap (unbound_function& other) noexcept
    {
      base::swap_functions (other);
    }

    template <auto F, std::enable_if_t<is_compatible_function<F>> * = nullptr>
    static constexpr
    function_type *
    create_invoker (void) noexcept
    {
      return base::template create_invoker_impl<F> ();
    }
  };

  template <typename Function>
  constexpr
  void
  swap (unbound_function<Function>& lhs, unbound_function<Function>& rhs) noexcept
  {
    lhs.swap (rhs);
  }

  template <typename FunctionLHS, typename FunctionRHS>
  constexpr
  bool
  operator== (unbound_function<FunctionLHS> lhs, unbound_function<FunctionRHS> rhs) noexcept
  {
    return lhs.target () == rhs.target ();
  }

  template <typename FunctionLHS, typename FunctionRHS>
  constexpr
  bool
  operator!= (unbound_function<FunctionLHS> lhs, unbound_function<FunctionRHS> rhs) noexcept
  {
    return ! (lhs == rhs);
  }

  template <typename Function>
  constexpr
  bool
  operator!= (unbound_function<Function> lhs, unbound_function<Function> rhs) noexcept
  {
    return ! (lhs == rhs);
  }

  template <typename Function>
  constexpr
  bool
  operator== (unbound_function<Function> lhs, std::nullptr_t) noexcept
  {
    return ! lhs;
  }

  template <typename Function>
  constexpr
  bool
  operator== (std::nullptr_t, unbound_function<Function> rhs) noexcept
  {
    return ! rhs;
  }

  template <typename Function>
  constexpr
  bool
  operator!= (unbound_function<Function> lhs, std::nullptr_t) noexcept
  {
    return ! (lhs == nullptr);
  }

  template <typename Function>
  constexpr
  bool
  operator!= (std::nullptr_t, unbound_function<Function> rhs) noexcept
  {
    return ! (nullptr == rhs);
  }

  template <auto F>
  struct static_unbound_function
  {
    static constexpr
    unbound_function<unified_equivalent_function_t<decltype (F)>>
    value { static_function_v<F> };
  };

  template <auto F>
  inline constexpr
  auto
  static_unbound_function_v = static_unbound_function<F>::value;

  template <typename Function>
  unbound_function (Function) -> unbound_function<Function>;

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

  template <template <typename ...> typename MapperT, typename Pack, typename ...Args>
  struct common_map_pack_result
  { };

  template <template <typename ...> typename MapperT, template <typename ...> typename PackT,
            typename ...Ts, typename ...Args>
  struct common_map_pack_result<MapperT, PackT<Ts...>, Args...>
    : std::common_type<std::remove_reference_t<std::invoke_result_t<MapperT<Ts>, Args...>>...>
  { };

  template <template <typename ...> typename MapperT, typename Pack, typename ...Args>
  using common_map_pack_result_t
    = typename common_map_pack_result<MapperT, Pack, Args...>::type;

  namespace detail
  {

    template <template <typename ...> typename MapperT, typename Pack>
    struct map_pack_impl
    { };

    template <template <typename ...> typename MapperT,
              template <typename ...> typename PackT,
              typename ...Ts>
    struct map_pack_impl<MapperT, PackT<Ts...>>
    {
      template <typename ...Args>
      constexpr
      std::array<common_map_pack_result_t<MapperT, PackT<Ts...>, const Args&...>, sizeof...(Ts)>
      operator() (const Args&... args)
        noexcept (noexcept (
          std::array<common_map_pack_result_t<MapperT, PackT<Ts...>, const Args&...>,
                     sizeof...(Ts)> {
            gch::invoke (MapperT<Ts> { }, args...)... }))
      {
        return { gch::invoke (MapperT<Ts> { }, args...)... };
      }
    };

    template <typename Ret, template <typename ...> typename MapperT, typename Pack>
    struct map_pack_impl_r
    { };

    template <typename Ret,
              template <typename ...> typename MapperT,
              template <typename ...> typename PackT,
              typename ...Ts>
    struct map_pack_impl_r<Ret, MapperT, PackT<Ts...>>
    {
      template <typename ...Args>
      constexpr
      std::array<Ret, sizeof...(Ts)>
      operator() (const Args&... args)
        noexcept (noexcept (
          std::array<Ret, sizeof...(Ts)> { gch::invoke_r<Ret> (MapperT<Ts> { }, args...)... }))
      {
        return { gch::invoke_r<Ret> (MapperT<Ts> { }, args...)... };
      }
    };

  } // namespace gch::detail

  template <template <typename ...> typename MapperT, typename Pack, typename ...Args>
  constexpr
  std::array<common_map_pack_result_t<MapperT, Pack, const Args&...>, pack_size_v<Pack>>
  map_pack (const Args&... args)
  {
    return gch::invoke (detail::map_pack_impl<MapperT, Pack> { }, args...);
  }

  template <typename Ret, template <typename ...> typename MapperT, typename Pack, typename ...Args>
  constexpr
  std::array<Ret, pack_size_v<Pack>>
  map_pack (const Args&... args)
  {
    return gch::invoke (detail::map_pack_impl_r<Ret, MapperT, Pack> { }, args...);
  }

}

#endif // OCTAVE_IR_UTILITIES_IR_FUNCTIONAL_HPP

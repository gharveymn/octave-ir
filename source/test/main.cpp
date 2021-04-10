/** main.cpp
 * Short description here.
 *
 * Copyright © 2019 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-ir-compiler.hpp"
#include "gch/octave-ir-utilities/ir-functional.hpp"
#include <functional>
#include <utility>

struct test_struct
{
  void
  f (void) &&
  { }

  void
  g (void) const volatile && noexcept
  { }

  int x;
};

short
func (int, long)
{ }

short
func1 (int, long = 0);

template <typename F, typename ...Args>
struct compat;

template <typename M, typename T, typename ...Args>
struct compat<M T::*, Args...>
  : std::is_invocable<M T::*, Args...>
{ };

template <typename Function>
struct function_result_impl1
  : gch::pack_push_front<gch::function_args_t<Function>, Function>
{ };

template <typename M, typename T>
struct function_result_impl1<M T::*>
  : gch::pack_concatenate<gch::type_pack<M T::*, gch::match_function_cvref_t<M, T>>,
                          gch::function_args_t<M>>
{ };

int
main (void)
{
  test_struct ts;
  auto bf = gch::bind_front (&test_struct::f, ts);
  auto bff = std::bind_front (&test_struct::f, ts);
  // static_assert (std::is_same_v<void, decltype (bf)>);
  // static_assert (std::is_same_v<void, decltype (bff)>);
  std::invoke (std::move (bff));

  auto bf1 = gch::bind_front (&test_struct::x, ts);
  int x = gch::invoke (bf1);

  static_assert (std::is_same_v<short, gch::function_result_t<decltype (func)>>);
  static_assert (std::is_same_v<short, gch::function_result_t<decltype (&func)>>);
  static_assert (std::is_same_v<decltype (&test_struct::f), std::remove_pointer_t<decltype (&test_struct::f)>>);
  static_assert (std::is_same_v<decltype (&test_struct::f), std::decay_t<decltype (&test_struct::f)>>);
  static_assert (std::is_same_v<short, std::invoke_result_t<decltype (&func), int, long>>);

  static_assert (gch::is_rvref_qualified_v<decltype (&test_struct::f)>);

  auto p = gch::static_unbound_function_v<&test_struct::g>;

  static_assert (std::is_same_v<void (const volatile test_struct&&) noexcept,
                                gch::unified_equivalent_function_t<decltype (&test_struct::g)>>);

  static_assert (std::is_same_v<void (const volatile test_struct&&) noexcept,
                                decltype (p)::function_type>);

  gch::unbound_function<void (void)> uff ([] { });
  gch::unbound_function<void (test_struct&&)> uf (gch::static_function_v<&test_struct::f>);
  uf (test_struct { });

  return 0;
}

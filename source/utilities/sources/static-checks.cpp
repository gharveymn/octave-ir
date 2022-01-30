/** static-checks.cpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-optional-util.hpp"
#include "ir-type-pack.hpp"
#include "ir-type-traits.hpp"

#include <gch/optional_ref.hpp>

#include <optional>
#include <type_traits>
#include <utility>

namespace gch
{

  static_assert (0 == pack_index_v<type_pack<int>, int>);
  static_assert (0 == pack_index_v<type_pack<int, long>, int>);
  static_assert (1 == pack_index_v<type_pack<int, long>, long>);

  static_assert (! pack_contains_v<type_pack<>, int>);
  static_assert (  pack_contains_v<type_pack<int>, int>);
  static_assert (  pack_contains_v<type_pack<long, int>, int>);
  static_assert (  pack_contains_v<type_pack<int, long>, int>);
  static_assert (! pack_contains_v<type_pack<long>, int>);

  static_assert (std::is_same_v<type_pack<int, long>,
                                pack_concatenate_t<type_pack<int, long>>>);

  static_assert (std::is_same_v<type_pack<int, long, char, short>,
                                pack_concatenate_t<type_pack<int, long>,
                                                          type_pack<char, short>>>);

  static_assert (std::is_same_v<type_pack<int, long, char, short, bool, int>,
                                pack_concatenate_t<type_pack<int, long>,
                                                          type_pack<char, short>,
                                                          type_pack<bool, int>>>);

  static_assert (std::is_same_v<type_pack<int, long, void>,
                                pack_flatten_t<type_pack<int, type_pack<long, void>>>>);

  static_assert (std::is_same_v<type_pack<int, long, char, short>,
                                pack_flatten_t<type_pack<type_pack<int, long>, char, short>>>);

  static_assert (std::is_same_v<type_pack<int, long, char, short>,
                                pack_flatten_t<
                                  type_pack<type_pack<int, type_pack<long>, type_pack<>>,
                                             type_pack<type_pack<type_pack<char>>>,
                                             short>>>);

  static_assert (std::is_same_v<type_pack<int>, pack_unique_t<type_pack<int>>>);
  static_assert (std::is_same_v<type_pack<int>, pack_unique_t<type_pack<int, int>>>);
  static_assert (pack_equivalent_v<type_pack<int, long>, pack_unique_t<type_pack<int, long, int>>>);

  static_assert (  pack_equivalent_v<type_pack<>, type_pack<>>);
  static_assert (! pack_equivalent_v<type_pack<>, type_pack<int>>);
  static_assert (! pack_equivalent_v<type_pack<int>, type_pack<>>);
  static_assert (  pack_equivalent_v<type_pack<int>, type_pack<int>>);
  static_assert (! pack_equivalent_v<type_pack<int>, type_pack<long>>);
  static_assert (  pack_equivalent_v<type_pack<int, long>, type_pack<int, long>>);
  static_assert (  pack_equivalent_v<type_pack<long, int>, type_pack<int, long>>);
  static_assert (! pack_equivalent_v<type_pack<int, char>, type_pack<int, long>>);
  static_assert (! pack_equivalent_v<type_pack<char, int>, type_pack<int, long>>);
  static_assert (! pack_equivalent_v<type_pack<int, long, char>, type_pack<int, long>>);

  static_assert (std::is_same_v<match_cvref_t<               int, long>,                long>);
  static_assert (std::is_same_v<match_cvref_t<const          int, long>, const          long>);
  static_assert (std::is_same_v<match_cvref_t<      volatile int, long>,       volatile long>);
  static_assert (std::is_same_v<match_cvref_t<const volatile int, long>, const volatile long>);

  static_assert (std::is_same_v<match_cvref_t<               int&, long>,                long&>);
  static_assert (std::is_same_v<match_cvref_t<const          int&, long>, const          long&>);
  static_assert (std::is_same_v<match_cvref_t<      volatile int&, long>,       volatile long&>);
  static_assert (std::is_same_v<match_cvref_t<const volatile int&, long>, const volatile long&>);

  static_assert (std::is_same_v<match_cvref_t<               int&&, long>,                long&&>);
  static_assert (std::is_same_v<match_cvref_t<const          int&&, long>, const          long&&>);
  static_assert (std::is_same_v<match_cvref_t<      volatile int&&, long>,       volatile long&&>);
  static_assert (std::is_same_v<match_cvref_t<const volatile int&&, long>, const volatile long&&>);

  static_assert (std::is_same_v<type_pack<>,          pack_reverse_t<type_pack<>>>);
  static_assert (std::is_same_v<type_pack<int>,       pack_reverse_t<type_pack<int>>>);
  static_assert (std::is_same_v<type_pack<int, long>, pack_reverse_t<type_pack<long, int>>>);

  static_assert (std::is_same_v<type_pack<>,            pack_erase_t<type_pack<int>, 0>>);
  static_assert (std::is_same_v<type_pack<int,  short>, pack_erase_t<type_pack<long, int, short>, 0>>);
  static_assert (std::is_same_v<type_pack<long, short>, pack_erase_t<type_pack<long, int, short>, 1>>);
  static_assert (std::is_same_v<type_pack<long, int>,   pack_erase_t<type_pack<long, int, short>, 2>>);

  static_assert (std::is_same_v<type_pack<>,            pack_remove_t<type_pack<>>>);
  static_assert (std::is_same_v<type_pack<int>,         pack_remove_t<type_pack<int>>>);
  static_assert (std::is_same_v<type_pack<>,            pack_remove_t<type_pack<int>, int>>);
  static_assert (std::is_same_v<type_pack<>,            pack_remove_t<type_pack<int, int>, int>>);
  static_assert (std::is_same_v<type_pack<short, long>, pack_remove_t<type_pack<int, short, int, long>, int>>);
  static_assert (std::is_same_v<type_pack<short, long>, pack_remove_t<type_pack<short, int, long, int>, int>>);
  static_assert (std::is_same_v<type_pack<short, long>, pack_remove_t<type_pack<short, long>, int>>);
  static_assert (std::is_same_v<type_pack<long>,        pack_remove_t<type_pack<int, short, int, long>, int, short>>);

  static_assert (std::is_same_v<type_pack<>,           pack_pop_front_t<type_pack<int>>>);
  static_assert (std::is_same_v<type_pack<int, short>, pack_pop_front_t<type_pack<long, int, short>>>);

  static_assert (std::is_same_v<type_pack<>,          pack_pop_back_t<type_pack<int>>>);
  static_assert (std::is_same_v<type_pack<long, int>, pack_pop_back_t<type_pack<long, int, short>>>);

  static_assert (std::is_same_v<type_pack<int>,              pack_insert_t<type_pack<>, 0, int>>);
  static_assert (std::is_same_v<type_pack<long, int, short>, pack_insert_t<type_pack<int,  short>, 0, long>>);
  static_assert (std::is_same_v<type_pack<long, int, short>, pack_insert_t<type_pack<long, short>, 1, int>>);
  static_assert (std::is_same_v<type_pack<long, int, short>, pack_insert_t<type_pack<long, int>,   2, short>>);

  static_assert (std::is_same_v<type_pack<int>,              pack_push_front_t<type_pack<>, int>>);
  static_assert (std::is_same_v<type_pack<long, int, short>, pack_push_front_t<type_pack<int, short>, long>>);

  static_assert (std::is_same_v<type_pack<int>,              pack_push_back_t<type_pack<>, int>>);
  static_assert (std::is_same_v<type_pack<long, int, short>, pack_push_back_t<type_pack<long, int>, short>>);

  static_assert (std::is_same_v<int *, remove_all_const_t<int *>>);
  static_assert (std::is_same_v<int *, remove_all_const_t<const int *>>);
  static_assert (std::is_same_v<int *, remove_all_const_t<const int * const>>);
  static_assert (std::is_same_v<int *, remove_all_const_t<int * const>>);

  static_assert (std::is_same_v<int&, remove_all_const_t<int&>>);
  static_assert (std::is_same_v<int&, remove_all_const_t<const int&>>);

  static_assert (std::is_same_v<int&&, remove_all_const_t<int&&>>);
  static_assert (std::is_same_v<int&&, remove_all_const_t<const int&&>>);

  static_assert (std::is_same_v<int *[], remove_all_const_t<int *[]>>);
  static_assert (std::is_same_v<int *[], remove_all_const_t<const int *[]>>);

  static_assert (std::is_same_v<int (&)[], remove_all_const_t<int (&)[]>>);
  static_assert (std::is_same_v<int (&)[], remove_all_const_t<const int (&)[]>>);

  static_assert (std::is_same_v<int *[2], remove_all_const_t<int *[2]>>);
  static_assert (std::is_same_v<int *[2], remove_all_const_t<const int *[2]>>);

  static_assert (std::is_same_v<int (&)[2], remove_all_const_t<int (&)[2]>>);
  static_assert (std::is_same_v<int (&)[2], remove_all_const_t<const int (&)[2]>>);

  static_assert (std::is_same_v<int *, remove_all_volatile_t<int *>>);
  static_assert (std::is_same_v<int *, remove_all_volatile_t<volatile int *>>);
  static_assert (std::is_same_v<int *, remove_all_volatile_t<volatile int * volatile>>);
  static_assert (std::is_same_v<int *, remove_all_volatile_t<int * volatile>>);

  static_assert (std::is_same_v<int&, remove_all_volatile_t<int&>>);
  static_assert (std::is_same_v<int&, remove_all_volatile_t<volatile int&>>);

  static_assert (std::is_same_v<int&&, remove_all_volatile_t<int&&>>);
  static_assert (std::is_same_v<int&&, remove_all_volatile_t<volatile int&&>>);

  static_assert (std::is_same_v<int *[], remove_all_volatile_t<int *[]>>);
  static_assert (std::is_same_v<int *[], remove_all_volatile_t<volatile int *[]>>);

  static_assert (std::is_same_v<int (&)[], remove_all_volatile_t<int (&)[]>>);
  static_assert (std::is_same_v<int (&)[], remove_all_volatile_t<volatile int (&)[]>>);

  static_assert (std::is_same_v<int *[2], remove_all_volatile_t<int *[2]>>);
  static_assert (std::is_same_v<int *[2], remove_all_volatile_t<volatile int *[2]>>);

  static_assert (std::is_same_v<int (&)[2], remove_all_volatile_t<int (&)[2]>>);
  static_assert (std::is_same_v<int (&)[2], remove_all_volatile_t<volatile int (&)[2]>>);

  static_assert (std::is_same_v<int *, remove_all_cv_t<int *>>);

  static_assert (std::is_same_v<int *, remove_all_cv_t<const int *>>);
  static_assert (std::is_same_v<int *, remove_all_cv_t<const volatile int *>>);
  static_assert (std::is_same_v<int *, remove_all_cv_t<const volatile int * volatile>>);
  static_assert (std::is_same_v<int *, remove_all_cv_t<const int * volatile>>);

  static_assert (std::is_same_v<int *, remove_all_cv_t<const int * const>>);
  static_assert (std::is_same_v<int *, remove_all_cv_t<const volatile int * const>>);
  static_assert (std::is_same_v<int *, remove_all_cv_t<const volatile int * const volatile>>);
  static_assert (std::is_same_v<int *, remove_all_cv_t<const int * const volatile>>);

  static_assert (std::is_same_v<int *, remove_all_cv_t<int * const>>);
  static_assert (std::is_same_v<int *, remove_all_cv_t<volatile int * const>>);
  static_assert (std::is_same_v<int *, remove_all_cv_t<volatile int * const volatile>>);
  static_assert (std::is_same_v<int *, remove_all_cv_t<int * const volatile>>);

  static_assert (std::is_same_v<int *, remove_all_cv_t<volatile int *>>);
  static_assert (std::is_same_v<int *, remove_all_cv_t<volatile int * volatile>>);
  static_assert (std::is_same_v<int *, remove_all_cv_t<int * volatile>>);

  static_assert (std::is_same_v<int&, remove_all_cv_t<int&>>);
  static_assert (std::is_same_v<int&, remove_all_cv_t<const int&>>);
  static_assert (std::is_same_v<int&, remove_all_cv_t<volatile int&>>);
  static_assert (std::is_same_v<int&, remove_all_cv_t<const volatile int&>>);

  static_assert (std::is_same_v<int&&, remove_all_cv_t<int&&>>);
  static_assert (std::is_same_v<int&&, remove_all_cv_t<const int&&>>);
  static_assert (std::is_same_v<int&&, remove_all_cv_t<volatile int&&>>);
  static_assert (std::is_same_v<int&&, remove_all_cv_t<const volatile int&&>>);

  static_assert (std::is_same_v<int *[], remove_all_cv_t<int *[]>>);
  static_assert (std::is_same_v<int *[], remove_all_cv_t<const int *[]>>);
  static_assert (std::is_same_v<int *[], remove_all_cv_t<volatile int *[]>>);
  static_assert (std::is_same_v<int *[], remove_all_cv_t<const volatile int *[]>>);

  static_assert (std::is_same_v<int (&)[], remove_all_cv_t<int (&)[]>>);
  static_assert (std::is_same_v<int (&)[], remove_all_cv_t<const int (&)[]>>);
  static_assert (std::is_same_v<int (&)[], remove_all_cv_t<volatile int (&)[]>>);
  static_assert (std::is_same_v<int (&)[], remove_all_cv_t<const volatile int (&)[]>>);

  static_assert (std::is_same_v<int *[2], remove_all_cv_t<int *[2]>>);
  static_assert (std::is_same_v<int *[2], remove_all_cv_t<const int *[2]>>);
  static_assert (std::is_same_v<int *[2], remove_all_cv_t<volatile int *[2]>>);
  static_assert (std::is_same_v<int *[2], remove_all_cv_t<const volatile int *[2]>>);

  static_assert (std::is_same_v<int (&)[2], remove_all_cv_t<int (&)[2]>>);
  static_assert (std::is_same_v<int (&)[2], remove_all_cv_t<const int (&)[2]>>);
  static_assert (std::is_same_v<int (&)[2], remove_all_cv_t<volatile int (&)[2]>>);
  static_assert (std::is_same_v<int (&)[2], remove_all_cv_t<const volatile int (&)[2]>>);

  static_assert (std::is_same_v<const int * const, make_all_levels_const_t<int *>>);
  static_assert (std::is_same_v<const int * const, make_all_levels_const_t<const int *>>);
  static_assert (std::is_same_v<const int * const, make_all_levels_const_t<int * const>>);
  static_assert (std::is_same_v<const int * const, make_all_levels_const_t<const int * const>>);

  static_assert (std::is_same_v<const int * const&, make_all_levels_const_t<int *&>>);
  static_assert (std::is_same_v<const int * const&, make_all_levels_const_t<const int *&>>);
  static_assert (std::is_same_v<const int * const&, make_all_levels_const_t<int * const&>>);
  static_assert (std::is_same_v<const int * const&, make_all_levels_const_t<const int * const&>>);

  static_assert (std::is_same_v<const int&, make_all_levels_const_t<int&>>);
  static_assert (std::is_same_v<const int&, make_all_levels_const_t<const int&>>);

  static_assert (std::is_same_v<const int&&, make_all_levels_const_t<int&&>>);
  static_assert (std::is_same_v<const int&&, make_all_levels_const_t<const int&&>>);

  static_assert (std::is_same_v<const int (&)[], make_all_levels_const_t<int (&)[]>>);
  static_assert (std::is_same_v<const int (&)[], make_all_levels_const_t<int (&)[]>>);
  static_assert (std::is_same_v<const int (&)[], make_all_levels_const_t<int (&)[]>>);
  static_assert (std::is_same_v<const int (&)[], make_all_levels_const_t<int (&)[]>>);

  static_assert (std::is_same_v<const int (&)[2], make_all_levels_const_t<int (&)[2]>>);
  static_assert (std::is_same_v<const int (&)[2], make_all_levels_const_t<int (&)[2]>>);
  static_assert (std::is_same_v<const int (&)[2], make_all_levels_const_t<int (&)[2]>>);
  static_assert (std::is_same_v<const int (&)[2], make_all_levels_const_t<int (&)[2]>>);

  static constexpr int zint = 6;

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

    constexpr const std::optional<const optional_ref<const int>> z { std::in_place, zint };

    static_assert (std::is_same_v<const optional_ref<const int>, flattened_optional_t<decltype (z)>>);
    static_assert (std::is_same_v<const optional_ref<const int>, decltype (flatten_optional (z))>);
    static_assert (flatten_optional (z).has_value ());
    static_assert (flatten_optional (z) == zint);
  }

}

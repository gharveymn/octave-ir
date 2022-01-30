/** ir-type-pack.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_UTILITIES_IR_TYPE_PACK_HPP
#define OCTAVE_IR_UTILITIES_IR_TYPE_PACK_HPP

#include <type_traits>

namespace gch
{

  template <typename ...Ts>
  struct type_pack
  {
    using type = type_pack<Ts...>;
  };

  template <typename Pack>
  struct is_type_pack;

  template <typename Pack>
  inline constexpr
  bool
  is_type_pack_v = is_type_pack<Pack>::value;

  template <typename Pack>
  struct pack_size;

  template <typename Pack>
  inline constexpr
  std::size_t
  pack_size_v = pack_size<Pack>::value;

  template <typename Pack>
  struct pack_empty;

  template <typename Pack>
  inline constexpr bool pack_empty_v = pack_empty<Pack>::value;

  template <typename Pack, std::size_t I>
  struct pack_select;

  template <typename Pack, std::size_t I>
  using pack_select_t = typename pack_select<Pack, I>::type;

  template <typename Pack>
  struct pack_front;

  template <typename Pack>
  using pack_front_t = typename pack_front<Pack>::type;

  template <typename Pack>
  struct pack_back;

  template <typename Pack>
  using pack_back_t = typename pack_back<Pack>::type;

  template <typename Pack, typename T>
  struct pack_index;

  template <typename Pack, typename T>
  inline constexpr
  std::size_t
  pack_index_v = pack_index<Pack, T>::value;

  template <typename Pack, typename T>
  struct pack_contains;

  template <typename Pack, typename T>
  inline constexpr
  bool
  pack_contains_v = pack_contains<Pack, T>::value;

  template <typename ...Packs>
  struct pack_concatenate;

  template <typename ...Packs>
  using pack_concatenate_t = typename pack_concatenate<Packs...>::type;

  template <typename Pack>
  struct pack_flatten;

  template <typename Pack>
  using pack_flatten_t = typename pack_flatten<Pack>::type;

  template <typename Pack>
  struct pack_unique;

  template <typename Pack>
  using pack_unique_t = typename pack_unique<Pack>::type;

  template <typename ...Pack>
  struct pack_union;

  template <typename ...Pack>
  using pack_union_t = typename pack_union<Pack...>::type;

  template <typename ...Packs>
  struct pack_equivalent;

  template <typename ...Packs>
  inline constexpr
  bool
  pack_equivalent_v = pack_equivalent<Packs...>::value;

  template <typename Pack>
  struct pack_homogenenous;

  template <typename Pack>
  inline constexpr
  bool
  pack_homogenenous_v = pack_homogenenous<Pack>::value;

  template <typename Pack, template <typename ...> typename TT>
  struct pack_apply;

  template <typename Pack, template <typename ...> typename TT>
  using pack_apply_t = typename pack_apply<Pack, TT>::type;

  template <typename Pack, template <typename> typename TT>
  struct pack_for_each;

  template <typename Pack, template <typename> typename TT>
  using pack_for_each_t = typename pack_for_each<Pack, TT>::type;

  template <typename Pack>
  struct pack_reverse;

  template <typename Pack>
  using pack_reverse_t = typename pack_reverse<Pack>::type;

  template <typename Pack, std::size_t Index>
  struct pack_erase;

  template <typename Pack, std::size_t Index>
  using pack_erase_t = typename pack_erase<Pack, Index>::type;

  template <typename Pack, typename ...Ts>
  struct pack_remove;

  template <typename Pack, typename ...Ts>
  using pack_remove_t = typename pack_remove<Pack, Ts...>::type;

  template <typename Pack>
  struct pack_pop_front;

  template <typename Pack>
  using pack_pop_front_t = typename pack_pop_front<Pack>::type;

  template <typename Pack>
  struct pack_pop_back;

  template <typename Pack>
  using pack_pop_back_t = typename pack_pop_back<Pack>::type;

  template <typename Pack, std::size_t Index, typename T>
  struct pack_insert;

  template <typename Pack, std::size_t Index, typename T>
  using pack_insert_t = typename pack_insert<Pack, Index, T>::type;

  template <typename Pack, typename T>
  struct pack_push_front;

  template <typename Pack, typename T>
  using pack_push_front_t = typename pack_push_front<Pack, T>::type;

  template <typename Pack, typename T>
  struct pack_push_back;

  template <typename Pack, typename T>
  using pack_push_back_t = typename pack_push_back<Pack, T>::type;

  namespace detail
  {

    template <typename Pack>
    struct is_type_pack_impl
      : std::false_type
    { };

    template <template <typename ...> typename PackT, typename ...Ts>
    struct is_type_pack_impl<PackT<Ts...>>
      : std::true_type
    { };

    template <typename Pack>
    struct pack_size_impl
    { };

    template <template <typename ...> typename PackT, typename ...Ts>
    struct pack_size_impl<PackT<Ts...>>
      : std::integral_constant<std::size_t, sizeof...(Ts)>
    { };

    template <template <auto ...> typename PackT, auto ...Vs>
    struct pack_size_impl<PackT<Vs...>>
      : std::integral_constant<std::size_t, sizeof...(Vs)>
    { };

    template <typename Pack, std::size_t I>
    struct pack_select_impl
    { };

    template <template <typename ...> typename PackT, typename Head, typename ...Tail>
    struct pack_select_impl<PackT<Head, Tail...>, 0>
    {
      using type = Head;
    };

    template <std::size_t I,
              template <typename ...> typename PackT, typename Head, typename ...Tail>
    struct pack_select_impl<PackT<Head, Tail...>, I>
      : pack_select_impl<PackT<Tail...>, I - 1>
    { };

    template <typename Pack, typename T, std::size_t I = 0>
    struct pack_index_impl
    { };

    template <typename T,
              template <typename ...> typename PackT, typename ...Rest,
              std::size_t I>
    struct pack_index_impl<PackT<T, Rest...>, T, I>
      : std::integral_constant<std::size_t, I>
    { };

    template <template <typename ...> typename PackT, typename Head, typename ...Tail,
              typename T,
              std::size_t I>
    struct pack_index_impl<PackT<Head, Tail...>, T, I>
      : pack_index_impl<PackT<Tail...>, T, I + 1>
    { };

    template <typename Pack, typename T, typename Enable = void>
    struct pack_contains_impl
      : std::false_type
    { };

    template <typename Pack, typename T>
    struct pack_contains_impl<Pack, T, std::void_t<typename pack_index<Pack, T>::type>>
      : std::true_type
    { };

    template <typename ...Packs>
    struct pack_concatenate_impl
    { };

    template <typename Pack>
    struct pack_concatenate_impl<Pack>
    {
      using type = Pack;
    };

    template <template <typename ...> typename PackT,
              typename ...LHS, typename ...RHS, typename ...Rest>
    struct pack_concatenate_impl<PackT<LHS...>, PackT<RHS...>, Rest...>
      : pack_concatenate_impl<PackT<LHS..., RHS...>, Rest...>
    { };

    template <typename Pack>
    struct pack_flatten_impl
    {
      using type = Pack;
    };

    template <template <typename ...> typename PackT, typename Head, typename ...Tail>
    struct pack_flatten_impl<PackT<Head, Tail...>>
      : pack_concatenate<PackT<Head>, pack_flatten_t<PackT<Tail...>>>
    { };

    template <template <typename ...> typename PackT, typename... Ts, typename ...Tail>
    struct pack_flatten_impl<PackT<PackT<Ts...>, Tail...>>
      : pack_concatenate<pack_flatten_t<PackT<Ts>>..., pack_flatten_t<PackT<Tail...>>>
    { };

    template <typename PackIn, typename PackOut>
    struct pack_unique_helper
    { };

    template <template <typename ...> typename PackT, typename ...OutTs>
    struct pack_unique_helper<PackT<>, PackT<OutTs...>>
    {
      using type = PackT<OutTs...>;
    };

    template <template <typename ...> typename PackT,
              typename InHead, typename ...InTail,
              typename ...OutTs>
    struct pack_unique_helper<PackT<InHead, InTail...>, PackT<OutTs...>>
      : pack_unique_helper<PackT<InTail...>,
                           std::conditional_t<pack_contains_v<PackT<OutTs...>, InHead>,
                                              PackT<OutTs...>,
                                              PackT<OutTs..., InHead>>>
    { };

    template <typename Pack>
    struct pack_unique_impl
    { };

    template <template <typename ...> typename PackT, typename ...Ts>
    struct pack_unique_impl<PackT<Ts...>>
      : pack_unique_helper<PackT<Ts...>, PackT<>>
    { };

    template <typename ...Packs>
    struct pack_equivalent_impl
      : std::false_type
    { };

    template <template <typename ...> typename PackTLHS, typename PackRHS>
    struct pack_equivalent_impl<PackTLHS<>, PackRHS>
      : pack_empty<PackRHS>
    { };

    template <template <typename ...> typename PackTLHS, typename ...TsLHS, typename PackRHS>
    struct pack_equivalent_impl<PackTLHS<TsLHS...>, PackRHS>
      : std::conjunction<pack_contains<PackRHS, TsLHS>...>
    { };

    template <typename PackLHS, typename ...Packs>
    struct pack_equivalent_impl<PackLHS, Packs...>
      : std::conjunction<pack_equivalent_impl<PackLHS, Packs>...>
    { };

    template <typename Pack>
    struct pack_homogenenous_impl
      : std::false_type
    { };

    template <template <typename ...> typename PackT, typename Head, typename ...Tail>
    struct pack_homogenenous_impl<PackT<Head, Tail...>>
      : std::conjunction<std::is_same<Head, Tail>...>
    { };

    template <typename Pack, template <typename ...> typename TT>
    struct pack_apply_impl
    { };

    template <template <typename ...> typename PackT, typename ...Ts,
              template <typename ...> typename TT>
    struct pack_apply_impl<PackT<Ts...>, TT>
    {
      using type = TT<Ts...>;
    };

    template <typename Pack, template <typename> typename TT>
    struct pack_for_each_impl
    { };

    template <template <typename ...> typename PackT, typename ...Ts,
              template <typename> typename TT>
    struct pack_for_each_impl<PackT<Ts...>, TT>
    {
      using type = PackT<TT<Ts>...>;
    };

    template <typename PackIn, typename PackOut>
    struct pack_reverse_helper
    { };

    template <template <typename ...> typename PackT, typename ...OutTs>
    struct pack_reverse_helper<PackT<>, PackT<OutTs...>>
    {
      using type = PackT<OutTs...>;
    };

    template <template <typename ...> typename PackT,
              typename InHead, typename ...InTail,
              typename ...OutTs>
    struct pack_reverse_helper<PackT<InHead, InTail...>, PackT<OutTs...>>
      : pack_reverse_helper<PackT<InTail...>, PackT<InHead, OutTs...>>
    { };

    template <typename Pack>
    struct pack_reverse_impl
    { };

    template <template <typename ...> typename PackT, typename ...Ts>
    struct pack_reverse_impl<PackT<Ts...>>
      : pack_reverse_helper<PackT<Ts...>, PackT<>>
    { };

    template <std::size_t I, typename PackIn, typename PackOut, typename Enable = void>
    struct pack_erase_helper
    { };

    template <template <typename ...> typename PackT,
              typename PackLHS, typename RHSHead, typename ...RHSTail>
    struct pack_erase_helper<0, PackLHS, PackT<RHSHead, RHSTail...>>
    {
      using type = pack_concatenate_t<PackLHS, PackT<RHSTail...>>;
    };

    template <std::size_t I,
              template <typename ...> typename PackT,
              typename ...LHS, typename RHSHead, typename ...RHSTail>
    struct pack_erase_helper<I, PackT<LHS...>, PackT<RHSHead, RHSTail...>,
                              std::enable_if_t<I != 0>>
      : pack_erase_helper<I - 1, PackT<LHS..., RHSHead>, PackT<RHSTail...>>
    { };

    template <typename Pack, std::size_t I>
    struct pack_erase_impl
    { };

    template <template <typename ...> typename PackT, typename ...Ts, std::size_t I>
    struct pack_erase_impl<PackT<Ts...>, I>
      : pack_erase_helper<I, PackT<>, PackT<Ts...>>
    { };

    template <typename Pack, typename T, typename IndexType = pack_index<Pack, T>, typename = void>
    struct pack_remove_helper
    {
      using type = Pack;
    };

    template <typename Pack, typename T, typename IndexType>
    struct pack_remove_helper<Pack, T, IndexType, std::void_t<typename IndexType::type>>
      : pack_remove_helper<pack_erase_t<Pack, IndexType::value>, T>
    { };

    template <typename Pack, typename ...Ts>
    struct pack_remove_impl;

    template <typename Pack>
    struct pack_remove_impl<Pack>
    {
      using type = Pack;
    };

    template <typename Pack, typename T, typename ...Rest>
    struct pack_remove_impl<Pack, T, Rest...>
      : pack_remove_impl<typename pack_remove_helper<Pack, T>::type, Rest...>
    { };

    template <std::size_t I, typename T, typename PackLHS, typename PackRHS,
              typename Enable = void>
    struct pack_insert_helper
    { };

    template <typename T, template <typename ...> typename PackT,
              typename PackLHS, typename ...RHS>
    struct pack_insert_helper<0, T, PackLHS, PackT<RHS...>>
    {
      using type = pack_concatenate_t<PackLHS, PackT<T>, PackT<RHS...>>;
    };

    template <std::size_t I, typename T,
              template <typename ...> typename PackT,
              typename ...LHS, typename RHSHead, typename ...RHSTail>
    struct pack_insert_helper<I, T, PackT<LHS...>, PackT<RHSHead, RHSTail...>,
                              std::enable_if_t<I != 0>>
      : pack_insert_helper<I - 1, T, PackT<LHS..., RHSHead>, PackT<RHSTail...>>
    { };

    template <typename Pack, std::size_t I, typename T>
    struct pack_insert_impl;

    template <template <typename ...> typename PackT, typename ...Ts, std::size_t I, typename T>
    struct pack_insert_impl<PackT<Ts...>, I, T>
      : pack_insert_helper<I, T, PackT<>, PackT<Ts...>>
    { };

  } // namespace gch::detail

  template <typename Pack>
  struct pack_size
    : detail::pack_size_impl<Pack>
  { };

  template <typename Pack>
  struct pack_empty
    : std::bool_constant<pack_size_v<Pack> == 0>
  { };

  template <typename Pack>
  struct is_type_pack
    : detail::is_type_pack_impl<Pack>
  { };

  template <typename Pack, std::size_t I>
  struct pack_select
    : detail::pack_select_impl<Pack, I>
  { };

  template <typename Pack>
  struct pack_front
    : pack_select<Pack, 0>
  { };

  template <typename Pack>
  struct pack_back
    : pack_select<Pack, pack_size_v<Pack> - 1>
  { };

  template <typename Pack, typename T>
  struct pack_index
    : detail::pack_index_impl<Pack, T, 0>
  { };

  template <typename Pack, typename T>
  struct pack_contains
    : detail::pack_contains_impl<Pack, T>
  { };

  template <typename ...Packs>
  struct pack_concatenate
    : detail::pack_concatenate_impl<Packs...>
  { };

  template <typename Pack>
  struct pack_flatten
    : detail::pack_flatten_impl<Pack>
  { };

  template <typename ...Pack>
  struct pack_union
    : pack_unique<pack_concatenate_t<Pack...>>
  { };

  template <typename Pack>
  struct pack_unique
    : detail::pack_unique_impl<Pack>
  { };

  template <typename ...Packs>
  struct pack_equivalent
    : detail::pack_equivalent_impl<Packs...>
  { };

  template <typename Pack>
  struct pack_homogenenous
    : detail::pack_homogenenous_impl<Pack>
  { };

  template <typename Pack, template <typename ...> typename TT>
  struct pack_apply
    : detail::pack_apply_impl<Pack, TT>
  { };

  template <typename Pack, template <typename> typename TT>
  struct pack_for_each
    : detail::pack_for_each_impl<Pack, TT>
  { };

  template <typename Pack>
  struct pack_reverse
    : detail::pack_reverse_impl<Pack>
  { };

  template <typename Pack, std::size_t Index>
  struct pack_erase
    : detail::pack_erase_impl<Pack, Index>
  { };

  template <typename Pack, typename ...Ts>
  struct pack_remove
    : detail::pack_remove_impl<Pack, Ts...>
  { };

  template <typename Pack>
  struct pack_pop_front
    : pack_erase<Pack, 0>
  { };

  template <typename Pack>
  struct pack_pop_back
    : pack_erase<Pack, pack_size_v<Pack> - 1>
  { };

  template <typename Pack, std::size_t Index, typename T>
  struct pack_insert
    : detail::pack_insert_impl<Pack, Index, T>
  { };

  template <typename Pack, typename T>
  struct pack_push_front
    : pack_insert<Pack, 0, T>
  { };

  template <typename Pack, typename T>
  struct pack_push_back
    : pack_insert<Pack, pack_size_v<Pack>, T>
  { };

}

#endif // OCTAVE_IR_UTILITIES_IR_TYPE_PACK_HPP

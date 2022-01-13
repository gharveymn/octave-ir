/** ir-index-sequence-map.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_UTILITIES_IR_INDEX_SEQUENCE_MAP_HPP
#define OCTAVE_IR_UTILITIES_IR_INDEX_SEQUENCE_MAP_HPP

#include "ir-utility.hpp"

#include <optional>
#include <unordered_map>

namespace gch
{

  template <typename Iterator>
  struct map_insertion_pair
  {
    Iterator position;
    bool     inserted;
  };

  template <typename Iterator>
  constexpr
  decltype (auto)
  get_key (const map_insertion_pair<Iterator>& p) noexcept
  {
    return p.position->first;
  }

  template <typename Iterator>
  constexpr
  decltype (auto)
  get_value (const map_insertion_pair<Iterator>& p) noexcept
  {
    return p.position->second;
  }

  template <typename Key, typename IndexType = std::size_t>
  class ir_index_sequence_map
  {
  public:
    using key_type        = Key;
    using index_type      = IndexType;

    using container_type  = std::unordered_map<key_type, index_type>;
    using value_type      = typename container_type::value_type;
    using allocator_type  = typename container_type::allocator_type;
    using size_type       = typename container_type::size_type;
    using difference_type = typename container_type::difference_type;
    using reference       = typename container_type::reference;
    using const_reference = typename container_type::const_reference;
    using pointer         = typename container_type::pointer;
    using const_pointer   = typename container_type::const_pointer;

    using iterator        = typename container_type::iterator;
    using const_iterator  = typename container_type::const_iterator;

    using val_t   = value_type;
    using alloc_t = allocator_type;
    using size_ty = size_type;
    using diff_ty = difference_type;
    using ref     = reference;
    using cref    = const_reference;
    using ptr     = pointer;
    using cptr    = const_pointer;

    using iter    = iterator;
    using citer   = const_iterator;

    using insert_return_type = map_insertion_pair<const_iterator>;

    ir_index_sequence_map            (void)                             = default;
    ir_index_sequence_map            (const ir_index_sequence_map&)     = default;
    ir_index_sequence_map            (ir_index_sequence_map&&) noexcept = default;
    ir_index_sequence_map& operator= (const ir_index_sequence_map&)     = default;
    ir_index_sequence_map& operator= (ir_index_sequence_map&&) noexcept = default;
    ~ir_index_sequence_map           (void)                             = default;

    [[nodiscard]]
    index_type
    sequence_length (void) const noexcept
    {
      return static_cast<index_type> (m_map.size ());
    }

    template <typename ...Args,
              std::enable_if_t<std::is_constructible_v<key_type, Args...>> * = nullptr>
    insert_return_type
    emplace (Args&&... args)
    {
      return rebind_to_aggregate<insert_return_type> (
        m_map.try_emplace (key_type (std::forward<Args> (args)...), sequence_length ()));
    }

    [[nodiscard]]
    bool
    contains (const key_type& key) const
    {
      return m_map.find (key) != m_map.end ();
    }

    [[nodiscard]]
    std::optional<index_type>
    find (const key_type& key) const
    {
      if (auto found = m_map.find (key) ; found != m_map.end ())
        return { found->second };
      return std::nullopt;
    }

  private:
    container_type m_map;
  };

} // namespace gch

#endif // OCTAVE_IR_UTILITIES_IR_INDEX_SEQUENCE_MAP_HPP

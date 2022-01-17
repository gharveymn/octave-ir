/** ir-static-block.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_STATIC_IR_IR_STATIC_BLOCK_HPP
#define OCTAVE_IR_STATIC_IR_IR_STATIC_BLOCK_HPP

#include "ir-type.hpp"

#include "ir-common.hpp"
#include "ir-utility.hpp"

#include <iosfwd>
#include <vector>

namespace gch
{

  class ir_instruction;
  class ir_static_variable_map;
  class ir_timeline_origin_map;

  class ir_metadata;
  class ir_static_instruction;
  class ir_static_def;

  class ir_static_block_id
    : public named_type<std::size_t>
  {
  public:
    using named_type<std::size_t>::named_type;

    ir_static_block_id&
    operator++ (void) noexcept
    {
      return *this = ir_static_block_id { static_cast<value_type> (*this) + 1U };
    }

    ir_static_block_id
    operator++ (int) noexcept
    {
      ir_static_block_id ret { *this };
      ++(*this);
      return ret;
    }
  };

  inline constexpr
  ir_static_block_id
  ir_undefined_block_id { static_cast<std::size_t> (-1) };

  class ir_static_block
  {
  public:
    using container_type          = std::vector<ir_static_instruction>;
    using value_type              = container_type::value_type;
    using allocator_type          = container_type::allocator_type;
    using size_type               = container_type::size_type;
    using difference_type         = container_type::difference_type;
    using reference               = container_type::reference;
    using const_reference         = container_type::const_reference;
    using pointer                 = container_type::pointer;
    using const_pointer           = container_type::const_pointer;

    using iterator                = container_type::iterator;
    using const_iterator          = container_type::const_iterator;
    using reverse_iterator        = container_type::reverse_iterator;
    using const_reverse_iterator  = container_type::const_reverse_iterator;

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
    using riter   = reverse_iterator;
    using criter  = const_reverse_iterator;

    ir_static_block            (void)                       = default;
    ir_static_block            (const ir_static_block&)     = default;
    ir_static_block            (ir_static_block&&) noexcept = default;
    ir_static_block& operator= (const ir_static_block&)     = default;
    ir_static_block& operator= (ir_static_block&&) noexcept = default;
    ~ir_static_block           (void)                       = default;

    [[nodiscard]]
    const_iterator
    begin (void) const noexcept;

    [[nodiscard]]
    const_iterator
    end (void) const noexcept;

    [[nodiscard]]
    const_reverse_iterator
    rbegin (void) const noexcept;

    [[nodiscard]]
    const_reverse_iterator
    rend (void) const noexcept;

    [[nodiscard]]
    const_reference
    front (void) const;

    [[nodiscard]]
    const_reference
    back (void) const;

    [[nodiscard]]
    size_type
    size (void) const noexcept;

    [[nodiscard]]
    bool
    empty (void) const noexcept;

    [[nodiscard]]
    const_reference
    operator[] (size_type n) const;

    void
    push_back (ir_static_instruction&& instr);

  private:
    container_type m_instructions;
  };

  std::ostream&
  operator<< (std::ostream& out, const ir_static_block& block);

}

#endif // OCTAVE_IR_STATIC_IR_IR_STATIC_BLOCK_HPP

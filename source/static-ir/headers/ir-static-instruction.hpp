/** ir-static-instruction.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_STATIC_IR_IR_STATIC_INSTRUCTION_HPP
#define OCTAVE_IR_STATIC_IR_IR_STATIC_INSTRUCTION_HPP

#include "ir-static-def.hpp"
#include "ir-static-operand.hpp"
#include "ir-metadata.hpp"

#include "ir-common.hpp"

#include <gch/optional_ref.hpp>
#include <gch/small_vector.hpp>

#include <iosfwd>
#include <optional>

namespace gch
{

  class ir_instruction;
  class ir_static_variable_map;

  class ir_static_instruction
  {
  public:
    using args_container_type     = small_vector<ir_static_operand, 2>;
    using value_type              = args_container_type::value_type;
    using allocator_type          = args_container_type::allocator_type;
    using size_type               = args_container_type::size_type;
    using difference_type         = args_container_type::difference_type;
    using reference               = args_container_type::reference;
    using const_reference         = args_container_type::const_reference;
    using pointer                 = args_container_type::pointer;
    using const_pointer           = args_container_type::const_pointer;

    using iterator                = args_container_type::iterator;
    using const_iterator          = args_container_type::const_iterator;
    using reverse_iterator        = args_container_type::reverse_iterator;
    using const_reverse_iterator  = args_container_type::const_reverse_iterator;

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

    ir_static_instruction            (void)                             = delete;
    ir_static_instruction            (const ir_static_instruction&)     = default;
    ir_static_instruction            (ir_static_instruction&&) noexcept = default;
    ir_static_instruction& operator= (const ir_static_instruction&)     = default;
    ir_static_instruction& operator= (ir_static_instruction&&) noexcept = default;
    ~ir_static_instruction           (void)                             = default;

    ir_static_instruction (ir_metadata m, ir_static_def def, args_container_type&& args) noexcept;

    ir_static_instruction (ir_metadata m, args_container_type&& args) noexcept;

    explicit
    ir_static_instruction (ir_metadata m) noexcept;

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
    size_type
    num_args (void) const noexcept;

    [[nodiscard]]
    bool
    has_args (void) const noexcept;

    [[nodiscard]]
    const_reference
    operator[] (size_type n) const;

    [[nodiscard]]
    ir_metadata
    get_metadata (void) const noexcept;

    [[nodiscard]]
    bool
    has_def (void) const noexcept;

    [[nodiscard]]
    const ir_static_def&
    get_def (void) const;

    [[nodiscard]]
    const std::optional<ir_static_def>&
    maybe_get_def (void) const noexcept;

    template <ir_opcode Op,
              typename ...Args,
              typename traits = ir_instruction_traits<Op>,
              std::enable_if_t<! traits::is_abstract && traits::has_def> * = nullptr>
    static
    ir_static_instruction
    create (ir_static_def def, Args&&... args) noexcept
    {
      return ir_static_instruction { ir_metadata_v<Op>, def, { std::forward<Args> (args)... } };
    }

    template <ir_opcode Op,
              typename ...Args,
              typename traits = ir_instruction_traits<Op>,
              std::enable_if_t<! traits::is_abstract> * = nullptr>
    static
    ir_static_instruction
    create (Args&&... args) noexcept
    {
      return ir_static_instruction { ir_metadata_v<Op>, { std::forward<Args> (args)... } };
    }

  private:
    ir_metadata                  m_metadata;
    std::optional<ir_static_def> m_def;
    args_container_type          m_args;
  };

  template <ir_opcode BaseOp>
  [[nodiscard]]
  bool
  is_a (const ir_static_instruction& instr) noexcept
  {
    return instr.get_metadata ().is_a (ir_metadata_v<BaseOp>);
  }

}

#endif // OCTAVE_IR_STATIC_IR_IR_STATIC_INSTRUCTION_HPP

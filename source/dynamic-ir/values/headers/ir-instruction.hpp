/** ir-instruction.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_INSTRUCTION_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_INSTRUCTION_HPP

#include "ir-def.hpp"
#include "ir-instruction-fwd.hpp"
#include "ir-operand.hpp"
#include "ir-variable.hpp"

#include "ir-common.hpp"

#include "ir-constant.hpp"
#include "ir-metadata.hpp"
#include "ir-type.hpp"

#include <gch/small_vector.hpp>

#include <unordered_map>
#include <list>
#include <memory>
#include <utility>

namespace gch
{

  class ir_instruction
  {
    template <ir_opcode Op>
    using type = ir_instruction_traits<Op>;

  public:
    template <ir_opcode Op>
    using tag_t = type<Op>;

    template <ir_opcode Op>
    static constexpr tag_t<Op> tag { };

    using metadata_t = ir_metadata;

    using args_container_type     = small_vector<ir_operand, 2>;
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

    ir_instruction            (void)                      = delete;
    ir_instruction            (const ir_instruction&)     = delete;
    ir_instruction            (ir_instruction&&) noexcept;
    ir_instruction& operator= (const ir_instruction&)     = delete;
    ir_instruction& operator= (ir_instruction&&) noexcept;
    ~ir_instruction           (void)                      = default;

    template <ir_opcode Op>
    static constexpr
    void
    assert_not_abstract (void)
    {
      static_assert (! type<Op>::is_abstract,
                     "Cannot instantiate abstract instruction");
    }

    template <ir_opcode Op>
    static constexpr
    void
    assert_has_def (void)
    {
      static_assert (type<Op>::has_def,
                     "Instruction metadata specified no such def.");
    }

    template <ir_opcode Op>
    static constexpr
    void
    assert_no_def (void)
    {
      static_assert (! type<Op>::has_def,
                     "Instruction metadata specified a def.");
    }

    template <ir_opcode Op, std::size_t N>
    static constexpr
    void
    assert_is_valid_num_args (void)
    {
      static_assert (type<Op>::template is_valid_num_args<N>,
                     "Instruction metadata specified a different number of arguments.");
    }

    template <ir_opcode Op>
    static constexpr
    void
    assert_no_args (void)
    {
      static_assert (type<Op>::template is_valid_num_args<0>,
                     "Instruction metadata specified at least one argument");
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction (tag_t<Op>, ir_variable& ret_var, Args&&... args)
      : m_metadata (type<Op>::metadata),
        m_def      (std::in_place, *this, ret_var),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_has_def<Op> ();

      initialize_with_pack<Op> (std::forward<Args> (args)...);
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction (tag_t<Op>, ir_operand_in&& op, Args&&... args)
      : m_metadata (type<Op>::metadata),
        m_def      (std::nullopt)
    {
      assert_not_abstract<Op> ();
      assert_no_def<Op> ();

      initialize_with_pack<Op> (std::move (op), std::forward<Args> (args)...);
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction (tag_t<Op>, ir_constant&& c, Args&&... args)
      : m_metadata (type<Op>::metadata),
        m_def      (std::nullopt)
    {
      assert_not_abstract<Op> ();
      assert_no_def<Op> ();

      initialize_with_pack<Op> (std::move (c), std::forward<Args> (args)...);
    }



    template <ir_opcode Op, std::size_t N>
    ir_instruction (tag_t<Op>, ir_variable& ret_var, std::array<ir_operand_in, N>&& args)
      : m_metadata (type<Op>::metadata),
        m_def      (std::in_place, *this, ret_var)
    {
      assert_not_abstract<Op> ();
      assert_has_def<Op> ();

      initialize_with_array<Op> (std::move (args));
    }

    template <ir_opcode Op, std::size_t N>
    ir_instruction (tag_t<Op>, std::array<ir_operand_in, N>&& args)
      : m_metadata (type<Op>::metadata),
        m_def (std::nullopt)
    {
      assert_not_abstract<Op> ();
      assert_no_def<Op> ();

      initialize_with_array<Op> (std::move (args));
    }

    template <ir_opcode Op>
    explicit
    ir_instruction (tag_t<Op>)
      : m_metadata (type<Op>::metadata),
        m_def (std::nullopt)
    {
      assert_not_abstract<Op> ();
      assert_no_def<Op> ();
      assert_no_args<Op> ();
    }

    template <ir_opcode Op, typename ...Args>
    void
    initialize_with_pack (Args&&... args)
    {
      assert_is_valid_num_args<Op, sizeof... (Args)> ();

      m_args.reserve (sizeof... (Args));
      (m_args.emplace_back (*this, std::forward<Args> (args)), ...);
    }

    template <ir_opcode Op, std::size_t N>
    void
    initialize_with_array (std::array<ir_operand_in, N>&& args)
    {
      assert_is_valid_num_args<Op, N> ();

      m_args.reserve (N);
      std::for_each (std::move_iterator { args.begin () },
                     std::move_iterator { args.end () },
                     [this](ir_operand_in&& op)
                     {
                       emplace_back (std::move (op));
                     });
    }

    [[nodiscard]]
    iterator
    begin (void) noexcept;

    [[nodiscard]]
    const_iterator
    begin (void) const noexcept;

    [[nodiscard]]
    const_iterator
    cbegin (void) const noexcept;

    [[nodiscard]]
    iterator
    end (void) noexcept;

    [[nodiscard]]
    const_iterator
    end (void) const noexcept;

    [[nodiscard]]
    const_iterator
    cend (void) const noexcept;

    [[nodiscard]]
    reverse_iterator
    rbegin (void) noexcept;

    [[nodiscard]]
    const_reverse_iterator
    rbegin (void) const noexcept;

    [[nodiscard]]
    const_reverse_iterator
    crbegin (void) const noexcept;

    [[nodiscard]]
    reverse_iterator
    rend (void) noexcept;

    [[nodiscard]]
    const_reverse_iterator
    rend (void) const noexcept;

    [[nodiscard]]
    const_reverse_iterator
    crend (void) const noexcept;

    [[nodiscard]]
    reference
    front (void);

    [[nodiscard]]
    const_reference
    front (void) const;

    [[nodiscard]]
    reference
    back (void);

    [[nodiscard]]
    const_reference
    back (void) const;

    [[nodiscard]]
    size_type
    size (void) const noexcept;

    [[nodiscard]]
    bool
    empty (void) const noexcept;

    iter
    erase (citer pos);

    iter
    erase (citer first, citer last);

    void
    set_args (args_container_type&& args);

    template <typename ...Args>
    ir_operand&
    emplace_back (Args&&... args)
    {
      return m_args.emplace_back (*this, std::forward<Args> (args)...);
    }

    template <typename ...Args>
    iter
    emplace_before (citer pos, Args&&... args)
    {
      return m_args.emplace (pos, *this, std::forward<Args> (args)...);
    }

    [[nodiscard]]
    metadata_t
    get_metadata (void) const noexcept;

    [[nodiscard]]
    ir_def&
    get_def (void) noexcept;

    [[nodiscard]]
    const ir_def&
    get_def (void) const noexcept;

    [[nodiscard]]
    bool
    has_def (void) const noexcept;

    [[nodiscard]]
    optional_ref<ir_def>
    maybe_get_def (void) noexcept;

    [[nodiscard]]
    optional_cref<ir_def>
    maybe_get_def (void) const noexcept;

  private:
    metadata_t            m_metadata;
    std::optional<ir_def> m_def;
    args_container_type   m_args;
  };

  template <ir_opcode BaseOp>
  [[nodiscard]]
  bool
  is_a (const ir_instruction& instr) noexcept
  {
    return instr.get_metadata ().is_a (ir_metadata_v<BaseOp>);
  }

  [[nodiscard]]
  ir_def&
  get_def (ir_instruction& instr) noexcept;

  [[nodiscard]]
  const ir_def&
  get_def (const ir_instruction& instr) noexcept;

  [[nodiscard]]
  bool
  has_def (const ir_instruction& instr) noexcept;

  [[nodiscard]]
  optional_ref<ir_def>
  maybe_get_def (ir_instruction& instr) noexcept;

  [[nodiscard]]
  optional_cref<ir_def>
  maybe_get_def (const ir_instruction& instr) noexcept;

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_INSTRUCTION_HPP

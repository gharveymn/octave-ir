/*

Copyright (C) 2019 Gene Harvey

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#ifndef OCTAVE_IR_IR_INSTRUCTION_HPP
#define OCTAVE_IR_IR_INSTRUCTION_HPP

#include "utilities/ir-common.hpp"
#include "values/types/ir-instruction-metadata.hpp"
#include "values/types/ir-type-ir.hpp"
#include "values/ir-instruction-fwd.hpp"
#include "values/ir-variable.hpp"

#include <unordered_map>
#include <list>
#include <vector>
#include <memory>
#include <utility>

namespace gch
{

  template <ir_opcode Op>
  struct ir_instruction_traits
  {
    explicit
    ir_instruction_traits (void) = default;

    static constexpr auto metadata    = ir_instruction_metadata_v<Op>;
    static constexpr auto opcode      = metadata.get_opcode ();
    static constexpr auto name        = metadata.get_name ();
    static constexpr auto is_abstract = metadata.is_abstract ();
    static constexpr auto has_base    = metadata.has_base ();
    static constexpr auto has_def     = metadata.has_def ();
    static constexpr auto arity       = metadata.get_arity ();

    static constexpr auto is_n_ary    = metadata.is_n_ary ();
    static constexpr auto is_nullary  = metadata.is_nullary ();
    static constexpr auto is_unary    = metadata.is_unary ();
    static constexpr auto is_binary   = metadata.is_binary ();
    static constexpr auto is_ternary  = metadata.is_ternary ();

    template <bool HasBase = has_base, std::enable_if_t<HasBase> * = nullptr>
    static constexpr
    auto
    base = metadata.get_base ();

    template <ir_opcode BaseOp>
    static constexpr
    bool
    is_a = metadata.is_a (ir_instruction_metadata_v<BaseOp>);

    template <ir_opcode OtherOp>
    static constexpr
    bool
    is_base_of = metadata.is_base_of (ir_instruction_metadata_v<OtherOp>);

    template <std::size_t N>
    static constexpr
    bool
    is_valid_num_args = is_n_ary || (static_cast<std::size_t> (arity) == N);
  };

  static_assert (ir_instruction_traits<ir_opcode::band>::is_a<ir_opcode::bitwise>);

  class ir_instruction
  {
    template <ir_opcode Op>
    using type = ir_instruction_traits<Op>;

  public:
    template <ir_opcode Op>
    using tag_t = type<Op>;

    template <ir_opcode Op>
    static constexpr tag_t<Op> tag { };

    using metadata_t = ir_instruction_metadata;

    using args_container_type     = std::vector<ir_operand>;
    using iterator                = typename args_container_type::iterator;
    using const_iterator          = typename args_container_type::const_iterator;
    using reverse_iterator        = typename args_container_type::reverse_iterator;
    using const_reverse_iterator  = typename args_container_type::const_reverse_iterator;
    using reference               = typename args_container_type::reference;
    using const_reference         = typename args_container_type::const_reference;
    using size_type               = typename args_container_type::size_type;
    using difference_type         = typename args_container_type::difference_type;
    using value_type              = typename args_container_type::value_type;
    using allocator_type          = typename args_container_type::allocator_type;

    using iter  = iterator;
    using citer = const_iterator;
    using ref   = reference;
    using cref  = const_reference;

    ir_instruction            (void)                      = delete;
    ir_instruction            (const ir_instruction&)     = delete;
    ir_instruction            (ir_instruction&&) noexcept;
    ir_instruction& operator= (const ir_instruction&)     = delete;
//  ir_instruction& operator= (ir_instruction&&) noexcept = impl;
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
        m_def (std::in_place, ret_var, *this),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_has_def<Op> ();

      initialize_with_pack<Op> (std::forward<Args> (args)...);
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction (tag_t<Op>, ir_operand_in&& op, Args&&... args)
      : m_metadata (type<Op>::metadata),
        m_def (std::nullopt),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_no_def<Op> ();

      initialize_with_pack<Op> (std::move (op), std::forward<Args> (args)...);
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction (tag_t<Op>, ir_constant&& c, Args&&... args)
      : m_metadata (type<Op>::metadata),
        m_def (std::nullopt),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_no_def<Op> ();

      initialize_with_pack<Op> (std::move (c), std::forward<Args> (args)...);
    }



    template <ir_opcode Op, std::size_t N>
    explicit
    ir_instruction (tag_t<Op>, ir_variable& ret_var, std::array<ir_operand_in, N>&& args)
      : m_metadata (type<Op>::metadata),
        m_def (std::in_place, ret_var, *this),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_has_def<Op> ();

      initialize_with_array<Op> (std::move (args));
    }

    template <ir_opcode Op, std::size_t N>
    explicit
    ir_instruction (tag_t<Op>, std::array<ir_operand_in, N>&& args)
      : m_metadata (type<Op>::metadata),
        m_def (std::nullopt),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_no_def<Op> ();

      initialize_with_array<Op> (std::move (args));
    }

    template <ir_opcode Op>
    explicit
    ir_instruction (tag_t<Op>)
      : m_metadata (type<Op>::metadata),
        m_def (std::nullopt),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_no_def<Op> ();
      assert_no_args<Op> ();
    }

    ir_instruction&
    operator= (ir_instruction&& other) noexcept
    {
      m_metadata = other.m_metadata;
      set_def (std::move (other.m_def));
      set_args (std::move (other.m_args));
      return *this;
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

    [[nodiscard]] auto  begin   (void)       noexcept { return m_args.begin ();   }
    [[nodiscard]] auto  begin   (void) const noexcept { return m_args.begin ();   }
    [[nodiscard]] auto  cbegin  (void) const noexcept { return m_args.cbegin ();  }

    [[nodiscard]] auto  end     (void)       noexcept { return m_args.end ();     }
    [[nodiscard]] auto  end     (void) const noexcept { return m_args.end ();     }
    [[nodiscard]] auto  cend    (void) const noexcept { return m_args.cend ();    }

    [[nodiscard]] auto  rbegin  (void)       noexcept { return m_args.rbegin ();  }
    [[nodiscard]] auto  rbegin  (void) const noexcept { return m_args.rbegin ();  }
    [[nodiscard]] auto  crbegin (void) const noexcept { return m_args.crbegin (); }

    [[nodiscard]] auto  rend    (void)       noexcept { return m_args.rend ();    }
    [[nodiscard]] auto  rend    (void) const noexcept { return m_args.rend ();    }
    [[nodiscard]] auto  crend   (void) const noexcept { return m_args.crend ();   }

    [[nodiscard]] auto& front   (void)       noexcept { return m_args.front ();   }
    [[nodiscard]] auto& front   (void) const noexcept { return m_args.front ();   }

    [[nodiscard]] auto& back    (void)       noexcept { return m_args.back ();    }
    [[nodiscard]] auto& back    (void) const noexcept { return m_args.back ();    }

    // distinguish between expected num_args from arity
    [[nodiscard]]
    auto
    current_num_args (void) const noexcept
    {
      return m_args.size ();
    }

    // instead of empty () to distinguish between expected empty instruction and current state
    [[nodiscard]]
    bool
    is_currently_empty (void) const noexcept
    {
      return m_args.empty ();
    }

    iter
    erase (citer pos)
    {
      return m_args.erase (pos);
    };

    iter
    erase (citer first, citer last)
    {
      return m_args.erase (first, last);
    };

    void
    set_def (std::optional<ir_def>&& def);

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

    [[nodiscard]] constexpr
    metadata_t
    get_metadata (void) const noexcept
    {
      return m_metadata;
    }

    [[nodiscard]] constexpr
    ir_def&
    get_def (void) noexcept
    {
      return *m_def;
    }
    [[nodiscard]] constexpr
    const ir_def&
    get_def (void) const noexcept
    {
      return *m_def;
    }

  private:
    metadata_t            m_metadata;
    std::optional<ir_def> m_def;
    args_container_type   m_args;
  };

  template <ir_opcode BaseOp>
  [[nodiscard]] constexpr
  bool
  is_a (const ir_instruction& instr) noexcept
  {
    return instr.get_metadata ().is_a (ir_instruction_metadata_v<BaseOp>);
  }

  [[nodiscard]] constexpr
  bool
  has_def (const ir_instruction& instr) noexcept
  {
    return instr.get_metadata ().has_def ();
  }

}

#endif

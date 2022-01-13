/** ir-static-instruction.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-static-instruction.hpp"

#include "ir-optional-util.hpp"

#include <numeric>
#include <ostream>

namespace gch
{

  ir_static_instruction::
  ir_static_instruction (metadata_t m, ir_static_def def, args_container_type&& args)
    : m_metadata (m),
      m_def (def),
      m_args (std::move (args))
  { }

  ir_static_instruction::
  ir_static_instruction (metadata_t m, args_container_type&& args)
    : m_metadata (m),
      m_args (std::move (args))
  { }

  ir_static_instruction::
  ir_static_instruction (metadata_t m)
    : m_metadata (m)
  { }

  auto
  ir_static_instruction::
  begin (void) const noexcept
    -> citer
  {
    return m_args.begin ();
  }

  auto
  ir_static_instruction::
  end (void) const noexcept
    -> citer
  {
    return m_args.end ();
  }

  auto
  ir_static_instruction::
  rbegin (void) const noexcept
    -> criter
  {
    return m_args.rbegin ();
  }

  auto
  ir_static_instruction::
  rend (void) const noexcept
    -> criter
  {
    return m_args.rend ();
  }

  auto
  ir_static_instruction::
  front (void) const
    -> cref
  {
    return m_args.front ();
  }

  auto
  ir_static_instruction::
  back (void) const
    -> cref
  {
    return m_args.back ();
  }

  auto
  ir_static_instruction::
  size (void) const noexcept
    -> size_ty
  {
   return m_args.size ();
  }

  bool
  ir_static_instruction::
  empty (void) const noexcept
  {
    return m_args.empty ();
  }

  auto
  ir_static_instruction::
  num_args (void) const noexcept
    -> size_ty
  {
   return size ();
  }

  bool
  ir_static_instruction::
  has_args (void) const noexcept
  {
    return ! empty ();
  }

  auto
  ir_static_instruction::
  operator[] (size_type n) const
    -> const_reference
  {
    return m_args[n];
  }

  auto
  ir_static_instruction::
  get_metadata (void) const noexcept
  -> metadata_t
  {
    return m_metadata;
  }

  const ir_static_def&
  ir_static_instruction::
  get_def (void) const noexcept
  {
    return *m_def;
  }

  enum class needs_parentheses : bool
  {
    yes = true,
    no  = false
  };

  template <ir_opcode Op>
  constexpr
  needs_parentheses
  needs_parentheses_v = static_cast<needs_parentheses> (
      ! ir_metadata_v<Op>.is_a (ir_metadata_v<ir_opcode::relation>)
    &&! ir_metadata_v<Op>.is_a (ir_metadata_v<ir_opcode::arithmetic>)
    &&! ir_metadata_v<Op>.is_a (ir_metadata_v<ir_opcode::logical>)
    &&! ir_metadata_v<Op>.is_a (ir_metadata_v<ir_opcode::bitwise>));

  template <ir_opcode Op,
            ir_metadata::flag::is_abstract IsAbstract       = ir_metadata_v<Op>.get_is_abstract (),
            ir_metadata::flag::has_def     HasDef           = ir_metadata_v<Op>.get_has_def (),
            ir_metadata::flag::arity       Arity            = ir_metadata_v<Op>.get_arity (),
            needs_parentheses              NeedsParentheses = needs_parentheses_v<Op>>
  struct instruction_printer
  {
    static
    std::ostream&
    print (std::ostream&, const ir_static_instruction&)
    {
      throw std::ios_base::failure { "Cannot print an instruction (it shouldn't even exist)." };
    }
  };

  template <ir_opcode Op, needs_parentheses NeedsParentheses>
  struct instruction_printer<Op,
                             ir_metadata::flag::is_abstract::no,
                             ir_metadata::flag::has_def::    no,
                             ir_metadata::flag::arity::      nullary,
                             NeedsParentheses>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      return out << instr.get_metadata ().get_name ();
    }
  };

  template <ir_opcode Op, needs_parentheses NeedsParentheses>
  struct instruction_printer<Op,
                             ir_metadata::flag::is_abstract::no,
                             ir_metadata::flag::has_def::    yes,
                             ir_metadata::flag::arity::      nullary,
                             NeedsParentheses>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      return out << instr.get_def () << " = " << instr.get_metadata ().get_name ();
    }
  };

  template <ir_opcode Op, ir_metadata::flag::arity NotNullary>
  struct instruction_printer<Op,
                             ir_metadata::flag::is_abstract::no,
                             ir_metadata::flag::has_def::    yes,
                             NotNullary,
                             needs_parentheses::             yes>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      out << instr.get_def ()
          << " = "
          << instr.get_metadata ().get_name ()
          << " ("
          << instr[0];

      std::for_each (std::next (instr.begin ()), instr.end (),
                     [&](const ir_static_operand& op) { out << ", " << op; });

      return out << ')';
    }
  };

  template <ir_opcode Op>
  struct instruction_printer<Op,
                             ir_metadata::flag::is_abstract::no,
                             ir_metadata::flag::has_def::    yes,
                             ir_metadata::flag::arity::      unary,
                             needs_parentheses::             no>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      return out << instr.get_def ()
                 << " = "
                 << instr.get_metadata ().get_name ()
                 << instr[0];
    }
  };

  template <ir_opcode Op>
  struct instruction_printer<Op,
                             ir_metadata::flag::is_abstract::no,
                             ir_metadata::flag::has_def::    yes,
                             ir_metadata::flag::arity::      binary,
                             needs_parentheses::             no>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      return out << instr.get_def ()
                 << " = "
                 << instr[0]
                 << ' '
                 << instr.get_metadata ().get_name ()
                 << ' '
                 << instr[1];
    }
  };

  template <ir_opcode Op>
  struct instruction_printer_mapper
  {
    constexpr
    auto
    operator() (void) const noexcept
    {
      return instruction_printer<Op>::print;
    }
  };

  static constexpr
  auto
  instruction_printer_map = ir_metadata::generate_map<instruction_printer_mapper> ();

  std::ostream&
  operator<< (std::ostream& out, const ir_static_instruction& instr)
  {
    return instruction_printer_map[instr.get_metadata ()] (out, instr);
  }

}

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
  ir_static_instruction (ir_metadata m, ir_static_def def, args_container_type&& args)
    : m_metadata (m),
      m_def (def),
      m_args (std::move (args))
  {
    assert (m.has_def ());
  }

  ir_static_instruction::
  ir_static_instruction (ir_metadata m, args_container_type&& args)
    : m_metadata (m),
      m_args (std::move (args))
  {
    // assert (! m.has_def ());
  }

  ir_static_instruction::
  ir_static_instruction (ir_metadata m)
    : ir_static_instruction (m, { })
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

  ir_metadata
  ir_static_instruction::
  get_metadata (void) const noexcept
  {
    return m_metadata;
  }

  bool
  ir_static_instruction::
  has_def (void) const noexcept
  {
    return m_def.has_value ();
  }

  const ir_static_def&
  ir_static_instruction::
  get_def (void) const
  {
    return *m_def;
  }

  const std::optional<ir_static_def>&
  ir_static_instruction::
  maybe_get_def (void) const noexcept
  {
    return m_def;
  }

  template <ir_opcode Op,
            ir_metadata::flag::is_abstract IsAbstract       = ir_metadata_v<Op>.get_is_abstract (),
            ir_metadata::flag::has_def     HasDef           = ir_metadata_v<Op>.get_has_def (),
            ir_metadata::flag::arity       Arity            = ir_metadata_v<Op>.get_arity ()>
  struct instruction_printer
  {
    static
    std::ostream&
    print (std::ostream&, const ir_static_instruction&)
    {
      throw std::ios_base::failure { "Cannot print an instruction (it shouldn't even exist)." };
    }
  };

  template <ir_opcode Op>
  struct instruction_printer<Op,
                             ir_metadata::flag::is_abstract::no,
                             ir_metadata::flag::has_def::    no,
                             ir_metadata::flag::arity::      nullary>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      return out << instr.get_metadata ().get_name ();
    }
  };

  template <ir_opcode Op>
  struct instruction_printer<Op,
                             ir_metadata::flag::is_abstract::no,
                             ir_metadata::flag::has_def::    yes,
                             ir_metadata::flag::arity::      nullary>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      return out << instr.get_def () << " = " << instr.get_metadata ().get_name ();
    }
  };

  template <>
  struct instruction_printer<ir_opcode::call>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      if (instr.has_def ())
        out << instr.get_def () << " = ";

      out << instr.get_metadata ().get_name ()
          << " (";

      if (instr.has_args ())
      {
        out << instr[0];
        std::for_each (std::next (instr.begin ()), instr.end (),
                       [&](const ir_static_operand& op) { out << ", " << op; });
      }

      return out << ')';
    }
  };

  template <>
  struct instruction_printer<ir_opcode::phi>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      if (instr.has_def ())
        out << instr.get_def () << " = ";

      out << instr.get_metadata ().get_name ()
          << " (";

      if (instr.has_args ())
      {
        out << instr[1]
            << " : Block"
            << instr[0];

        for (auto it = std::next (instr.begin (), 2); it != instr.end (); ++it)
        {
          auto block_it = it++;
          out << " | "
              << *it
              << " : Block"
              << *block_it;
        }
      }

      return out << ')';
    }
  };

  template <>
  struct instruction_printer<ir_opcode::assign>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      if (instr.has_def ())
        out << instr.get_def () << " = ";
      return out << instr[0];
    }
  };

  template <ir_opcode Op>
  struct instruction_printer<Op,
                             ir_metadata::flag::is_abstract::no,
                             ir_metadata::flag::has_def::    yes,
                             ir_metadata::flag::arity::      unary>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      if (instr.has_def ())
        out << instr.get_def () << " = ";
      return out << instr.get_metadata ().get_name ()
                 << instr[0];
    }
  };

  template <ir_opcode Op>
  struct instruction_printer<Op,
                             ir_metadata::flag::is_abstract::no,
                             ir_metadata::flag::has_def::    no,
                             ir_metadata::flag::arity::      unary>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      return out << instr.get_metadata ().get_name ()
                 << ' '
                 << instr[0];
    }
  };

  template <ir_opcode Op>
  struct instruction_printer<Op,
                             ir_metadata::flag::is_abstract::no,
                             ir_metadata::flag::has_def::    yes,
                             ir_metadata::flag::arity::      binary>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      if (instr.has_def ())
        out << instr.get_def () << " = ";
      return out << instr[0]
                 << ' '
                 << instr.get_metadata ().get_name ()
                 << ' '
                 << instr[1];
    }
  };

  template <>
  struct instruction_printer<ir_opcode::cbranch>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr)
    {
      return out << instr.get_metadata ().get_name ()
                 << ' '
                 << instr[0]
                 << " ? "
                 << instr[1]
                 << " : "
                 << instr[2];
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

/** ir-static-unit.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-static-function.hpp"

#include "ir-static-block.hpp"
#include "ir-static-instruction.hpp"
#include "ir-static-variable.hpp"

#include <numeric>
#include <ostream>

namespace gch
{

  ir_static_function::
  ir_static_function (std::string_view name,
                      container_type&& blocks,
                      std::vector<ir_static_variable>&& vars,
                      small_vector<ir_variable_id>&& ret_ids,
                      small_vector<ir_variable_id>&& arg_ids)
    : m_name      (name),
      m_blocks    (std::move (blocks)),
      m_variables (std::move (vars)),
      m_ret_ids   (std::move (ret_ids)),
      m_arg_ids   (std::move (arg_ids))
  { }

  ir_static_function::~ir_static_function (void) = default;

  auto
  ir_static_function::
  begin (void) const noexcept
    -> citer
  {
    return m_blocks.begin ();
  }

  auto
  ir_static_function::
  end (void) const noexcept
    -> citer
  {
    return m_blocks.end ();
  }

  auto
  ir_static_function::
  rbegin (void) const noexcept
    -> criter
  {
    return m_blocks.rbegin ();
  }

  auto
  ir_static_function::
  rend (void) const noexcept
    -> criter
  {
    return m_blocks.rend ();
  }

  auto
  ir_static_function::
  front (void) const
    -> cref
  {
    return m_blocks.front ();
  }

  auto
  ir_static_function::
  back (void) const
    -> cref
  {
    return m_blocks.back ();
  }

  auto
  ir_static_function::
  size (void) const noexcept
    -> size_ty
  {
   return m_blocks.size ();
  }

  bool
  ir_static_function::
  empty (void) const noexcept
  {
    return m_blocks.empty ();
  }

  auto
  ir_static_function::
  num_blocks (void) const noexcept
    -> size_ty
  {
   return size ();
  }

  auto
  ir_static_function::
  operator[] (size_type pos) const
    -> const_reference
  {
    return m_blocks[pos];
  }

  auto
  ir_static_function::
  variables_begin (void) const noexcept
    -> variables_const_iterator
  {
    return m_variables.begin ();
  }

  auto
  ir_static_function::
  variables_end (void) const noexcept
    -> variables_const_iterator
  {
    return m_variables.end ();
  }

  small_vector<ir_variable_id>::const_iterator
  ir_static_function::
  returns_begin (void) const noexcept
  {
    return m_ret_ids.begin ();
  }

  small_vector<ir_variable_id>::const_iterator
  ir_static_function::
  returns_end (void) const noexcept
  {
    return m_ret_ids.end ();
  }

  bool
  ir_static_function::
  has_returns (void) const noexcept
  {
    return ! m_ret_ids.empty ();
  }

  small_vector<ir_variable_id>::const_iterator
  ir_static_function::
  args_begin (void) const noexcept
  {
    return m_arg_ids.begin ();
  }

  small_vector<ir_variable_id>::const_iterator
  ir_static_function::
  args_end (void) const noexcept
  {
    return m_arg_ids.end ();
  }

  std::string_view
  ir_static_function::
  get_name (void) const noexcept
  {
    return m_name;
  }

  std::string
  ir_static_function::
  get_block_name (const ir_static_block& block) const
  {
    return std::string ("BLOCK").append (std::to_string (std::distance (&m_blocks[0], &block)));
  }

  const ir_static_variable&
  ir_static_function::
  get_variable (ir_variable_id var_id) const
  {
    return m_variables[var_id];
  }

  const ir_static_variable&
  ir_static_function::
  get_variable (ir_static_def def) const
  {
    return get_variable (def.get_variable_id ());
  }

  const ir_static_variable&
  ir_static_function::
  get_variable (ir_static_use use) const
  {
    return get_variable (use.get_variable_id ());
  }

  std::string_view
  ir_static_function::
  get_variable_name (ir_variable_id var_id) const
  {
    return get_variable (var_id).get_name ();
  }

  std::string_view
  ir_static_function::
  get_variable_name (ir_static_def def) const
  {
    return get_variable_name (def.get_variable_id ());
  }

  std::string_view
  ir_static_function::
  get_variable_name (ir_static_use use) const
  {
    return get_variable_name (use.get_variable_id ());
  }

  ir_type
  ir_static_function::
  get_type (ir_variable_id var_id) const
  {
    return get_variable (var_id).get_type ();
  }

  ir_type
  ir_static_function::
  get_type (ir_static_def def) const
  {
    return get_type (def.get_variable_id ());
  }

  ir_type
  ir_static_function::
  get_type (ir_static_use use) const
  {
    return get_type (use.get_variable_id ());
  }

  std::ostream&
  ir_static_function::
  print (std::ostream& out, const ir_static_def& def) const
  {
    return out << get_variable (def.get_variable_id ()).get_name () << def.get_id ();
  }

  std::ostream&
  ir_static_function::
  print (std::ostream& out, const ir_static_use& use) const
  {
    out << get_variable (use.get_variable_id ()).get_name ();
    if (std::optional def_id { use.get_def_id () })
      return out << *def_id;
    return out << "??";
  }

  std::ostream&
  ir_static_function::
  print (std::ostream& out, const ir_static_operand& op) const
  {
    if (optional_ref constant { maybe_as_constant (op) })
      return out << *constant;
    return print (out, op.as_use ());
  }

   template <ir_opcode Op,
            ir_metadata::flag::is_abstract IsAbstract = ir_metadata_v<Op>.get_is_abstract (),
            ir_metadata::flag::has_def     HasDef     = ir_metadata_v<Op>.get_has_def (),
            ir_metadata::flag::arity       Arity      = ir_metadata_v<Op>.get_arity ()>
  struct instruction_printer
  {
    static
    std::ostream&
    print (std::ostream&, const ir_static_instruction&, const ir_static_function&)
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
    print (std::ostream& out, const ir_static_instruction& instr, const ir_static_function&)
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
    print (std::ostream& out, const ir_static_instruction& instr, const ir_static_function& func)
    {
      out << instr.get_metadata ().get_name () << " ";
      return func.print (out, instr.get_def ());
    }
  };

  template <>
  struct instruction_printer<ir_opcode::call>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr, const ir_static_function& func)
    {
      if (instr.has_def ())
        func.print (out, instr.get_def ()) << " = ";

      assert (instr.has_args ());
      out << as_constant<ir_external_function_info> (instr[0]).get_name ()
          << " (";

      if (1 < instr.num_args ())
      {
        func.print (out, instr[1]);
        std::for_each (std::next (instr.begin (), 2), instr.end (), [&](const auto& op) {
          func.print (out << ", ",  op);
        });
      }

      return out << ')';
    }
  };

  template <>
  struct instruction_printer<ir_opcode::phi>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr, const ir_static_function& func)
    {
      if (instr.has_def ())
        func.print (out, instr.get_def ()) << " = ";

      out << instr.get_metadata ().get_name ()
          << " (";

      if (instr.has_args ())
      {
        func.print (out, instr[1]);
        out << " : ";
        func.print (out, instr[0]);

        for (auto it = std::next (instr.begin (), 2); it != instr.end (); ++it)
        {
          auto block_it = it++;

          out << " | ";
          func.print (out, *it);
          out << " : ";
          func.print (out, *block_it);
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
    print (std::ostream& out, const ir_static_instruction& instr, const ir_static_function& func)
    {
      if (instr.has_def ())
        func.print (out, instr.get_def ()) << " = ";
      func.print (out, instr[0]);

      return out;
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
    print (std::ostream& out, const ir_static_instruction& instr, const ir_static_function& func)
    {
      if (instr.has_def ())
        func.print (out, instr.get_def ()) << " = ";

      out << instr.get_metadata ().get_name ();
      func.print (out, instr[0]);

      return out;
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
    print (std::ostream& out, const ir_static_instruction& instr, const ir_static_function& func)
    {
      out << instr.get_metadata ().get_name () << ' ';
      func.print (out, instr[0]);

      return out;
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
    print (std::ostream& out, const ir_static_instruction& instr, const ir_static_function& func)
    {
      if (instr.has_def ())
        func.print (out, instr.get_def ()) << " = ";
      func.print (out, instr[0]);
      out << ' ' << instr.get_metadata ().get_name () << ' ';
      func.print (out, instr[1]);

      return out;
    }
  };

  template <>
  struct instruction_printer<ir_opcode::cbranch>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr, const ir_static_function& func)
    {
      out << instr.get_metadata ().get_name () << ' ';
      func.print (out, instr[0]);
      out << " ? ";
      func.print (out, instr[1]);
      out << " : ";
      func.print (out, instr[2]);

      return out;
    }
  };

  template <>
  struct instruction_printer<ir_opcode::ret>
  {
    static
    std::ostream&
    print (std::ostream& out, const ir_static_instruction& instr, const ir_static_function& func)
    {
      out << instr.get_metadata ().get_name ();

      if (instr.has_args ())
      {
        return std::accumulate (std::next (instr.begin ()), instr.end (),
                                std::ref (func.print (out << ' ', instr[0])),
                                [&](std::ostream& o, const ir_static_operand& op) {
                                  return std::ref (func.print (o << ' ',  op));
                                });
      }

      return out;
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

  std::ostream&
  ir_static_function::
  print (std::ostream& out, const ir_static_instruction& instr) const
  {
    constexpr auto map = ir_metadata::generate_map<instruction_printer_mapper> ();
    return map[instr.get_metadata ()] (out, instr, *this);
  }

  std::ostream&
  ir_static_function::
  print (std::ostream& out, const ir_static_block& block) const
  {
    if (block.empty ())
      return out;

    return std::accumulate (std::next (block.begin ()), block.end (),
                            std::ref (print (out, block[0])),
                            [&](std::ostream& o, const ir_static_instruction& instr) {
                              return std::ref (print (o << '\n', instr));
                            });
  }

  std::ostream&
  operator<< (std::ostream& out, const ir_static_function& func)
  {
    class indenter
      : public std::streambuf
    {
    public:
      explicit
      indenter (const std::ostream& out)
        : m_buf (out.rdbuf ())
      { }

    protected:
      int
      overflow (int c) override
      {
        if (c == '\n')
          m_needs_indent = true;
        else if (m_needs_indent)
        {
          m_buf->sputn ("  ", 2);
          m_needs_indent = false;
        }

        return m_buf->sputc (static_cast<char> (c));
      }

    private:
      std::streambuf *m_buf;
      bool            m_needs_indent = true;
    };

    auto print_block = [&func] (std::ostream& o, const ir_static_block& block) {
      o << func.get_block_name (block) << ":\n";

      indenter indented_buffer { o };
      func.print (std::ostream { &indented_buffer }, block);

      return std::ref (o);
    };

    return std::accumulate (std::next (func.begin ()), func.end (), print_block (out, func[0]),
                            [&](std::ostream& o, const ir_static_block& block) {
                              return print_block (o << "\n\n", block);
                            });
  }

}

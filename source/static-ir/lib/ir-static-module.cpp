/** ir-static-module.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-ir-static-ir/ir-static-module.hpp"

#include "gch/octave-ir-static-ir/ir-static-block.hpp"
#include "gch/octave-ir-static-ir/ir-static-instruction.hpp"
#include "gch/octave-ir-static-ir/ir-static-variable.hpp"

#include <numeric>
#include <ostream>

namespace gch
{

  ir_static_module::
  ir_static_module (std::string_view name, ir_processed_id id, container_type&& blocks,
                    std::vector<ir_static_variable>&& vars)
    : m_name      (name),
      m_id        (id),
      m_blocks    (std::move (blocks)),
      m_variables (std::move (vars))
  { }

  auto
  ir_static_module::
  begin (void) const noexcept
    -> citer
  {
    return m_blocks.begin ();
  }

  auto
  ir_static_module::
  end (void) const noexcept
    -> citer
  {
    return m_blocks.end ();
  }

  auto
  ir_static_module::
  rbegin (void) const noexcept
    -> criter
  {
    return m_blocks.rbegin ();
  }

  auto
  ir_static_module::
  rend (void) const noexcept
    -> criter
  {
    return m_blocks.rend ();
  }

  auto
  ir_static_module::
  front (void) const
    -> cref
  {
    return m_blocks.front ();
  }

  auto
  ir_static_module::
  back (void) const
    -> cref
  {
    return m_blocks.back ();
  }

  auto
  ir_static_module::
  size (void) const noexcept
    -> size_ty
  {
   return m_blocks.size ();
  }

  bool
  ir_static_module::
  empty (void) const noexcept
  {
    return m_blocks.empty ();
  }

  auto
  ir_static_module::
  operator[] (size_type pos) const
    -> const_reference
  {
    return m_blocks[pos];
  }

  auto
  ir_static_module::
  variables_begin (void) const noexcept
    -> variables_const_iterator
  {
    return m_variables.begin ();
  }

  auto
  ir_static_module::
  variables_end (void) const noexcept
    -> variables_const_iterator
  {
    return m_variables.end ();
  }

  std::string_view
  ir_static_module::
  get_name (void) const noexcept
  {
    return m_name;
  }

  std::string
  ir_static_module::
  get_block_name (const ir_static_block& block) const
  {
    return std::to_string (std::distance (&m_blocks[0], &block));
  }

  std::ostream&
  operator<< (std::ostream& out, const ir_static_module& module)
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
        if (c != '\n' && m_needs_indent)
        {
          m_buf->sputn ("  ", 2);
          m_needs_indent = false;
        }
        else
          m_needs_indent = true;

        return m_buf->sputc (c);
      }

    private:
      std::streambuf *m_buf;
      bool            m_needs_indent = true;
    };

    auto print_block = [&module](std::ostream& o, const ir_static_block& block)
                       {
                         o << module.get_block_name (block) << ":\n";

                         indenter indented_buffer { o };
                         std::ostream { &indented_buffer } << block;

                         return std::ref (o);
                       };

    return std::accumulate (std::next (module.begin ()), module.end (),
                            print_block (out, module[0]),
                            [&](std::ostream& o, const auto& block)
                            {
                              return print_block (o << "\n\n", block);
                            });
  }

}

/** ir-static-unit.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-ir-static-ir/ir-static-unit.hpp"

#include "gch/octave-ir-static-ir/ir-static-block.hpp"
#include "gch/octave-ir-static-ir/ir-static-instruction.hpp"
#include "gch/octave-ir-static-ir/ir-static-variable.hpp"

#include <numeric>
#include <ostream>

namespace gch
{

  ir_static_unit::
  ir_static_unit (std::string_view name, ir_processed_id id, container_type&& blocks,
                  std::vector<ir_static_variable>&& vars)
    : m_name      (name),
      m_id        (id),
      m_blocks    (std::move (blocks)),
      m_variables (std::move (vars))
  { }

  auto
  ir_static_unit::
  begin (void) const noexcept
    -> citer
  {
    return m_blocks.begin ();
  }

  auto
  ir_static_unit::
  end (void) const noexcept
    -> citer
  {
    return m_blocks.end ();
  }

  auto
  ir_static_unit::
  rbegin (void) const noexcept
    -> criter
  {
    return m_blocks.rbegin ();
  }

  auto
  ir_static_unit::
  rend (void) const noexcept
    -> criter
  {
    return m_blocks.rend ();
  }

  auto
  ir_static_unit::
  front (void) const
    -> cref
  {
    return m_blocks.front ();
  }

  auto
  ir_static_unit::
  back (void) const
    -> cref
  {
    return m_blocks.back ();
  }

  auto
  ir_static_unit::
  size (void) const noexcept
    -> size_ty
  {
   return m_blocks.size ();
  }

  bool
  ir_static_unit::
  empty (void) const noexcept
  {
    return m_blocks.empty ();
  }

  auto
  ir_static_unit::
  num_blocks (void) const noexcept
    -> size_ty
  {
   return size ();
  }

  auto
  ir_static_unit::
  operator[] (size_type pos) const
    -> const_reference
  {
    return m_blocks[pos];
  }

  auto
  ir_static_unit::
  variables_begin (void) const noexcept
    -> variables_const_iterator
  {
    return m_variables.begin ();
  }

  auto
  ir_static_unit::
  variables_end (void) const noexcept
    -> variables_const_iterator
  {
    return m_variables.end ();
  }

  std::string_view
  ir_static_unit::
  get_name (void) const noexcept
  {
    return m_name;
  }

  ir_processed_id
  ir_static_unit::
  get_id (void) const noexcept
  {
    return m_id;
  }

  std::string
  ir_static_unit::
  get_block_name (const ir_static_block& block) const
  {
    return std::to_string (std::distance (&m_blocks[0], &block));
  }

  std::ostream&
  operator<< (std::ostream& out, const ir_static_unit& unit)
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

        return m_buf->sputc (static_cast<char> (c));
      }

    private:
      std::streambuf *m_buf;
      bool            m_needs_indent = true;
    };

    auto print_block = [&unit](std::ostream& o, const ir_static_block& block)
                       {
                         o << unit.get_block_name (block) << ":\n";

                         indenter indented_buffer { o };
                         std::ostream { &indented_buffer } << block;

                         return std::ref (o);
                       };

    return std::accumulate (std::next (unit.begin ()), unit.end (),
                            print_block (out, unit[0]),
                            [&](std::ostream& o, const auto& block)
                            {
                              return print_block (o << "\n\n", block);
                            });
  }

}

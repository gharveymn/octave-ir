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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "ir-instruction.h"
#include "ir-operand.h"
#include "ir-variable.h"
#include <algorithm>

namespace octave
{

  constexpr ir_type::impl ir_type::instance<ir_block_ref>::m_impl;
  constexpr ir_type::impl ir_type::instance<ir_def_ref>::m_impl;

  constexpr ir_type::impl ir_type::instance<ir_phi_arg::value_type>::m_impl;
  constexpr ir_type ir_type::instance<ir_phi_arg::value_type>::m_members[];

  //
  // ir_instruction
  //

  ir_instruction::iter
  ir_instruction::erase (citer pos)
  {
    return m_args.erase (pos);
  }

  //
  // ir_def_instruction
  //

  ir_def_instruction::ir_def_instruction (ir_basic_block& blk,
                                          ir_variable& ret_var,
                                          ir_type ret_ty)
    : ir_instruction (blk),
      m_ret (ret_var.create_def (ret_ty, *this))
  { }

  ir_def_instruction::~ir_def_instruction (void) = default;

  void
  ir_def_instruction::unlink_propagate (const instr_citer pos)
  {
    if (m_ret.has_uses ())
      {
        ir_def *prev_joined = m_ret.get_var ().join (get_block (), pos);
        prev_joined.transfer_from (m_ret);
      }
    if (m_ret.has_uses ())
      throw ir_exception ("return def unexpectedly had uses after unlinking");
  }

  //
  // ir_assign
  //

  ir_assign::ir_assign (ir_basic_block& blk, ir_variable& var, ir_def& src)
    : ir_def_instruction (blk, var, src.get_type ()),
      m_src (emplace_back<ir_use> (src.create_use (*this)))
  { }

  //
  // ir_fetch
  //

  ir_fetch::ir_fetch (ir_basic_block& blk, ir_variable& ret_var)
    : ir_def_instruction (blk, ret_var, ir_type::get<any> ()),
      m_name (emplace_back<ir_constant<std::string>> (ret_var.get_name ()))
  { }

  //
  // ir_branch
  //

  ir_branch::ir_branch (ir_basic_block& blk, ir_basic_block& dst)
    : ir_instruction (blk),
      m_dest_block (emplace_back<ir_block_ref> (&dst))
  { }

  //
  // ir_cbranch
  //

  ir_cbranch::ir_cbranch (ir_basic_block& blk, ir_def& d,
                          ir_basic_block& tblk, ir_basic_block& fblk)
    : ir_instruction (blk),
      m_condvar (emplace_back<ir_use> (d.create_use (*this))),
      m_tblock (emplace_back<ir_block_ref> (&tblk)),
      m_fblock (emplace_back<ir_block_ref> (&fblk))
  { }

  //
  // ir_convert
  //

  ir_convert::ir_convert (ir_basic_block& blk, ir_variable& ret_var,
                          ir_type ty, ir_def& d)
    : ir_def_instruction (blk, ret_var, ty),
      m_src (emplace_back<ir_use> (d.create_use (*this)))
  { }

  //
  // ir_phi
  //

  void
  ir_phi::append (ir_basic_block *blk, ir_def *dptr)
  {
    if (dptr == nullptr)
      {
        m_undef_blocks.push_back (blk);
        get_return ().set_needs_init_check (true);
      }
    else
      {
        if (dptr->needs_init_check ())
          get_return ().set_needs_init_check (true);
        emplace_back<ir_phi_arg> (blk, dptr);
      }
  }

  ir_phi::iter
  ir_phi::erase (const ir_basic_block *blk)
  {
    for (citer cit = begin (); cit != end (); ++cit)
      {
        ir_phi_arg *arg = static_cast<ir_phi_arg *> (cit->get ());
        if (arg->get<0> ().value () == blk)
          return ir_instruction::erase (cit);
      }
    throw ir_exception ("specified blk not found in phi node");
  }

  ir_def *
  ir_phi::find (const ir_basic_block *blk)
  {
    for (citer cit = begin (); cit != end (); ++cit)
      {
        ir_phi_arg *arg = static_cast<ir_phi_arg *> (cit->get ());
        if (arg->get<0> ().value () == blk)
          return arg->get<1> ().value ();
      }
    throw ir_exception ("Specified block not found in the phi node.");
  }

}

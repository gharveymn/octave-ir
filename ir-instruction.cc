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
  
  constexpr ir_type::impl ir_type::instance<ir_phi_arg::value_type>::m_impl;
  constexpr ir_type ir_type::instance<ir_phi_arg::value_type>::m_members[];
  
  //
  // ir_instruction
  //
  
  ir_instruction::ir_instruction (const ir_basic_block& blk)
    : m_block (blk)
  { }
  
  template <typename T, typename ...Args>
  enable_if_t<std::is_base_of<ir_operand, T>::value, const T> *
  ir_instruction::emplace_back (Args&&... args)
  {
    std::unique_ptr<T> uptr = make_unique<T> (std::forward<Args> (args)...);
    T *ret = uptr.get ();
    m_args.emplace_back (std::move (uptr));
    return ret;
  }
  
  template <typename T, typename ...Args>
  enable_if_t<std::is_base_of<ir_operand, T>::value, ir_instruction::citer>
  ir_instruction::emplace_before (citer pos, Args&&... args)
  {
    return m_args.insert (pos, make_unique<T> (std::forward<Args> (args)...));
  }
  
  ir_instruction::iter
  ir_instruction::erase (citer pos)
  {
    return m_args.erase (pos);
  }
  
  ir_variable::def&
  ir_instruction::get_return (void)
  {
    throw ir_exception ("This instruction has no return.");
  }
  
  //
  // ir_def_instruction
  //

  ir_def_instruction::ir_def_instruction (const ir_basic_block& blk,
                                          ir_variable& var, ir_type ty)
    : ir_instruction (blk),
      m_ret (var, ty, *this)
  { }
  
  ir_def_instruction::~ir_def_instruction (void) = default;
  
  ir_variable::def&
  ir_def_instruction::get_return (void)
  {
    return m_ret;
  }
  
  //
  // ir_fetch
  //
  
  ir_fetch::ir_fetch (const ir_basic_block& blk, ir_variable& var)
    : ir_def_instruction (blk, var, ir_type::get<any> ()),
      m_name (emplace_back<ir_constant<std::string>> (var.get_name ()))
  { }
  
  //
  // ir_branch
  //
  
  ir_branch::ir_branch (const ir_basic_block& blk, const ir_basic_block& dst)
    : ir_instruction (blk),
      m_dest_block (emplace_back<ir_block_ref> (dst))
  { }

  //
  // ir_cbranch
  //
  
  //
  // ir_convert
  //
  
  
  ir_convert::ir_convert (const ir_basic_block& blk, ir_variable& ret_var,
                          ir_type ty, def& d)
    : ir_def_instruction (blk, ret_var, ty),
      m_use (emplace_back<use> (d, *this))
  { }
  
  //
  // ir_phi
  //

  ir_phi::ir_phi (const ir_basic_block& blk, ir_variable& var, ir_type ty,
    const input_vec& pairs)
    : ir_def_instruction (blk, var, ty)
  {
    for (const input_type& p : pairs)
      {
        if (p.second == nullptr)
          m_undef_blocks.push_back (&p.first);
        else
          emplace_back<arg_type> (p.first, use (*p.second, *this));
      }
  }
  
  void
  ir_phi::append (const ir_basic_block& blk, ir_variable::def& d)
  {
    emplace_back<arg_type> (blk, d.create_use (*this));
  }
  
  ir_phi::iter
  ir_phi::erase (const ir_basic_block* blk)
  {
    for (citer cit = begin (); cit != end (); ++cit)
      {
        arg_type *arg = static_cast<arg_type *> (cit->get ());
        if (&arg->get<0> ().value () == blk)
          return ir_instruction::erase (cit);
      }
    throw ir_exception ("specified blk not found in phi node");
  }
  
  const ir_variable::use&
  ir_phi::find (const ir_basic_block* blk)
  {
    for (citer cit = begin (); cit != end (); ++cit)
      {
        arg_type *arg = static_cast<arg_type *> (cit->get ());
        if (&arg->get<0> ().value () == blk)
          return arg->get<1> ();
      }
    throw ir_exception ("Specified block not found in the phi node.");
  }

}

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

#include "ir-variable.h"
#include "ir-common-util.h"
#include "ir-type.h"
#include "ir-instruction.h"
#include "ir-component.h"

#include <iostream>

namespace octave
{

  ir_variable::def
  ir_variable::create_def (ir_type ty, const ir_instruction& instr)
  {
    return { *this, ty, instr };
  }

  // def

  ir_variable::def::def (ir_variable& var, ir_type ty,
                         const ir_instruction& instr)
    : m_var (&var),
      m_type (ty),
      m_instr (instr)
  {
    var.track_def (this);
  }

  std::ostream&
  ir_variable::def::print (std::ostream& os) const
  {
    return os << get_name () << get_id ();
  }

  template <typename InputIt>
  ir_type ir_variable::def::common_type (InputIt first, InputIt last)
  {
    ir_type curr_ty = *first->get_type ();
    for (; first != last; ++first)
      {
        if (curr_ty == ir_type::get<any> ())
          return curr_ty;
        curr_ty = ir_type::lca (curr_ty, *first->get_type ());
      }
    return curr_ty;
  }

  // use

  ir_variable::use::use (def& d, const ir_instruction& instr)
    : ir_operand (d.get_type ()),
      m_def (&d),
      m_self_iter (d.track_use (this)),
      m_instr (&instr)
  { }
  
  ir_variable::use::use (use&& u) noexcept
    : m_def (u.m_def),
      m_self_iter (u.m_self_iter),
      m_instr (u.m_instr)
  {
    *m_self_iter = this;

    // invalidate the old use
    u.invalidate();
  }

  std::string ir_variable::use::get_name (void) const
  {
    return m_def->get_name ();
  }

  std::size_t ir_variable::use::get_id (void)
  {
    return std::distance (m_def->begin (), m_self_iter);
  }

  ir_variable::use
  ir_variable::def::create_use (const ir_instruction& instr)
  {
    return { *this, instr };
  }

  std::string ir_variable::def::get_name (void) const
  {
    return m_var->get_name ();
  }

  std::size_t ir_variable::def::get_id (void) const
  {
    return std::distance (m_var->begin (), m_self_iter);
  }

  // ir_variable

  ir_type
  ir_variable::normalize_types (block_def_vec& pairs)
  {

    using pair_iter = block_def_vec::iterator;

    if (pairs.empty ())
      throw ir_exception ("block-def pair list unexpectedly empty.");

    // find the closest common type
    ir_type common_ty = pairs.front ().second->get_type ();
    for (const block_def_pair& p : pairs)
      {
        if (common_ty == ir_type::get<any> ())
          break;
        common_ty = ir_type::lca (common_ty, p.second->get_type ());
      }

    for (block_def_pair& p : pairs)
      {
        ir_basic_block& blk = p.first;
        def *d = p.second;
        if (d->get_type () != common_ty)
          {
            ir_convert *instr = blk.emplace_back<ir_convert> (common_ty, d);
            p.second = &instr->get_return ();
          }
      }

    return common_ty;

  }

  void
  ir_variable::mark_uninitialized (ir_basic_block& blk)
  {
    if (m_uninit_sentinel == nullptr)
      {
        ir_basic_block *entry_block;
      }
  }
  
  constexpr ir_type::impl ir_type::instance<ir_variable::use>::m_impl;

//  ir_variable::use
//  ir_variable::create_use (ir_basic_block& blk, const ir_instruction& instr)
//  {
//    // search for def
//    // if it's in this block we're done
//    // if not we need to check if we might need a phi node, checking all
//    // branches
//
//    if (blk.back ().get () == &instr)
//      {
//        // this is the last instruction, so we can just check the cache
//        if (def *d = blk.fetch_cached_def (*this))
//          return {*d, instr};
//      }
//    else if (def *d = blk.fetch_proximate_def (*this, instr))
//      return {*d, instr};
//
//    // def was not in the initial block
//    def *d_latest = blk.join_pred_defs (*this);
//
//    if (d_latest == nullptr)
//      throw ir_exception ("variable not defined.");
//
//    // cache the latest def
//    blk.set_cached_def (*d_latest);
//
//    return {*d_latest, instr};
//
//  }

}


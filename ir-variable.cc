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
#include "ir-type-std.h"
#include "ir-instruction.h"
#include "ir-component.h"
#include "ir-function.h"

#include <iostream>

namespace octave
{

  constexpr ir_type::impl ir_type::instance<ir_def* >::m_impl;

  constexpr ir_type::impl ir_type::instance<ir_use *>::m_impl;

  //
  // ir_variable
  //

  ir_variable::ir_variable (ir_function& m, std::string name)
    : m_function (m),
      m_name (std::move (name)),
      m_def_observer (this)
  { }

//  ir_variable::ir_variable (ir_variable&& o) noexcept
//    : m_function (o.m_function),
//      m_name (std::move (o.m_name)),
//      m_defs (std::move (o.m_defs)),
//      m_sentinel (std::move (o.m_sentinel))
//  {
//    o.m_defs.clear ();
//  }

  ir_def
  ir_variable::create_def (ir_type ty, const ir_def_instruction& instr)
  {
    return { m_def_observer, ty, instr };
  }

  ir_type
  ir_variable::normalize_types (block_def_vect& pairs)
  {
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
    if (common_ty == ir_type::get<void> ())
      throw ir_exception ("no common type");

    for (block_def_pair& p : pairs)
      {
        ir_basic_block& blk = p.first;
        ir_def *d = p.second;
        if (d->get_type () != common_ty)
          {
            ir_convert& instr = blk.emplace_back<ir_convert> (d->get_var (),
                                                              common_ty, *d);
            p.second = &instr.get_return ();
          }
      }
    return common_ty;
  }

  ir_variable&
  ir_variable::get_sentinel (void)
  {
    if (! has_sentinel ())
      initialize_sentinel ();
    return *m_sentinel;
  }

  std::string
  ir_variable::get_sentinel_name (void) const
  {
    return "_" + m_name + "_sentinel";
  }

  ir_def&
  ir_variable::join (ir_basic_block& blk)
  {
    return join (blk, blk.body_end ());
  }

  ir_def&
  ir_variable::join (ir_basic_block& blk, instr_citer pos)
  {
    ir_def *ret = blk.fetch_proximate_def (*this, pos);
    if (ret == nullptr)
      ret = blk.join_pred_defs (*this);

    // if ret is still nullptr then we need to insert a fetch instruction
    if (ret == nullptr)
      return blk.emplace_before<ir_fetch> (pos, *this).get_return ();
    else
      {
        // if the ir_def was created by a phi node, there may be
      }

    return *ret;
  }

  void
  ir_variable::initialize_sentinel (void)
  {
    m_sentinel = octave::make_unique<ir_variable> (get_module (),
                                                   get_sentinel_name ());
    // set false (meaning undecided state) at the beginning of the module.
    ir_basic_block *entry = get_function ().get_entry_block ();
    entry->emplace_front<ir_assign> (*m_sentinel,
                                     ir_constant<bool> {false});
  }

//  void
//  ir_variable::mark_uninit (ir_basic_block& blk)
//  {
//    ir_variable& sentinel = get_sentinel ();
//  }

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

  //
  // ir_def
  //

  ir_def::ir_def (observee::observer_type& obs, ir_type ty,
                  const ir_def_instruction& instr)
    : observee (obs),
      m_use_tracker (this),
      m_type (ty),
      m_instr (instr),
      m_needs_init_check (false)
  { }

  ir_def::ir_def (ir_def&& d) noexcept
    : observee (std::move (d)),
      m_use_tracker (std::move (d.m_use_tracker)),
      m_type (d.m_type),
      m_instr (d.m_instr),
      m_needs_init_check (d.m_needs_init_check)
  { }

  ir_variable&
  ir_def::get_var (void) const noexcept (false)
  {
    if (has_parent ())
      return *get_parent ();
    throw ir_exception ("ir_def is in an invalid state and has no ir_variable.");
  }

  ir_use
  ir_def::create_use (const ir_instruction& instr)
  {
    return { m_use_tracker, instr };
  }

  template <typename InputIt>
  ir_type
  ir_def::common_type (InputIt first, InputIt last)
  {
    ir_type curr_ty = *first->get_type ();
    for (; first != last; ++first)
      {
        if (curr_ty == ir_type::get<any> ())
          return curr_ty;
        curr_ty = ir_type::lca (curr_ty, *first->get_type ());
      }
    if (curr_ty == ir_type::get<void> ())
      throw ir_exception ("no common type");
    return curr_ty;
  }

  constexpr ir_basic_block&
  ir_def::get_block () const
  {
    return get_instruction ().get_block ();
  }

  std::ostream&
  ir_def::print (std::ostream& os) const
  {
    return os << get_name () << get_id ();
  }

  std::string
  ir_def::get_name (void) const
  {
    return get_var ().get_name ();
  }

  std::size_t
  ir_def::get_id (void) const
  {
    return get_position ();
  }

  //
  // ir_use
  //

  ir_use::ir_use (observee::observer_type& obs, const ir_instruction& instr)
    : observee (obs),
      m_instr (&instr)
  { }

  ir_use::ir_use (ir_use&& u) noexcept
    : observee (std::move (u)),
      m_instr (u.m_instr)
  { }

  std::string
  ir_use::get_name (void) const
  {
    return get_def ().get_name ();
  }

  std::size_t
  ir_use::get_id (void)
  {
    return get_position ();
  }

  ir_type
  ir_use::get_type (void) const
  {
    return get_def ().get_type ();
  }

  ir_def&
  ir_use::get_def (void) const noexcept (false)
  {
    if (has_parent ())
      return *get_parent ();
    throw ir_exception ("ir_use is in an invalid state and has no ir_def.");
  }

}


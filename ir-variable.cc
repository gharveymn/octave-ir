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
#include "ir-module.h"

#include <iostream>

namespace octave
{
  
  constexpr ir_type::impl ir_type::instance<ir_variable::def>::m_impl;
  template struct ir_type::instance<ir_variable::def *>;
  
  constexpr ir_type::impl ir_type::instance<ir_variable::use>::m_impl;
  template struct ir_type::instance<ir_variable::use *>;

  //
  // def
  //

  ir_variable::def::def (ir_variable& var, ir_type ty,
                         const ir_def_instruction& instr)
    : m_var (&var),
      m_self_iter (var.track_def (this)),
      m_type (ty),
      m_instr (instr),
      m_needs_init_check (false)
  { }

  ir_variable::def::def (def&& d) noexcept
    : m_var (d.m_var),
      m_self_iter (d.m_self_iter),
      m_type (d.m_type),
      m_instr (d.m_instr),
      m_uses (std::move (d.m_uses)),
      m_needs_init_check (d.m_needs_init_check)
  {
    *m_self_iter = this;

    // invalidate the old def
    d.invalidate ();
    d.m_uses.clear ();
  }

  ir_variable::def::~def (void) noexcept
  {
    if (m_var)
      m_var->untrack_def (m_self_iter);

    for (use *u : m_uses)
      u->invalidate ();
  }

  ir_variable& ir_variable::def::get_var (void) const noexcept (false)
  {
    if (m_var)
      return *m_var;
    throw ir_exception ("def is in an invalid state and has no ir_variable.");
  }

  ir_variable::use
  ir_variable::def::create_use (const ir_instruction& instr)
  {
    return { *this, instr };
  }

  ir_variable::use_iter ir_variable::def::track_use (use *u)
  {
    return m_uses.insert (m_uses.end (), u);
  }

  void ir_variable::def::untrack_use (use_citer cit)
  {
    m_uses.erase (cit);
  }

  template <typename InputIt>
  ir_type
  ir_variable::def::common_type (InputIt first, InputIt last)
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

  std::ostream&
  ir_variable::def::print (std::ostream& os) const
  {
    return os << get_name () << get_id ();
  }

  std::string
  ir_variable::def::get_name (void) const
  {
    return get_var ().get_name ();
  }

  std::size_t
  ir_variable::def::get_id (void) const
  {
    return std::distance (get_var ().begin (), m_self_iter);
  }

  //
  // use
  //

  ir_variable::use::use (def& d, const ir_instruction& instr)
    : m_def (&d),
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

  ir_variable::use::~use (void) noexcept
  {
    if (has_def ())
      m_def->untrack_use (m_self_iter);
  }

  std::string
  ir_variable::use::get_name (void) const
  {
    return get_def ().get_name ();
  }

  std::size_t
  ir_variable::use::get_id (void)
  {
    return std::distance (get_def ().begin (), m_self_iter);
  }

  ir_type
  ir_variable::use::get_type (void) const
  {
    return get_def ().get_type ();
  }

  void
  ir_variable::use::replace_def (def& new_def) noexcept (false)
  {
    def& old_def = get_def ();

    // check if this is a valid replacement
    if (&new_def.get_var () != &old_def.get_var ())
      throw ir_exception ("ir_variable of replacement def must match the "
                          "current ir_variable.");
    if (new_def.get_type() != old_def.get_type ())
      throw ir_exception ("ir_type of replacement def must match the "
                          "current ir_type.");

    use_iter it = new_def.track_use (this);
    old_def.untrack_use (m_self_iter);
    m_def = &new_def;
    m_self_iter = it;
  }

  ir_variable::def&
  ir_variable::use::get_def (void) const noexcept (false)
  {
    if (m_def)
      return *m_def;
    throw ir_exception ("use is in an invalid state and has no def.");
  }

  //
  // ir_variable
  //

  ir_variable::ir_variable (ir_module& m, std::string name)
    : m_module (m),
      m_name (std::move (name))
  { }

//  ir_variable::ir_variable (ir_variable&& o) noexcept
//    : m_module (o.m_module),
//      m_name (std::move (o.m_name)),
//      m_defs (std::move (o.m_defs)),
//      m_uninit_sentinel (std::move (o.m_uninit_sentinel))
//  {
//    o.m_defs.clear ();
//  }

  ir_variable::~ir_variable (void) noexcept
  {
    // Remove references to this in the tracked defs and uses
    for (def *d : m_defs)
      d->invalidate ();
  }

  ir_variable::def
  ir_variable::create_def (ir_type ty, const ir_def_instruction& instr)
  {
    return { *this, ty, instr };
  }

  ir_type
  ir_variable::normalize_types (block_def_vec& pairs)
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
        def *d = p.second;
        if (d->get_type () != common_ty)
          {
            ir_convert& instr = blk.emplace_back<ir_convert> (d->get_var (),
                                                              common_ty, d);
            p.second = &instr.get_return ();
          }
      }
    return common_ty;
  }

  ir_variable&
  ir_variable::get_sentinel (void)
  {
    if (m_uninit_sentinel == nullptr)
      {
        m_uninit_sentinel = octave::make_unique<ir_variable> (get_module (),
                                                        get_sentinel_name ());
        // set false (meaning undecided state) at the beginning of the module.
        ir_basic_block *entry = m_module.get_entry_block ();
        entry->emplace_front<ir_assign> (m_uninit_sentinel,
                                         ir_constant<bool> {false});
      }
    return *m_uninit_sentinel;
  }

  std::string
  ir_variable::get_sentinel_name (void) const
  {
    return "_" + m_name + "_sentinel";
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

}


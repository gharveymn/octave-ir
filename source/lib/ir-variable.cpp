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

#include <ir-variable.hpp>
#include <ir-common-util.hpp>
#include <ir-type-std.hpp>
#include <ir-instruction.hpp>
#include <ir-component.hpp>
#include <ir-function.hpp>

#include <algorithm>
#include <iostream>
#include <numeric>

namespace gch
{

  constexpr ir_type::impl ir_type::instance<ir_def* >::m_impl;

  constexpr ir_type::impl ir_type::instance<ir_use *>::m_impl;

  //
  // ir_variable
  //

  ir_variable::ir_variable (ir_function& m, std::string name)
    : m_function (m),
      m_name (std::move (name))
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
  ir_variable::create_def (ir_type ty, ir_def_instruction& instr)
  {
    return { *this, ty, instr };
  }

  ir_type
  ir_variable::normalize_types (block_def_vect& pairs)
  {
    if (pairs.empty ())
      throw ir_exception ("block-def pair list unexpectedly empty.");

    auto get_block  = [] (auto&& pair)
                      {
                        return std::get<nonnull_ptr<ir_basic_block>> (pair);
                      };

    auto get_def  = [] (auto&& pair)
                    {
                      return std::get<nonnull_ptr<ir_def>> (pair);
                    };

    auto get_type = [&get_def] (auto&& pair) -> ir_type
                    {
                      return get_def (pair)->get_type ();
                    };

    // find the closest common type
    ir_type common_ty = std::accumulate (++pairs.begin (), pairs.end (), get_type (pairs.front ()),
                                         [&get_type] (ir_type curr, auto&& pair)
                                         {
                                           return ir_type::lca (curr, get_type (pair));
                                         });

    if (common_ty == ir_type_v<void>)
      throw ir_exception ("no common type");

    std::for_each (pairs.begin (), pairs.end (),
                   [&get_type, common_ty] (auto& pair)
                   {
                     auto& [block, def] = pair;
                     if (def->get_type () != common_ty)
                     {
                       auto& instr = block->template emplace_back<ir_convert> (common_ty, def);
                       def = instr->get_def ();
                     }
                   });
    return common_ty;
  }

  ir_variable&
  ir_variable::get_sentinel (void)
  {
    if (! has_sentinel ())
      return initialize_sentinel ();
    return *m_sentinel;
  }

  std::string
  ir_variable::get_sentinel_name (void) const
  {
    return "_" + m_name + "_sentinel";
  }

  optional_ref<ir_def>
  ir_variable::join (ir_basic_block& blk)
  {
    return join (blk, blk.body_end ());
  }

  optional_ref<ir_def>
  ir_variable::join (ir_basic_block& blk, instr_citer pos)
  {
    gch::optional_ref<ir_def> ret = blk.get_latest_def_before (*this, pos);
    if (! ret.has_value ())
      ret = blk.join_preceding_defs (*this);

    // if ret is still nullptr then we need to insert a fetch instruction
    if (! ret.has_value ())
      return blk.emplace_before<ir_fetch> (pos, *this).get_def ();
    else
      {
        // if the ir_def was created by a phi node, there may be
      }
      
    // guaranteed return  
    return *ret;
  }
  
  void
  ir_variable::set_type (ir_type ty)
  {
    if (ty == m_type)
      return;
  
    std::for_each (begin (), end (), [&ty](ir_def& def) { def.propagate_type (ty); });
    m_type = ty;
  }

  ir_variable&
  ir_variable::initialize_sentinel (void)
  {
    m_sentinel = std::make_unique<ir_variable> (get_module (), get_sentinel_name ());
    // set false (meaning undecided state) at the beginning of the module.
    ir_basic_block& entry = get_function ().get_entry_block ();
    entry.emplace_front<ir_assign> (*m_sentinel, ir_constant<bool> {false});
  }
  
  // ir_variable::tracker_type::citer
  // ir_variable::find (const ir_def& d) const
  // {
  //   return std::find_if (m_def_tracker.begin (), m_def_tracker.end (),
  //                        [&d](const ir_def& x) { return &x == &d; });
  // }

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

  ir_def::ir_def (ir_variable& tkr, ir_def_instruction& instr)
    : base_reporter (tag::bind, tkr),
      m_instr (instr),
      m_needs_init_check (false)
  { }

  void
  ir_def::propagate_type (ir_type ty)
  {
    std::for_each (begin (), end (), [&ty] (ir_use_timeline& tl) { tl.propagate_type (ty); });
  }

  template <typename InputIt>
  ir_type
  common_type (InputIt first, InputIt last)
  {
    ir_type curr_ty = *first->get_type ();
    for (; first != last; ++first)
      {
        if (curr_ty == ir_type::get<any> ())
          return curr_ty;
        curr_ty = ir_type::lca (curr_ty, *first->get_type ());
      }
    // couldn't find a common type
    if (curr_ty == ir_type::get<void> ())
      throw ir_exception ("no common type");
    return curr_ty;
  }
  
  ir_basic_block& ir_def::get_block (void) const noexcept
  {
    return get_instruction ().get_block ();
  }

  std::ostream&
  ir_def::print (std::ostream& os) const
  {
    return os << get_name () << get_id ();
  }

  const std::string&
  ir_def::get_name (void) const
  {
    return get_var ().get_name ();
  }

  std::size_t
  ir_def::get_id (void) const
  {
    return get_position ();
  }

  bool ir_def::has_uses (void) const noexcept
  {
    return std::any_of (begin (), end (),
                        [] (const ir_use_timeline& dt) { return dt.has_remotes (); });
  }

  //
  // ir_use
  //

  ir_use::ir_use (typename reporter_type::remote_interface_type& tkr, 
                  ir_instruction& instr)
    : reporter_type (tag::bind, tkr),
      m_instr (instr)
  { }
  
  ir_use_timeline&
  ir_use::get_timeline (void) noexcept
  {
    return get_remote ();
  }
  
  const ir_use_timeline&
  ir_use::get_timeline (void) const noexcept
  {
    return get_remote ();
  }

  ir_def&
  ir_use::get_def (void) noexcept
  {
    return get_remote ().get_def ();
  }
  
  const ir_def&
  ir_use::get_def (void) const noexcept
  {
    return get_remote ().get_def ();
  }
  
  ir_variable&
  ir_use::get_var (void) noexcept
  {
    return get_def ().get_var ();
  }
  
  const ir_variable&
  ir_use::get_var (void) const noexcept
  {
    return get_def ().get_var ();
  }
  
  ir_basic_block&
  ir_use::get_block (void) noexcept
  {
    return get_instruction ().get_block ();
  }
  
  const ir_basic_block&
  ir_use::get_block (void) const noexcept
  {
    return get_instruction ().get_block ();
  }
  
  ir_type
  ir_use::get_type (void) const noexcept
  {
    return get_def ().get_type ();
  }
  
  const std::string&
  ir_use::get_name (void) const
  {
    return get_def ().get_name ();
  }
  
  std::size_t
  ir_use::get_id (void)
  {
    return get_position ();
  }
  
  ir_operand::ir_operand (ir_use_timeline& tl, ir_instruction& instr)
    : m_data (std::in_place_type<ir_use>, tl, instr)
  { }

}


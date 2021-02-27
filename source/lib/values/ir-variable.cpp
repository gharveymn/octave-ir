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

#include <values/ir-variable.hpp>
#include <utilities/ir-common-util.hpp>
#include <values/types/ir-type-std.hpp>
#include <values/ir-instruction.hpp>
#include <components/ir-component.hpp>
#include <components/ir-function.hpp>

#include <algorithm>
#include <iostream>
#include <numeric>

namespace gch
{

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

  // ir_type
  // ir_variable::normalize_types (block_def_vect& pairs)
  // {
  //   if (pairs.empty ())
  //     throw ir_exception ("block-def pair list unexpectedly empty.");
  //
  //   auto get_block  = [] (auto&& pair)
  //                     {
  //                       return std::get<nonnull_ptr<ir_block>> (pair);
  //                     };
  //
  //   auto get_def  = [] (auto&& pair)
  //                   {
  //                     return std::get<nonnull_ptr<ir_def>> (pair);
  //                   };
  //
  //   auto get_type = [&get_def] (auto&& pair) -> ir_type
  //                   {
  //                     return get_def (pair)->get_type ();
  //                   };
  //
  //   // find the closest common type
  //   ir_type common_ty = std::accumulate (++pairs.begin (), pairs.end (), get_type (pairs.front ()),
  //                                        [&get_type] (ir_type curr, auto&& pair)
  //                                        {
  //                                          return ir_type::lca (curr, get_type (pair));
  //                                        });
  //
  //   if (common_ty == ir_type_v<void>)
  //     throw ir_exception ("no common type");
  //
  //   std::for_each (pairs.begin (), pairs.end (),
  //                  [&get_type, common_ty] (auto& pair)
  //                  {
  //                    auto& [block, def] = pair;
  //                    if (def->get_type () != common_ty)
  //                    {
  //                      auto& instr = block->template emplace_back<ir_convert> (common_ty, def);
  //                      def = instr->get_def ();
  //                    }
  //                  });
  //   return common_ty;
  // }

  ir_variable&
  ir_variable::get_sentinel (void)
  {
    if (! has_sentinel ())
      return get_undef_var ();
    return *m_undef_var;
  }

  ir_variable&
  ir_variable::get_undef_var (void)
  {
    return *(m_undef_var = std::make_unique<ir_variable> (get_module (), make_undef_name (*this)));
  }

  //
  // ir_def
  //

  template <typename InputIt>
  ir_type
  common_type (InputIt first, InputIt last)
  {
    ir_type curr_ty = *first->get_type ();
    for (; first != last; ++first)
      {
        if (curr_ty == ir_type::get<any> ())
          return curr_ty;
        curr_ty = lca (curr_ty, *first->get_type ());
      }
    // couldn't find a common type
    if (curr_ty == ir_type::get<void> ())
      throw ir_exception ("no common type");
    return curr_ty;
  }

  std::ostream&
  ir_def::print (std::ostream& os) const
  {
    return os << get_name () << get_id ();
  }

  const std::string&
  ir_def::get_name (void) const
  {
    return get_variable ().get_name ();
  }

  //
  // ir_use
  //

  ir_use::ir_use (ir_instruction& instr, ir_use_timeline& ut, ir_use_timeline::citer pos)
    : reporter_type (tag::bind, ut, pos),
      m_instr (instr)
  { }

  ir_use::ir_use (ir_instruction& instr, const ir_use_info& info)
    : ir_use (instr, info.get_timeline (), info.get_insertion_position ())
  { }

  ir_use_timeline&
  ir_use::get_timeline (void) noexcept
  {
    return get_remote ();
  }

  ir_def&
  ir_use::get_def (void) noexcept
  {
    return get_remote ().get_def ();
  }

  ir_variable&
  ir_use::get_variable (void) noexcept
  {
    return get_def ().get_variable ();
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

  ir_def&
  ir_use_timeline::
  get_def (void) noexcept
  {
    return get_def_instruction ().get_def ();
  }

  ir_instruction_iter
  ir_use_timeline::
  get_def_pos (void) noexcept
  {
    return m_instruction_pos;
  }

  ir_instruction&
  ir_use_timeline::
  get_def_instruction (void) noexcept
  {
    return *get_def_pos ();
  }

  ir_variable&
  ir_use_timeline::
  get_variable (void) noexcept
  {
    return get_def ().get_variable ();
  }

  void
  ir_use_timeline::
  set_instruction_pos (const ir_instruction_iter instr) noexcept
  {
    m_instruction_pos = instr;
  }

}

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

#include <ir-block.hpp>
#include <ir-type-std.hpp>
#include <ir-variable.hpp>
#include <ir-instruction.hpp>
#include <ir-structure.hpp>

#include <nonnull_ptr.hpp>
#include <optional_ref.hpp>
#include <select-iterator.hpp>

#include <algorithm>
#include <numeric>
#include <utility>

namespace gch
{
  //
  // free functions
  //

  // has side-effects
  template <typename It>
  ir_type
  normalize_types (It first, It last)
  {
    if (first == last)
      throw ir_exception ("block-def pair list unexpectedly empty.");

    auto get_block  = [] (auto&& pair) -> ir_basic_block&
                      {
                        return std::get<ir_basic_block&> (pair);
                      };

    auto get_def  = [] (auto&& pair) -> ir_def&
                    {
                      return std::get<optional_ref<ir_def>> (pair);
                    };

    auto get_type = [&get_def] (auto&& pair) -> ir_type
                    {
                      return get_def (pair).get_type ();
                    };

    // find the closest common type
    ir_type common_ty = std::accumulate (std::next (first), last, get_type (*first),
      [&get_type] (ir_type curr, auto&& pair)
      {
        return ir_type::lca (curr, get_type (pair));
      });

    if (common_ty == ir_type::get<void> ())
      throw ir_exception ("no common type");

    std::for_each (first, last,
      [&get_def, &get_type, common_ty] (auto&& pair)
      {
        if (get_type (pair) != common_ty)
        {
          ir_convert& instr
            = get_block (pair).template emplace_back<ir_convert> (common_ty, get_def (pair));
          std::get<std::reference_wrapper<ir_def>> (pair) = instr.get_def ();
        }
      });

    for (block_def_pair& p : pairs)
      {
        ir_basic_block& blk = p.first;
        ir_def *d = p.second;
        if (d->get_type () != common_ty)
          {
            ir_convert& instr = blk.emplace_back<ir_convert> (d->get_var (),
                                                              common_ty, *d);
            p.second = &instr.get_def ();
          }
      }
    return common_ty;
  }


  //
  // use_timeline
  //

  ir_use_timeline& ir_use_timeline::operator= (ir_use_timeline&& other) noexcept
  {
    if (&other != this)
    {
      base_tracker::operator= (std::move (other));
      m_def_link.move_binding (other.m_def_link);
    }
    return *this;
  }

  ir_use_timeline::ir_use_timeline (ir_use_timeline&& other, ir_basic_block& block) noexcept
    : base_tracker (std::move (other)),
      m_def_link   (std::move (other.m_def_link), block)
  { }

  ir_use_timeline::ir_use_timeline (ir_basic_block& block)
    : m_def_link (block)
  { }

  ir_use_timeline::ir_use_timeline (ir_basic_block& block, ir_def& def)
    : m_def_link (block, def)
  { }

  void
  ir_use_timeline::rebind (ir_def& d)
  {
    m_def_link.rebind (d);
  }

  ir_instruction&
  ir_use_timeline::get_instruction (void) noexcept
  {
    return get_def ().get_instruction ();
  }

  const ir_instruction&
  ir_use_timeline::get_instruction (void) const noexcept
  {
    return get_def ().get_instruction ();
  }
  
  void
  ir_use_timeline::propagate_type (ir_type ty)
  {
    std::for_each (begin (), end (),
                   [&ty] (ir_use& use)
                   {
                     if (auto *call_instr = dynamic_cast<ir_call *> (&use.get_instruction ()))
                       call_instr->resolve ();
                   });
  }

  //
  // ir_basic_block
  //

  ir_basic_block::def_timeline&
  ir_basic_block::def_timeline::operator= (def_timeline&& other) noexcept
  {
    if (&other != this)
    {
      m_block     = other.m_block;
      m_instances = std::move (other.m_instances);
    }
    return *this;
  }

  ir_basic_block::def_timeline::citer
  ir_basic_block::def_timeline::find (const ir_instruction *instr) const
  {
    auto pos = std::find_if (m_instances.cbegin (), m_instances.cend (),
                             [instr] (const ir_use_timeline& dt)
                             {
                               return &dt.get_instruction () == instr;
                             });
    if (pos == m_instances.end ())
      throw ir_exception ("instruction not found in the timeline.");
    return pos;
  }
  
  optional_ref<ir_def>
  ir_basic_block::def_timeline::get_latest (void) noexcept
  {
    if (m_instances.empty ())
      return nullopt;
    return m_instances.back ().get_def ();
  }

  optional_ref<const ir_def>
  ir_basic_block::def_timeline::get_latest (void) const noexcept
  {
    if (m_instances.empty ())
      return nullopt;
    return m_instances.back ().get_def ();
  }

  ir_basic_block::ir_basic_block (ir_structure& parent)
    : m_parent (parent)
  { }

  ir_basic_block::~ir_basic_block (void) noexcept = default;

  void move_instruction_before (instr_citer pos, std::unique_ptr<ir_instruction>&& instr)
  {

  }

  ir_basic_block&
  ir_basic_block::split (instr_iter pivot, ir_basic_block& dest)
  {
    if (pivot == m_body.end ())
      return dest;

    // move needed timelines into dest
    for (auto&& pair : m_timeline_map)
    {
      auto& vt = std::get<def_timeline> (pair);
      auto vt_pivot = find_latest_def_before (pivot, vt).base ();
      if (vt_pivot != vt.end ())
      {
        def_timeline& dest_vt = dest.get_timeline (*std::get<ir_variable * const> (pair));
        dest_vt.splice (dest_vt.end (), vt, vt_pivot, vt.end ());
      }
    }

    // move the range into dest
    dest.m_body.splice (dest.m_body.end (), m_body, pivot, m_body.end ());
    std::for_each (pivot, dest.m_body.end (),
                   [&dest] (std::unique_ptr<ir_instruction>& u)
                   {
                     u->set_block (dest);
                   });

    return dest;
  }

  ir_basic_block::instr_iter
  ir_basic_block::erase (const instr_citer pos)
  {
    (*pos)->unlink_propagate (pos);
    return m_body.erase (pos);
  }

  ir_basic_block::instr_iter
  ir_basic_block::erase (const instr_citer first,
                         const instr_citer last) noexcept
  {
    std::for_each (instr_criter { last }, instr_criter { first },
                   [&first] (instr_cref uptr)
                   {
                     uptr->unlink_propagate (first);
                   });
    return m_body.erase (first, last);
  }
  
  std::vector<nonnull_ptr<ir_def>>
  ir_basic_block::get_latest_defs (ir_variable& var) noexcept
  {
    if (auto opt_def = get_latest_def (var))
      return { *opt_def };
    return { };
  }

  optional_ref<ir_def>
  ir_basic_block::get_latest_def (ir_variable& var)
  {
    if (auto vt = find_timeline (var))
    {
      if (auto local_latest = vt->get_latest ())
        return local_latest;
    }
    return join_defs (var);
  }

  optional_ref<ir_def>
  ir_basic_block::get_latest_def_before (ir_variable& var, const instr_citer pos) noexcept
  {
    if (pos == body_end ())
      return get_latest_def (var);
    
    if (auto dt = find_timeline (var))
    {
      if (auto found = find_latest_def_before (pos, *dt) ; found != dt->crend ())
        return found->get_def ();
    }
    return join_defs (var);
  }

  ir_basic_block::def_timeline::riter
  ir_basic_block::find_latest_def_before (instr_citer pos, def_timeline& dt) const noexcept
  {
    if (! dt.has_use_timelines ())
      return dt.rend ();
    
    if (pos == m_body.begin ())
      return dt.rend ();
    return std::find_if (dt.rbegin (), dt.rend (),
                         [rfirst = body_crbegin (),
                          rlast  = body_crend (),
                          cmp    = std::prev (pos)] (const ir_use_timeline& ut) mutable
                         {
                           for (; rfirst != rlast &&
                                  rfirst->get () != &ut.get_instruction (); ++rfirst)
                           {
                             if (*rfirst == *cmp)
                               return true;
                           }
                           return false;
                         });
  }

  ir_basic_block::def_timeline::criter
  ir_basic_block::find_latest_def_before (instr_citer pos, const def_timeline& dt) const noexcept
  {
    if (! dt.has_use_timelines ())
      return dt.crend ();
    
    if (pos == m_body.begin ())
      return dt.crend ();
    return std::find_if (dt.rbegin (), dt.rend (),
                         [rfirst = body_crbegin (),
                          rlast  = body_crend (),
                          cmp   = std::prev (pos)] (const ir_use_timeline& ut) mutable
                         {
                           for (; rfirst != rlast &&
                                  rfirst->get () != &ut.get_instruction (); ++rfirst)
                           {
                             if (*rfirst == *cmp)
                               return true;
                           }
                           return false;
                         });
  }
  
  optional_ref<ir_def>
  ir_basic_block::join_defs (ir_variable& var)
  {
    std::size_t npreds = num_preds ();
    
    if (npreds == 0)
      return nullopt;
    
    if (npreds == 1)
      return pred_front ()->get_latest_def (var);
  
    // only for debugging
    if (std::any_of (phi_begin (), phi_end (),
                     [&var](ir_phi& phi) { return &phi.get_def().get_var () == &var; }))
      throw ir_exception ("block already contains a phi instruction");
  
    // create phi instruction
    ir_phi& phi_instr = create_phi (var);
    
    std::vector<std::pair<nonnull_ptr<ir_basic_block>, nonnull_ptr<ir_def>>> incoming_defs;
    std::vector<nonnull_ptr<ir_basic_block>> indet_branchs;
    
    // TODO: remove recursion
    std::for_each (pred_begin (), pred_end (),
                   [&incoming_defs, &indet_branchs, &var] (nonnull_ptr<ir_basic_block> block)
                   {
                     optional_ref<ir_def> def = block->get_latest_def (var);
                     if (def.has_value ())
                     {
                       ir_def_instruction& def_instr = def->get_instruction ();
                       if (auto *def_phi = dynamic_cast<ir_phi *>(&def_instr))
                       {
                         if (def_phi->has_indeterminate_preds ())
                         {
                           indet_branchs.emplace_back (block);
                         }
                       }
                       incoming_defs.emplace_back (block, *def);
                     }
                     else
                     {
                       indet_branchs.emplace_back (block);
                     }
                   });
    
    std::vector<std::pair<nonnull_ptr<ir_basic_block>, ir_use>> phi_args;
    std::for_each (incoming_defs.begin (), incoming_defs.end (),
                    [&phi_instr, &phi_args, &dt = get_timeline(var)](auto&& p)
                    {
                      ir_use_timeline& ut = dt.emplace_front (*std::get<nonnull_ptr<ir_def>> (p));
                      phi_args.emplace_back (std::get<nonnull_ptr<ir_basic_block>> (p),
                                             ir_use (ut, phi_instr));
                    });
    
    return phi_instr.get_def ();
  }

  auto
  ir_basic_block::create_def_before (ir_variable& var, instr_citer pos)
  {
    // if pos has any succeeding instructions in this block or
    // the block has any successors, then we need to repoint
    // any dominated uses to the created def.
    if (has_succs (pos))

  }

  void
  ir_basic_block::set_cached_def (ir_def& d)
  {
    get_timeline (d.get_var ()).replace_cache (d);
  }

  void
  ir_basic_block::track_phi_def (ir_def& d)
  {
    get_timeline (d.get_var ()).track_phi (d);
  }

  void
  ir_basic_block::def_emplace_before (const instr_citer pos, ir_def& d)
  {
    def_timeline& vt = get_timeline (d.get_var ());
    if (pos == body_end ())
      return vt.emplace_back (d);
    vt.emplace_before (find_latest_def_before (pos, vt).base (), d);
  }

  void
  ir_basic_block::def_emplace_front (ir_def& d)
  {
    get_timeline (d.get_var ()).emplace_front (d);
  }

  void
  ir_basic_block::def_emplace_back (ir_def& d)
  {
    get_timeline (d.get_var ()).emplace_back (d);
  }

  void
  ir_basic_block::def_emplace_before (instr_citer pos, ir_def_instruction& d)
  {
    return def_emplace_before (pos, d.get_def ());
  }

  void
  ir_basic_block::def_emplace_front (ir_def_instruction& d)
  {
    return def_emplace_front (d.get_def ());
  }

  void
  ir_basic_block::def_emplace_back (ir_def_instruction& d)
  {
    return def_emplace_back (d.get_def ());
  }

  gch::optional_ref<ir_def>
  ir_basic_block::join_preceding_defs (ir_variable& var)
  {
    std::size_t npreds = num_preds ();
    if (npreds == 0)
      return nullopt;
    if (npreds == 1)
      return var.join (*pred_front ());

    ir_variable::block_def_vect pairs;
    std::vector<nonnull_ptr<ir_basic_block>> undef_blocks;
    std::for_each (pred_begin (), pred_end (),
                   [&var, &pairs, &undef_blocks] (nonnull_ptr<ir_basic_block> pred)
                    {
                      if (optional_ref<ir_def> opt_def = var.join (*pred))
                        pairs.emplace_back (pred, *opt_def);
                      else
                        undef_blocks.emplace_back (pred);
                    });

    // TODO if we have null returns here we need to create extra diversion
    //  blocks for those code paths. Otherwise the code will crash.

    auto cmp_def = std::get<nonnull_ptr<ir_def>> (pairs.front ());
    if (std::any_of (pairs.begin (), pairs.end (),
          [cmp_def] (auto&& p) { return std::get<nonnull_ptr<ir_def>> (p) != cmp_def; }))
    {
      ir_type common_ty = ir_variable::normalize_types (pairs);
      return create_phi (var, common_ty, pairs).get_def ();
    }
    return *cmp_def;
  }

  ir_component::link_iter
  ir_basic_block::pred_begin (void)
  {
    return m_parent.pred_begin (*this);
  }

  ir_component::link_iter
  ir_basic_block::pred_end (void)
  {
    return m_parent.pred_end (*this);
  }

  std::size_t
  ir_basic_block::num_preds (void)
  {
    return m_parent.num_preds (*this);
  }

  ir_component::link_iter
  ir_basic_block::succ_begin (void)
  {
    return m_parent.succ_begin (*this);
  }

  ir_component::link_iter
  ir_basic_block::succ_end (void)
  {
    return m_parent.succ_end (*this);
  }

  std::size_t
  ir_basic_block::num_succs (void)
  {
    return m_parent.num_succs (*this);
  }

  ir_function&
  ir_basic_block::get_function (void) noexcept
  {
    return m_parent.get_function ();
  }

  const ir_function&
  ir_basic_block::get_function (void) const noexcept
  {
    return m_parent.get_function ();
  }

  void
  ir_basic_block::reset (void) noexcept
  {
    erase (body_begin (), body_end ());
    
    // TODO remove when verified
    for (const auto& p : m_timeline_map)
    {

    }
  }

  constexpr ir_type::impl ir_type::instance<ir_basic_block *>::m_impl;

}

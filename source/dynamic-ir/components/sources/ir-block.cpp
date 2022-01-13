/** ir-block.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-block.hpp"
#include "ir-structure.hpp"
#include "ir-def-resolution.hpp"
#include "ir-all-subcomponent-visitors.hpp"

#include "ir-iterator.hpp"

#include <gch/select-iterator.hpp>

#include <algorithm>
#include <utility>
#include <list>

namespace gch
{

  ir_block::
  ~ir_block (void) = default;

  ir_block::
  ir_block (ir_structure& parent)
    : ir_subcomponent (parent)
  { }

  ir_block::
  ir_block (ir_structure& parent, ir_block&& other) noexcept
    : ir_subcomponent     (parent),
      m_instr_partition   (std::move (other.m_instr_partition)),
      m_def_timelines_map (std::move (other.m_def_timelines_map))
  {
    std::for_each (selected<ir_def_timeline> (m_def_timelines_map.begin ()),
                   selected<ir_def_timeline> (m_def_timelines_map.end ()),
                   [this](ir_def_timeline& dt) { dt.set_block (*this); });
  }

  ir_block&
  ir_block::
  operator= (ir_block&& other) noexcept
  {
    m_instr_partition   = std::move (other.m_instr_partition);
    m_def_timelines_map = std::move (other.m_def_timelines_map);

    std::for_each (selected<ir_def_timeline> (m_def_timelines_map.begin ()),
                   selected<ir_def_timeline> (m_def_timelines_map.end ()),
                   [this](ir_def_timeline& dt) { dt.set_block (*this); });

    return *this;
  }

  ir_def_timeline::ut_riter
  ir_block::
  find_latest_use_timeline_before (const citer pos, ir_def_timeline& dt,
                                   ir_def_timeline::ut_riter start) const
  {
    for (; start != dt.local_rend (); ++start)
    {
      auto       subrange_riter = dt.instructions_rbegin (start);
      const auto subrange_rend  = dt.instructions_rend (start);

      for (; subrange_riter != subrange_rend; ++subrange_riter)
      {
        if (subrange_riter.base () == pos)
          return start;
      }
    }
    return start;
  }

  ir_def_timeline::ut_riter
  ir_block::
  find_latest_use_timeline_before (const citer pos, ir_def_timeline& dt) const
  {
    if (pos == end<range::body> ())
      return dt.use_timelines_rbegin ();

    if (pos == cbegin<range::body> ())
      return dt.local_rend ();

    return find_latest_use_timeline_before (pos, dt, dt.local_rbegin ());
  }

  ir_def_timeline::use_timelines_iterator
  ir_block::
  get_latest_use_timeline (ir_def_timeline& dt)
  {
    if (dt.has_outgoing_timeline ())
      return dt.get_outgoing_timeline_iter ();

    // else we need to join
    join_at (dt);
    assert (dt.has_incoming_timeline ());
    return dt.get_incoming_timeline_iter ();
  }

  ir_use_timeline&
  ir_block::
  get_latest_use_timeline (ir_variable& var)
  {
    return *get_latest_use_timeline (get_def_timeline (var));
  }

  ir_def_timeline::use_timelines_iterator
  ir_block::
  get_latest_use_timeline_before (const citer pos, ir_def_timeline& dt,
                                  ir_def_timeline::ut_riter start)
  {
    if (pos == end<range::body> ())
      return get_latest_use_timeline (dt);

    auto found = find_latest_use_timeline_before (pos, dt, start);
    if (found != dt.use_timelines_rend ())
      return std::prev (found.base ());

    // else we need to join
    join_at (dt);
    assert (dt.has_incoming_timeline ());
    return dt.get_incoming_timeline_iter ();
  }

  ir_def_timeline::use_timelines_iterator
  ir_block::
  get_latest_use_timeline_before (const citer pos, ir_def_timeline& dt)
  {
    return get_latest_use_timeline_before (pos, dt, dt.local_rbegin ());
  }

  ir_use_timeline&
  ir_block::
  get_latest_use_timeline_before (const citer pos, ir_variable& var)
  {
    return *get_latest_use_timeline_before (pos, get_def_timeline (var));
  }

  ir_use_info
  ir_block::
  prepare_operand (citer pos, ir_variable& var)
  {
    ir_def_timeline& dt           = get_def_timeline (var);
    auto             ut_it        = get_latest_use_timeline (dt);
    auto             use_position = dt.first_use_after (pos, ut_it);

    return { *ut_it, use_position };
  }

  const ir_constant&
  ir_block::
  prepare_operand (citer, const ir_constant& c) noexcept
  {
    return c;
  }

  ir_constant&&
  ir_block::
  prepare_operand (citer, ir_constant&& t) noexcept
  {
    return std::move (t);
  }

  ir_use_timeline&
  ir_block::
  track_def_at (iter pos, ir_variable& var)
  {
    ir_def_timeline& dt     = get_def_timeline (var);
    auto             ut_pos = find_latest_use_timeline_before (pos, dt).base ();

    return *dt.emplace_local (ut_pos, pos);
  }

  ir_block::iter
  ir_block::
  remove_instruction (const citer pos)
  {
    const ir_instruction& instr = *pos;
    assert (! is_a<ir_opcode::phi> (instr) && "cannot remove phi instruction");

    if (has_def (instr))
    {
      // do some stuff to relink the dependent uses

      assert (has_def_timeline (instr.get_def ().get_variable ()));
      auto& dt = std::get<ir_def_timeline> (*find_def_timeline (pos->get_def ().get_variable ()));

      auto ut_rit = std::find_if (dt.local_rbegin (), dt.local_rend (),
                                  [&](const ir_use_timeline& ut)
                                  {
                                    return ut.get_def_pos () == pos;
                                  });

      assert (ut_rit != dt.local_rend () && "could not find the use-timeline for the instruction");

      if (std::next (ut_rit) == dt.use_timelines_rend ())
      {
        // then local_rend == use_timelines_rend => no incoming-timeline (need to join)

        join_at (dt);
        assert (dt.has_incoming_timeline ());

        ir_use_timeline& incoming_tl = dt.get_incoming_timeline ();
        incoming_tl.splice_back (*ut_rit);
      }
      else
        std::next (ut_rit)->splice_back (*ut_rit);

      dt.erase_local (std::next (ut_rit).base ());
    }

    return erase<range::body> (pos);
  }

  ir_block::iter
  ir_block::
  remove_range (const citer first, const citer last) noexcept
  {
    ir_link_set<const ir_variable> unlinked_vars;
    std::for_each (criter { last }, criter { first },
                   [&](const ir_instruction& instr)
                   {
                     if (! has_def (instr))
                       return;

                     const ir_def&      def = instr.get_def ();
                     const ir_variable& var = def.get_variable ();

                     if (! unlinked_vars.contains (var))
                     {
                       assert (has_def_timeline (var));
                       auto& dt = std::get<ir_def_timeline> (*find_def_timeline (var));

                       auto ut_rit = std::find_if (dt.local_rbegin (),
                                                   dt.local_rend (),
                                                   [&](const ir_use_timeline& tl)
                                                   {
                                                     return &instr == &tl.get_def_instruction ();
                                                   });

                       assert (ut_rit != dt.local_rend ()
                           &&  "could not find the use-timeline for the instruction");

                       if (std::next (ut_rit) == dt.use_timelines_rend ())
                       {
                         // then local_rend == use_timelines_rend
                         //   => no incoming-timeline (need to join)

                         join_at (dt);
                         assert (dt.has_incoming_timeline ());

                         ir_use_timeline& incoming_tl = dt.get_incoming_timeline ();
                         incoming_tl.splice_back (*ut_rit);
                       }
                       else
                       {
                         auto& ut = *get_latest_use_timeline_before (first, dt, std::next (ut_rit));
                         ut.splice_back (*ut_rit);
                       }

                       dt.erase_local (std::next (ut_rit).base ());

                       unlinked_vars.emplace (var);
                     }
                   });

    return erase<range::body> (first, last);
  }

  auto
  ir_block::
  create_phi (ir_variable& var)
    -> iter
  {
    emplace_back<range::phi, ir_opcode::phi> (var);
    return std::prev (end<range::phi> ());
  }

  auto
  ir_block::
  erase_phi (citer pos)
    -> iter
  {
    return get<range::phi> ().erase (pos);
  }

  auto
  ir_block::
  erase_phi (ir_variable& var)
    -> iter
  {
    auto pos = std::find_if (begin<range::phi> (), end<range::phi> (),
                             [&var] (const ir_instruction& instr)
                             {
                               return &instr.get_def ().get_variable () == &var;
                             });

    assert (pos != end<range::phi> ());
    return erase_phi (pos);
  }

  auto
  ir_block::
  dt_map_begin (void) noexcept
    -> dt_map_iter
  {
    return m_def_timelines_map.begin ();
  }

  auto
  ir_block::
  dt_map_begin (void) const noexcept
    -> dt_map_citer
  {
    return as_mutable (*this).dt_map_begin ();
  }

  auto
  ir_block::
  dt_map_cbegin (void) const noexcept
    -> dt_map_citer
  {
    return dt_map_begin ();
  }

  auto
  ir_block::
  dt_map_end (void) noexcept
    -> dt_map_iter
  {
    return m_def_timelines_map.end ();
  }

  auto
  ir_block::
  dt_map_end (void) const noexcept
    -> dt_map_citer
  {
    return as_mutable (*this).dt_map_end ();
  }

  auto
  ir_block::
  dt_map_cend (void) const noexcept
    -> dt_map_citer
  {
    return dt_map_end ();
  }

  auto
  ir_block::
  dt_map_size (void) const noexcept
    -> dt_map_size_ty
  {
   return m_def_timelines_map.size ();
  }

  bool
  ir_block::
  dt_map_empty (void) const noexcept
  {
    return m_def_timelines_map.empty ();
  }

  auto
  ir_block::
  find_def_timeline (const ir_variable& var)
    -> dt_map_iter
  {
    return m_def_timelines_map.find (nonnull_ptr { var });
  }

  auto
  ir_block::
  find_def_timeline (const ir_variable& var) const
  -> dt_map_citer
  {
    return as_mutable (*this).find_def_timeline (var);
  }

  bool
  ir_block::
  has_def_timeline (const ir_variable& var) const
  {
    return find_def_timeline (var) != dt_map_end ();
  }

  auto
  ir_block::
  try_emplace_def_timeline (ir_variable& var)
    -> dt_map_emplace_return_type
  {
    auto res = m_def_timelines_map.try_emplace (nonnull_ptr { var }, *this, var);
    return { std::get<dt_map_iter> (res), std::get<bool> (res) };
  }

  ir_def_timeline&
  ir_block::
  get_def_timeline (ir_variable& var)
  {
    return std::get<ir_def_timeline> (*try_emplace_def_timeline (var).position);
  }

  optional_ref<ir_def_timeline>
  ir_block::
  maybe_get_def_timeline (const ir_variable& var)
  {
    if (auto cit = find_def_timeline (var); cit != m_def_timelines_map.end ())
      return optional_ref { std::get<ir_def_timeline> (*cit) };
    return nullopt;
  }

  optional_ref<const ir_def_timeline>
  ir_block::
  maybe_get_def_timeline (const ir_variable& var) const
  {
    return as_mutable (*this).maybe_get_def_timeline (var);
  }

  auto
  ir_block::
  remove_def_timeline (dt_map_iter it)
    -> dt_map_iter
  {
    return m_def_timelines_map.erase (it);
  }

  auto
  ir_block::
  remove_def_timeline (dt_map_citer cit)
    -> dt_map_iter
  {
    return m_def_timelines_map.erase (cit);
  }

  bool
  ir_block::
  remove_def_timeline (const ir_variable& var)
  {
    return m_def_timelines_map.erase (nonnull_ptr { var }) != 0;
  }

}

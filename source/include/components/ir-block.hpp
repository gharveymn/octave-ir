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


#ifndef OCTAVE_IR_IR_BLOCK_HPP
#define OCTAVE_IR_IR_BLOCK_HPP

#include "components/linkage/ir-def-timeline.hpp"
#include "components/ir-block-common.hpp"
#include "components/ir-component.hpp"

#include "utilities/ir-allocator-util.hpp"
#include "utilities/ir-common-util.hpp"

#include "values/types/ir-type.hpp"
#include "values/ir-instruction.hpp"
#include "values/ir-instruction-fwd.hpp"
#include "values/ir-constant.hpp"

#include <gch/tracker.hpp>
#include <gch/optional_ref.hpp>
#include <gch/partition/list_partition.hpp>
#include <gch/small_vector.hpp>

#include <plf_list.h>

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <stack>
#include <vector>
#include <list>
#include <map>

namespace gch
{

  class ir_block;
  class ir_structure;
  class ir_variable;
  class ir_def;
  class ir_use;

  class ir_block
    : public ir_subcomponent,
      public visitable<ir_block, ir_component_visitors>
  {
  public:
    using container_type          = ir_instruction_container;
    using value_type              = container_type::value_type;
    using allocator_type          = container_type::allocator_type;
    using size_type               = container_type::size_type;
    using difference_type         = container_type::difference_type;
    using reference               = container_type::reference;
    using const_reference         = container_type::const_reference;
    using pointer                 = container_type::pointer;
    using const_pointer           = container_type::const_pointer;

    using iterator                = container_type::iterator;
    using const_iterator          = container_type::const_iterator;
    using reverse_iterator        = container_type::reverse_iterator;
    using const_reverse_iterator  = container_type::const_reverse_iterator;

    using val_t   = value_type;
    using alloc_t = allocator_type;
    using size_ty = size_type;
    using diff_ty = difference_type;
    using ref     = reference;
    using cref    = const_reference;
    using ptr     = pointer;
    using cptr    = const_pointer;

    using iter    = iterator;
    using citer   = const_iterator;
    using riter   = reverse_iterator;
    using criter  = const_reverse_iterator;

    using instruction_container = as_list_partition_t<container_type, 2>;

    using dt_map_type            = std::unordered_map<nonnull_cptr<ir_variable>, ir_def_timeline>;
    using dt_map_value_type      = typename dt_map_type::value_type;
    using dt_map_allocator_type  = typename dt_map_type::allocator_type;
    using dt_map_size_type       = typename dt_map_type::size_type;
    using dt_map_difference_type = typename dt_map_type::difference_type;
    using dt_map_reference       = typename dt_map_type::reference;
    using dt_map_const_reference = typename dt_map_type::const_reference;
    using dt_map_pointer         = typename dt_map_type::pointer;
    using dt_map_const_pointer   = typename dt_map_type::const_pointer;

    using dt_map_iterator        = typename dt_map_type::iterator;
    using dt_map_const_iterator  = typename dt_map_type::const_iterator;

    using dt_map_val_t   = dt_map_value_type;
    using dt_map_alloc_t = dt_map_allocator_type;
    using dt_map_size_ty = dt_map_size_type;
    using dt_map_diff_ty = dt_map_difference_type;
    using dt_map_ref     = dt_map_reference;
    using dt_map_cref    = dt_map_const_reference;
    using dt_map_ptr     = dt_map_pointer;
    using dt_map_cptr    = dt_map_const_pointer;

    using dt_map_iter    = dt_map_iterator;
    using dt_map_citer   = dt_map_const_iterator;

    struct dt_map_emplace_return_type
    {
      dt_map_iterator position;
      bool            inserted;
    };

    ir_block            (void)                   = delete;
    ir_block            (const ir_block&)        = delete;
    ir_block            (ir_block&&) noexcept    = delete;
    ir_block& operator= (const ir_block&)        = delete;
    ir_block& operator= (ir_block&&) noexcept;
    ~ir_block           (void) noexcept override = default;

    explicit
    ir_block (ir_structure& parent);
    ir_block (ir_structure& parent, ir_block&& other) noexcept;

    using range = ir_instruction_range;

    /* clang-format off */
    // all
    [[nodiscard]]
    auto
    get_data_view (void) const noexcept
    {
      return m_instr_partition.data_view ();
    }

    template <range R>
    [[nodiscard]]
    auto&
    get (void) noexcept
    {
      return get_subrange<R> (m_instr_partition);
    }

    template <range R>
    [[nodiscard]]
    const auto&
    get (void) const noexcept
    {
      return get_subrange<R> (m_instr_partition);
    }

    template <range R>
    [[nodiscard]]
    iterator
    begin (void) noexcept
    {
      return get<R> ().begin ();
    }

    template <range R>
    [[nodiscard]]
    const_iterator
    begin (void) const noexcept
    {
      return as_mutable (*this).begin<R> ();
    }

    template <range R>
    [[nodiscard]]
    const_iterator
    cbegin (void) const noexcept
    {
      return begin<R> ();
    }

    template <range R>
    [[nodiscard]]
    iterator
    end (void) noexcept
    {
      return get<R> ().end ();
    }

    template <range R>
    [[nodiscard]]
    const_iterator
    end (void) const noexcept
    {
      return as_mutable (*this).end<R> ();
    }

    template <range R>
    [[nodiscard]]
    const_iterator
    cend (void) const noexcept
    {
      return end<R> ();
    }

    template <range R>
    [[nodiscard]]
    reverse_iterator
    rbegin (void) noexcept
    {
      return get<R> ().rbegin ();
    }

    template <range R>
    [[nodiscard]]
    const_reverse_iterator
    rbegin (void) const noexcept
    {
      return as_mutable (*this).rbegin<R> ();
    }

    template <range R>
    [[nodiscard]]
    const_reverse_iterator
    crbegin (void) const noexcept
    {
      return rbegin<R> ();
    }

    template <range R>
    [[nodiscard]]
    reverse_iterator
    rend (void) noexcept
    {
      return get<R> ().rend ();
    }

    template <range R>
    [[nodiscard]]
    const_reverse_iterator
    rend (void) const noexcept
    {
      return as_mutable (*this).rend<R> ();
    }

    template <range R>
    [[nodiscard]]
    const_reverse_iterator
    crend (void) const noexcept
    {
      return rend<R> ();
    }

    template <range R>
    [[nodiscard]]
    reference
    front (void)
    {
      return get<R> ().front ();
    }

    template <range R>
    [[nodiscard]]
    const_reference
    front (void) const
    {
      return as_mutable (*this).front<R> ();
    }

    template <range R>
    [[nodiscard]]
    reference
    back (void)
    {
      return get<R> ().back ();
    }

    template <range R>
    [[nodiscard]]
    const_reference
    back (void) const
    {
      return as_mutable (*this).back<R> ();
    }

    template <range R>
    [[nodiscard]]
    size_type
    size (void) const noexcept
    {
     return get<R> ().size ();
    }

    template <range R>
    [[nodiscard]]
    bool
    empty (void) const noexcept
    {
      return get<R> ().empty ();
    }

  private:
    template <range R, ir_opcode Op, typename ...Args>
    decltype (auto)
    emplace_front (Args&&... args)
    {
      return get<R> ().emplace_front (ir_instruction::tag<Op>, std::forward<Args> (args)...);
    }

    template <range R, ir_opcode Op, typename ...Args>
    decltype (auto)
    emplace (citer pos, Args&&... args)
    {
      return get<R> ().emplace (pos, ir_instruction::tag<Op>, std::forward<Args> (args)...);
    }

    template <range R, ir_opcode Op, typename ...Args>
    decltype (auto)
    emplace_back (Args&&... args)
    {
      return get<R> ().emplace_back (ir_instruction::tag<Op>, std::forward<Args> (args)...);
    }

    template <range R, typename ...Args>
    decltype (auto)
    splice (citer pos, ir_block& other, Args&&... args)
    {
      return get<R> ().splice (pos, other.get<R> (), std::forward<Args> (args)...);
    }

    template <range R>
    decltype (auto)
    pop_front (void)
    {
      return get<R> ().pop_front ();
    }

    template <range R>
    decltype (auto)
    pop_back (void)
    {
      return get<R> ().pop_back ();
    }

    template <range R, typename ...Args>
    decltype (auto)
    erase (Args&&... args)
    {
      return get<R> ().erase (std::forward<Args> (args)...);
    }

    ir_use_timeline&
    join_incoming (ir_def_timeline& dt);

    optional_ref<ir_def_timeline>
    maybe_join_incoming (ir_variable& var);

    std::vector<nonnull_ptr<ir_def_timeline>>
    collect_defs_incoming (ir_variable& var);

    std::vector<nonnull_ptr<ir_def_timeline>>
    forward_incoming_timelines (ir_variable& var);

    std::vector<nonnull_ptr<ir_def_timeline>>
    forward_outgoing_timelines (ir_variable& var);

    ir_def_timeline&
    append_incoming (ir_variable& var, ir_def_timeline& dt, ir_block& incoming_block,
                     ir_def_timeline& pred);

    ir_def_timeline&
    append_incoming (ir_variable& var, ir_def_timeline& dt, ir_block& incoming_block,
                     const std::vector<nonnull_ptr<ir_def_timeline>>& preds);

    ir_def_timeline&
    resolve_undefined_incoming (ir_variable& undef_var, ir_def_timeline& var_dt);

    ir_def_timeline&
    resolve_undefined_outgoing (ir_variable& undef_var, ir_def_timeline& var_dt);

    void
    propagate_def_timeline (ir_variable& var, ir_block& incoming_block,
                            ir_def_timeline& remote);

    ir_def_timeline&
    set_undefined_state (ir_variable& undef_var, bool state);

    ir_use_info
    prepare_operand (citer pos, ir_variable& var);

    static
    const ir_constant&
    prepare_operand (citer pos, const ir_constant& c) noexcept;

    static
    ir_constant&&
    prepare_operand (citer pos, ir_constant&& t) noexcept;

    ir_use_timeline&
    track_def_at (iter pos, ir_variable& var);

  public:
    iter
    create_phi (ir_variable& var);

    // unsafe
    iter
    erase_phi (ir_instruction_citer pos);

    iter
    erase_phi (ir_variable& var);

    [[nodiscard]]
    ir_def_timeline::use_timelines_reverse_iterator
    find_latest_use_timeline_before (const_iterator pos, ir_def_timeline& dt,
                                     ir_def_timeline::use_timelines_reverse_iterator start) const;

    [[nodiscard]]
    ir_def_timeline::use_timelines_reverse_iterator
    find_latest_use_timeline_before (const_iterator pos, ir_def_timeline& dt) const;

    [[nodiscard]]
    ir_def_timeline::use_timelines_iterator
    get_latest_use_timeline (ir_def_timeline& dt);

    [[nodiscard]]
    ir_use_timeline&
    get_latest_use_timeline (ir_variable& var);

    [[nodiscard]]
    ir_def_timeline::use_timelines_iterator
    get_latest_use_timeline_before (const_iterator pos, ir_def_timeline& dt,
                                    ir_def_timeline::use_timelines_reverse_iterator start);

    [[nodiscard]]
    ir_def_timeline::use_timelines_iterator
    get_latest_use_timeline_before (const_iterator pos, ir_def_timeline& dt);

    [[nodiscard]]
    ir_use_timeline&
    get_latest_use_timeline_before (const_iterator pos, ir_variable& var);

    iter
    remove_instruction (citer pos);

    iter
    remove_range (citer first, citer last) noexcept;

    ir_use_timeline
    split_uses (ir_use_timeline& src, citer pivot, citer last);

    ir_use_timeline::iter
    first_use_after (ir_use_timeline& tl, citer pos, citer last);

    [[nodiscard]]
    dt_map_iterator
    dt_map_begin (void) noexcept;

    [[nodiscard]]
    dt_map_const_iterator
    dt_map_begin (void) const noexcept;

    [[nodiscard]]
    dt_map_const_iterator
    dt_map_cbegin (void) const noexcept;

    [[nodiscard]]
    dt_map_iterator
    dt_map_end (void) noexcept;

    [[nodiscard]]
    dt_map_const_iterator
    dt_map_end (void) const noexcept;

    [[nodiscard]]
    dt_map_const_iterator
    dt_map_cend (void) const noexcept;

    [[nodiscard]]
    dt_map_size_type
    dt_map_size (void) const noexcept;

    [[nodiscard]]
    bool
    dt_map_empty (void) const noexcept;

    dt_map_iterator
    find_def_timeline (const ir_variable& var);

    dt_map_const_iterator
    find_def_timeline (const ir_variable& var) const;

    [[nodiscard]]
    bool
    has_def_timeline (const ir_variable& var) const;

    dt_map_emplace_return_type
    try_emplace_def_timeline (ir_variable& var);

    ir_def_timeline&
    get_def_timeline (ir_variable& var);

    optional_ref<ir_def_timeline>
    maybe_get_def_timeline (const ir_variable& var);

    optional_ref<const ir_def_timeline>
    maybe_get_def_timeline (const ir_variable& var) const;

    dt_map_iterator
    remove_def_timeline (dt_map_iterator it);

    dt_map_iterator
    remove_def_timeline (dt_map_const_iterator it);

    bool
    remove_def_timeline (const ir_variable& var);

    //
    // templated functions
    //

    // with return (places instruction immediately before `pos`)
    template <ir_opcode Op, typename ...Args,
              std::enable_if_t<ir_instruction_traits<Op>::has_def> * = nullptr>
    ir_instruction&
    emplace_instruction (const citer pos, ir_variable& v, Args&&... args)
    {
      iter it = emplace<range::body, Op> (v, prepare_operand (pos, std::forward<Args> (args))...);
      try
      {
        track_def_at (it, v);
      }
      catch (...)
      {
        erase<range::body> (it);
        throw;
      }
      return *it;
    }

    // no return (places instruction immediately before `pos`)
    template <ir_opcode Op, typename ...Args,
              std::enable_if_t<! ir_instruction_traits<Op>::has_def> * = nullptr>
    ir_instruction&
    emplace_instruction (const citer pos, Args&&... args)
    {
      return *emplace<range::body, Op> (pos, prepare_operand (pos, std::forward<Args> (args))...);
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction&
    append_instruction (ir_variable& v, Args&&... args)
    {
      return emplace_instruction<Op> (end<range::body> (), std::forward<Args> (args)...);
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction&
    prepend_instruction (ir_variable& v, Args&&... args)
    {
      return emplace_instruction<Op> (begin<range::body> (), std::forward<Args> (args)...);
    }

  private:
    instruction_container m_instr_partition;
    dt_map_type           m_def_timelines_map;
  };

  class ir_condition_block
    : public ir_block
  {
  public:
    using ir_block::ir_block;
  };

}

#endif

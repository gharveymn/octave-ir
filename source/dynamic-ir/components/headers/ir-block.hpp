/** ir-block.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifndef OCTAVE_IR_DYNAMIC_IR_IR_BLOCK_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_BLOCK_HPP

#include "linkage/ir-def-timeline.hpp"
#include "ir-block-common.hpp"
#include "ir-component.hpp"

#include "ir-allocator-util.hpp"
#include "ir-utility.hpp"

#include "ir-type.hpp"
#include "ir-instruction.hpp"
#include "ir-instruction-fwd.hpp"
#include "ir-constant.hpp"
#include "ir-use-timeline.hpp"

#include "subcomponent/ir-subcomponent-visitors-fwd.hpp"

#include <gch/tracker/tracker.hpp>
#include <gch/optional_ref.hpp>
#include <gch/partition/list_partition.hpp>
#include <gch/small_vector.hpp>

#include <unordered_map>

namespace gch
{

  class ir_block;
  class ir_structure;
  class ir_variable;
  class ir_def;
  class ir_use;

  class ir_block final
    : public ir_subcomponent,
      public visitable<ir_block, consolidated_visitors_t<ir_subcomponent>>
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
    ~ir_block           (void) override;

    ir_block (ir_structure& parent, std::string_view name = "");

    ir_block (ir_structure& parent, ir_variable& condition_var, std::string_view name = "");

    ir_block (ir_structure& parent, ir_block&& other) noexcept;

    using range = ir_instruction_range;

    [[nodiscard]]
    auto
    get_data_view (void) const noexcept
    {
      return m_instr_partition.get_data_view ();
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

    template <range R = range::all>
    [[nodiscard]]
    iterator
    begin (void) noexcept
    {
      return get<R> ().begin ();
    }

    template <range R = range::all>
    [[nodiscard]]
    const_iterator
    begin (void) const noexcept
    {
      return as_mutable (*this).begin<R> ();
    }

    template <range R = range::all>
    [[nodiscard]]
    const_iterator
    cbegin (void) const noexcept
    {
      return begin<R> ();
    }

    template <range R = range::all>
    [[nodiscard]]
    iterator
    end (void) noexcept
    {
      return get<R> ().end ();
    }

    template <range R = range::all>
    [[nodiscard]]
    const_iterator
    end (void) const noexcept
    {
      return as_mutable (*this).end<R> ();
    }

    template <range R = range::all>
    [[nodiscard]]
    const_iterator
    cend (void) const noexcept
    {
      return end<R> ();
    }

    template <range R = range::all>
    [[nodiscard]]
    reverse_iterator
    rbegin (void) noexcept
    {
      return get<R> ().rbegin ();
    }

    template <range R = range::all>
    [[nodiscard]]
    const_reverse_iterator
    rbegin (void) const noexcept
    {
      return as_mutable (*this).rbegin<R> ();
    }

    template <range R = range::all>
    [[nodiscard]]
    const_reverse_iterator
    crbegin (void) const noexcept
    {
      return rbegin<R> ();
    }

    template <range R = range::all>
    [[nodiscard]]
    reverse_iterator
    rend (void) noexcept
    {
      return get<R> ().rend ();
    }

    template <range R = range::all>
    [[nodiscard]]
    const_reverse_iterator
    rend (void) const noexcept
    {
      return as_mutable (*this).rend<R> ();
    }

    template <range R = range::all>
    [[nodiscard]]
    const_reverse_iterator
    crend (void) const noexcept
    {
      return rend<R> ();
    }

    template <range R = range::all>
    [[nodiscard]]
    reference
    front (void)
    {
      return get<R> ().front ();
    }

    template <range R = range::all>
    [[nodiscard]]
    const_reference
    front (void) const
    {
      return as_mutable (*this).front<R> ();
    }

    template <range R = range::all>
    [[nodiscard]]
    reference
    back (void)
    {
      return get<R> ().back ();
    }

    template <range R = range::all>
    [[nodiscard]]
    const_reference
    back (void) const
    {
      return as_mutable (*this).back<R> ();
    }

    template <range R = range::all>
    [[nodiscard]]
    size_type
    size (void) const noexcept
    {
     return get<R> ().size ();
    }

    template <range R = range::all>
    [[nodiscard]]
    bool
    empty (void) const noexcept
    {
      return get<R> ().empty ();
    }

    friend class ir_def_timeline;

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
    prepare_operand (citer pos, ir_constant&& c) noexcept;

    template <typename T>
    static
    ir_constant
    prepare_operand (citer, T&& t) noexcept
    {
      return ir_constant (std::forward<T> (t));
    }

    ir_use_timeline&
    track_def_at (iter pos, ir_variable& var);

    iter
    create_phi (ir_variable& var);

    // unsafe
    iter
    erase_phi (ir_instruction_citer pos);

    iter
    erase_phi (ir_variable& var);

  public:
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

    dt_map_emplace_return_type
    try_emplace_def_timeline (ir_variable& var);

    [[nodiscard]]
    bool
    has_def_timeline (const ir_variable& var) const;

    [[nodiscard]]
    ir_def_timeline&
    get_def_timeline (ir_variable& var);

    [[nodiscard]]
    const ir_def_timeline&
    get_def_timeline (const ir_variable& var) const;

    [[nodiscard]]
    optional_ref<ir_def_timeline>
    maybe_get_def_timeline (const ir_variable& var);

    [[nodiscard]]
    optional_cref<ir_def_timeline>
    maybe_get_def_timeline (const ir_variable& var) const;

    dt_map_iterator
    remove_def_timeline (dt_map_iterator it);

    dt_map_iterator
    remove_def_timeline (dt_map_const_iterator it);

    bool
    remove_def_timeline (const ir_variable& var);

    void
    set_condition_variable (ir_variable& var);

    bool
    has_condition_variable (void) const noexcept;

    ir_variable&
    get_condition_variable (void) noexcept;

    const ir_variable&
    get_condition_variable (void) const noexcept;

    optional_ref<ir_variable>
    maybe_get_condition_variable (void) noexcept;

    optional_cref<ir_variable>
    maybe_get_condition_variable (void) const noexcept;

    //
    // templated functions
    //

    // with return (places instruction immediately before `pos`)
    template <ir_opcode Op, typename ...Args,
              std::enable_if_t<ir_instruction_traits<Op>::has_def> * = nullptr>
    ir_instruction&
    emplace_instruction_with_def (const citer pos, ir_variable& var, Args&&... args)
    {
      static_assert (Op != ir_opcode::call
                 ||  std::is_same_v<remove_cvref_t<pack_front_t<type_pack<Args...>>>,
                                    ir_external_function_info>,
                     "The first argument to a call instruction must be ir_external_function_info");

      iter it = emplace<range::body, Op> (
        pos,
        var,
        prepare_operand (pos, std::forward<Args> (args))...);

      try
      {
        track_def_at (it, var);
      }
      catch (...)
      {
        erase<range::body> (it);
        throw;
      }
      return *it;
    }

    // no return (places instruction immediately before `pos`)
    template <ir_opcode Op, typename ...Args>
    ir_instruction&
    emplace_instruction (const citer pos, Args&&... args)
    {
      static_assert (Op != ir_opcode::call
                 ||  std::is_same_v<remove_cvref_t<pack_front_t<type_pack<Args...>>>,
                                    ir_external_function_info>,
                     "The first argument to a call instruction must be ir_external_function_info");

      return *emplace<range::body, Op> (pos, prepare_operand (pos, std::forward<Args> (args))...);
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction&
    append_instruction_with_def (ir_variable& var, Args&&... args)
    {
      return emplace_instruction_with_def<Op> (
        end<range::body> (),
        var,
        std::forward<Args> (args)...);
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction&
    append_instruction (Args&&... args)
    {
      return emplace_instruction<Op> (end<range::body> (), std::forward<Args> (args)...);
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction&
    prepend_instruction_with_def (ir_variable& var, Args&&... args)
    {
      return emplace_instruction_with_def<Op> (
        begin<range::body> (),
        var,
        std::forward<Args> (args)...);
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction&
    prepend_instruction (Args&&... args)
    {
      return emplace_instruction<Op> (begin<range::body> (), std::forward<Args> (args)...);
    }

    std::string_view
    get_name (void) const noexcept
    {
      return m_name;
    }

    void
    set_name (std::string_view name)
    {
      m_name = name;
    }

  private:
    instruction_container     m_instr_partition;
    dt_map_type               m_def_timelines_map;
    optional_ref<ir_variable> m_condition_var;
    std::string               m_name;
  };

  template <typename RandomIt,
            std::enable_if_t<std::is_convertible_v<
              typename std::iterator_traits<RandomIt>::value_type,
              ir_block::const_iterator>> * = nullptr>
  void
  relative_sort (const RandomIt first, RandomIt last,
                 const ir_block::const_iterator instr_first, ir_block::const_iterator instr_last)
  {
    while (first != last)
    {
      RandomIt found = last;
      while (instr_first != instr_last && found == last)
        found = std::find (first, last, --instr_last);
      std::iter_swap (found, --last);
    }
  }

}

#endif

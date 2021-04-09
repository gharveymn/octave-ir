/** ir-def-timeline.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_DEF_TIMELINE_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_DEF_TIMELINE_HPP

#include "components/linkage/ir-incoming-node.hpp"
#include "values/ir-instruction-fwd.hpp"

#include <gch/optional_ref.hpp>
#include <gch/partition/list_partition.hpp>
#include <gch/tracker/tracker.hpp>

#include <unordered_map>

namespace gch
{

  class ir_def_resolution;
  class ir_use_timeline;
  class ir_variable;

  // On def-timeline architecture
  // A def-timeline contains a sequence of defs which were used in this block.
  //
  // There is a distinction between defs which were locally created and those
  // which were imported from a predecessor block. We refer to those as
  // 'incoming defs' with associated 'incoming blocks'.
  //
  // Locally created defs are trivial to organize, but the incoming defs are
  // troublesome because
  // 1. There may be multiple incoming blocks associated with a single incoming def
  // 2. The incoming block may not be the origin of the incoming def
  // 3. If we want to reassociate certain uses with a new def, we should be able to find
  //    the nearest def without traversing all the way to the origin.
  //
  // So for these reasons we use a lazily evaluated tree structure. The actual origin defs
  // are only resolved when we are doing codegen. Otherwise, incoming defs are abstract to
  // the uses in the block. The uses in the block are associated with a kind of virtual
  // def at the top of the block which doesn't actually do anything other than provide a
  // stub for them to associate to (basically an unresolved phi node).
  //
  // def_timeline:
  //   [incoming_node:
  //      incoming_block, [incoming_def_timeline, ...]
  //   , ...]

  class ir_def_timeline
    : public intrusive_tracker<ir_def_timeline, remote::intrusive_reporter<ir_incoming_node>>
  {
    using base = intrusive_tracker<ir_def_timeline, remote::intrusive_reporter<ir_incoming_node>>;

    enum class range : std::size_t
    {
      all      = partition_base_index,
      incoming = 0,
      local    = 1
    };

  public:
    /* use-timelines */

    using use_timelines_list                    = list_partition<ir_use_timeline, 2>;
    using use_timelines_value_type              = use_timelines_list::data_value_type;
    using use_timelines_allocator_type          = use_timelines_list::data_allocator_type;
    using use_timelines_size_type               = use_timelines_list::data_size_type;
    using use_timelines_difference_type         = use_timelines_list::data_difference_type;
    using use_timelines_reference               = use_timelines_list::data_reference;
    using use_timelines_const_reference         = use_timelines_list::data_const_reference;
    using use_timelines_pointer                 = use_timelines_list::data_pointer;
    using use_timelines_const_pointer           = use_timelines_list::data_const_pointer;

    using use_timelines_iterator                = use_timelines_list::data_iterator;
    using use_timelines_const_iterator          = use_timelines_list::data_const_iterator;
    using use_timelines_reverse_iterator        = use_timelines_list::data_reverse_iterator;
    using use_timelines_const_reverse_iterator  = use_timelines_list::data_const_reverse_iterator;

    using ut_val_t   = use_timelines_value_type;
    using ut_alloc_t = use_timelines_allocator_type;
    using ut_size_ty = use_timelines_size_type;
    using ut_diff_ty = use_timelines_difference_type;
    using ut_ref     = use_timelines_reference;
    using ut_cref    = use_timelines_const_reference;
    using ut_ptr     = use_timelines_pointer;
    using ut_cptr    = use_timelines_const_pointer;

    using ut_iter    = use_timelines_iterator;
    using ut_citer   = use_timelines_const_iterator;
    using ut_riter   = use_timelines_reverse_iterator;
    using ut_criter  = use_timelines_const_reverse_iterator;

    /* incoming map */

    using incoming_map_type = std::unordered_map<
      nonnull_cptr<ir_block>,
      ir_incoming_node,
      std::hash<nonnull_cptr<ir_block>>,
      std::equal_to<nonnull_cptr<ir_block>>,
      ir_incoming_node::element_allocator::rebind_for_map<nonnull_cptr<ir_block>>>;

    using incoming_value_type              = incoming_map_type::value_type;
    using incoming_allocator_type          = incoming_map_type::allocator_type;
    using incoming_size_type               = incoming_map_type::size_type;
    using incoming_difference_type         = incoming_map_type::difference_type;
    using incoming_reference               = incoming_map_type::reference;
    using incoming_const_reference         = incoming_map_type::const_reference;
    using incoming_pointer                 = incoming_map_type::pointer;
    using incoming_const_pointer           = incoming_map_type::const_pointer;

    using incoming_iterator                = incoming_map_type::iterator;
    using incoming_const_iterator          = incoming_map_type::const_iterator;

    using incoming_val_t   = incoming_value_type;
    using incoming_alloc_t = incoming_allocator_type;
    using incoming_size_ty = incoming_size_type;
    using incoming_diff_ty = incoming_difference_type;
    using incoming_ref     = incoming_reference;
    using incoming_cref    = incoming_const_reference;
    using incoming_ptr     = incoming_pointer;
    using incoming_cptr    = incoming_const_pointer;

    using incoming_iter    = incoming_iterator;
    using incoming_citer   = incoming_const_iterator;

    /* successors */

    using successors_tracker                 = base;
    using successors_value_type              = successors_tracker::value_type;
    using successors_allocator_type          = successors_tracker::allocator_type;
    using successors_size_type               = successors_tracker::size_type;
    using successors_difference_type         = successors_tracker::difference_type;
    using successors_reference               = successors_tracker::reference;
    using successors_const_reference         = successors_tracker::const_reference;
    using successors_pointer                 = successors_tracker::pointer;
    using successors_const_pointer           = successors_tracker::const_pointer;

    using successors_iterator                = successors_tracker::iterator;
    using successors_const_iterator          = successors_tracker::const_iterator;
    using successors_reverse_iterator        = successors_tracker::reverse_iterator;
    using successors_const_reverse_iterator  = successors_tracker::const_reverse_iterator;

    using succs_val_t   = successors_value_type;
    using succs_alloc_t = successors_allocator_type;
    using succs_size_ty = successors_size_type;
    using succs_diff_ty = successors_difference_type;
    using succs_ref     = successors_reference;
    using succs_cref    = successors_const_reference;
    using succs_ptr     = successors_pointer;
    using succs_cptr    = successors_const_pointer;

    using succs_iter    = successors_iterator;
    using succs_citer   = successors_const_iterator;
    using succs_riter   = successors_reverse_iterator;
    using succs_criter  = successors_const_reverse_iterator;

    ir_def_timeline (void)                                  = delete;
    ir_def_timeline (const ir_def_timeline&)                = delete;
    ir_def_timeline (ir_def_timeline&&) noexcept;
    ir_def_timeline& operator= (const ir_def_timeline&)     = delete;
    ir_def_timeline& operator= (ir_def_timeline&&) noexcept;
    ~ir_def_timeline (void)                                 = default;

    ir_def_timeline (ir_block& block, ir_variable& var) noexcept;
    ir_def_timeline (ir_block& block, ir_block& incoming_block, ir_def_timeline& pred);

    void
    set_block (ir_block& block) noexcept;

    [[nodiscard]]
    incoming_iterator
    incoming_begin (void) noexcept;

    [[nodiscard]]
    incoming_const_iterator
    incoming_begin (void) const noexcept;

    [[nodiscard]]
    incoming_const_iterator
    incoming_cbegin (void) const noexcept;

    [[nodiscard]]
    incoming_iterator
    incoming_end (void) noexcept;

    [[nodiscard]]
    incoming_const_iterator
    incoming_end (void) const noexcept;

    [[nodiscard]]
    incoming_const_iterator
    incoming_cend (void) const noexcept;

    [[nodiscard]]
    incoming_size_type
    num_incoming_blocks (void) const noexcept;

    [[nodiscard]]
    bool
    has_incoming_blocks (void) const noexcept;

    /* use-timelines */

  private:
    template <range R>
    [[nodiscard]]
    auto&
    get_use_timelines (void) noexcept
    {
      return get_subrange<R> (m_use_timelines);
    }

    template <range R>
    [[nodiscard]]
    const auto&
    get_use_timelines (void) const noexcept
    {
      return get_subrange<R> (m_use_timelines);
    }

  public:
    [[nodiscard]]
    use_timelines_iterator
    use_timelines_begin (void) noexcept;

    [[nodiscard]]
    use_timelines_const_iterator
    use_timelines_begin (void) const noexcept;

    [[nodiscard]]
    use_timelines_const_iterator
    use_timelines_cbegin (void) const noexcept;

    [[nodiscard]]
    use_timelines_iterator
    use_timelines_end (void) noexcept;

    [[nodiscard]]
    use_timelines_const_iterator
    use_timelines_end (void) const noexcept;

    [[nodiscard]]
    use_timelines_const_iterator
    use_timelines_cend (void) const noexcept;

    [[nodiscard]]
    use_timelines_reverse_iterator
    use_timelines_rbegin (void) noexcept;

    [[nodiscard]]
    use_timelines_const_reverse_iterator
    use_timelines_rbegin (void) const noexcept;

    [[nodiscard]]
    use_timelines_const_reverse_iterator
    use_timelines_crbegin (void) const noexcept;

    [[nodiscard]]
    use_timelines_reverse_iterator
    use_timelines_rend (void) noexcept;

    [[nodiscard]]
    use_timelines_const_reverse_iterator
    use_timelines_rend (void) const noexcept;

    [[nodiscard]]
    use_timelines_const_reverse_iterator
    use_timelines_crend (void) const noexcept;

    [[nodiscard]]
    use_timelines_reference
    use_timelines_front (void);

    [[nodiscard]]
    use_timelines_const_reference
    use_timelines_front (void) const;

    [[nodiscard]]
    use_timelines_reference
    use_timelines_back (void);

    [[nodiscard]]
    use_timelines_const_reference
    use_timelines_back (void) const;

    [[nodiscard]]
    use_timelines_size_type
    use_timelines_size (void) const noexcept;

    [[nodiscard]]
    bool
    use_timelines_empty (void) const noexcept;

    /* local timelines */

    [[nodiscard]]
    use_timelines_iterator
    local_begin (void) noexcept;

    [[nodiscard]]
    use_timelines_const_iterator
    local_begin (void) const noexcept;

    [[nodiscard]]
    use_timelines_const_iterator
    local_cbegin (void) const noexcept;

    [[nodiscard]]
    use_timelines_iterator
    local_end (void) noexcept;

    [[nodiscard]]
    use_timelines_const_iterator
    local_end (void) const noexcept;

    [[nodiscard]]
    use_timelines_const_iterator
    local_cend (void) const noexcept;

    [[nodiscard]]
    use_timelines_reverse_iterator
    local_rbegin (void) noexcept;

    [[nodiscard]]
    use_timelines_const_reverse_iterator
    local_rbegin (void) const noexcept;

    [[nodiscard]]
    use_timelines_const_reverse_iterator
    local_crbegin (void) const noexcept;

    [[nodiscard]]
    use_timelines_reverse_iterator
    local_rend (void) noexcept;

    [[nodiscard]]
    use_timelines_const_reverse_iterator
    local_rend (void) const noexcept;

    [[nodiscard]]
    use_timelines_const_reverse_iterator
    local_crend (void) const noexcept;

    [[nodiscard]]
    use_timelines_reference
    local_front (void);

    [[nodiscard]]
    use_timelines_const_reference
    local_front (void) const;

    [[nodiscard]]
    use_timelines_reference
    local_back (void);

    [[nodiscard]]
    use_timelines_const_reference
    local_back (void) const;

    [[nodiscard]]
    use_timelines_size_type
    local_size (void) const noexcept;

    [[nodiscard]]
    bool
    local_empty (void) const noexcept;

    /* successors */

    [[nodiscard]]
    successors_iterator
    successors_begin (void) noexcept;

    [[nodiscard]]
    successors_const_iterator
    successors_begin (void) const noexcept;

    [[nodiscard]]
    successors_const_iterator
    successors_cbegin (void) const noexcept;

    [[nodiscard]]
    successors_iterator
    successors_end (void) noexcept;

    [[nodiscard]]
    successors_const_iterator
    successors_end (void) const noexcept;

    [[nodiscard]]
    successors_const_iterator
    successors_cend (void) const noexcept;

    [[nodiscard]]
    successors_reverse_iterator
    successors_rbegin (void) noexcept;

    [[nodiscard]]
    successors_const_reverse_iterator
    successors_rbegin (void) const noexcept;

    [[nodiscard]]
    successors_const_reverse_iterator
    successors_crbegin (void) const noexcept;

    [[nodiscard]]
    successors_reverse_iterator
    successors_rend (void) noexcept;

    [[nodiscard]]
    successors_const_reverse_iterator
    successors_rend (void) const noexcept;

    [[nodiscard]]
    successors_const_reverse_iterator
    successors_crend (void) const noexcept;

    [[nodiscard]]
    successors_reference
    successors_front (void);

    [[nodiscard]]
    successors_const_reference
    successors_front (void) const;

    [[nodiscard]]
    successors_reference
    successors_back (void);

    [[nodiscard]]
    successors_const_reference
    successors_back (void) const;

    [[nodiscard]]
    successors_size_type
    successors_size (void) const noexcept;

    [[nodiscard]]
    bool
    successors_empty (void) const noexcept;

    /* instruction sub-ranges */

    [[nodiscard]]
    static
    ir_instruction_iter
    instructions_begin (use_timelines_iterator pos) noexcept;

    [[nodiscard]]
    static
    ir_instruction_citer
    instructions_begin (use_timelines_const_iterator pos) noexcept;

    [[nodiscard]]
    static
    ir_instruction_citer
    instructions_cbegin (use_timelines_const_iterator pos) noexcept;

    [[nodiscard]]
    ir_instruction_iter
    instructions_end (use_timelines_iterator pos) const noexcept;

    [[nodiscard]]
    ir_instruction_citer
    instructions_end (use_timelines_const_iterator pos) const noexcept;

    [[nodiscard]]
    ir_instruction_citer
    instructions_cend (use_timelines_const_iterator pos) const noexcept;

    [[nodiscard]]
    ir_instruction_riter
    instructions_rbegin (use_timelines_reverse_iterator pos) const noexcept;

    [[nodiscard]]
    ir_instruction_criter
    instructions_rbegin (use_timelines_const_reverse_iterator pos) const noexcept;

    [[nodiscard]]
    ir_instruction_criter
    instructions_crbegin (use_timelines_const_reverse_iterator pos) const noexcept;

    [[nodiscard]]
    static
    ir_instruction_riter
    instructions_rend (use_timelines_reverse_iterator pos) noexcept;

    [[nodiscard]]
    static
    ir_instruction_criter
    instructions_rend (use_timelines_const_reverse_iterator pos) noexcept;

    [[nodiscard]]
    static
    ir_instruction_criter
    instructions_crend (use_timelines_const_reverse_iterator pos) noexcept;

    [[nodiscard]]
    static
    ir_instruction&
    instructions_front (use_timelines_iterator pos) noexcept;

    [[nodiscard]]
    static
    const ir_instruction&
    instructions_front (use_timelines_const_iterator pos) noexcept;

    [[nodiscard]]
    ir_instruction&
    instructions_back (use_timelines_iterator pos) const noexcept;

    [[nodiscard]]
    const ir_instruction&
    instructions_back (use_timelines_const_iterator pos) const noexcept;

    [[nodiscard]]
    std::ptrdiff_t
    num_instructions (use_timelines_const_iterator pos) const noexcept;

    [[nodiscard]]
    bool
    has_instructions (use_timelines_const_iterator pos) const noexcept;

    /* use timeline queries */

    [[nodiscard]]
    bool
    has_timelines (void) const noexcept;

    [[nodiscard]]
    bool
    has_local_timelines (void) const noexcept;

    [[nodiscard]]
    bool
    has_incoming_timeline (void) const noexcept;

    [[nodiscard]]
    ir_use_timeline&
    get_incoming_timeline (void) noexcept;

    [[nodiscard]]
    const ir_use_timeline&
    get_incoming_timeline (void) const noexcept;

    [[nodiscard]]
    use_timelines_iterator
    get_incoming_timeline_iter (void) noexcept;

    [[nodiscard]]
    use_timelines_const_iterator
    get_incoming_timeline_iter (void) const noexcept;

    [[nodiscard]]
    optional_ref<ir_use_timeline>
    maybe_get_incoming_timeline (void) noexcept;

    [[nodiscard]]
    optional_ref<const ir_use_timeline>
    maybe_get_incoming_timeline (void) const noexcept;

    [[nodiscard]]
    optional_ref<ir_instruction>
    maybe_get_incoming_instruction (void) noexcept;

    [[nodiscard]]
    optional_ref<const ir_instruction>
    maybe_get_incoming_instruction (void) const noexcept;

    [[nodiscard]]
    bool
    has_outgoing_timeline (void) const noexcept;

    [[nodiscard]]
    ir_use_timeline&
    get_outgoing_timeline (void) noexcept;

    [[nodiscard]]
    const ir_use_timeline&
    get_outgoing_timeline (void) const noexcept;

    [[nodiscard]]
    use_timelines_iterator
    get_outgoing_timeline_iter (void) noexcept;

    [[nodiscard]]
    use_timelines_const_iterator
    get_outgoing_timeline_iter (void) const noexcept;

    [[nodiscard]]
    optional_ref<ir_use_timeline>
    maybe_get_outgoing_timeline (void) noexcept;

    [[nodiscard]]
    optional_cref<ir_use_timeline>
    maybe_get_outgoing_timeline (void) const noexcept;

    [[nodiscard]]
    ir_def&
    get_outgoing_def (void) noexcept;

    [[nodiscard]]
    const ir_def&
    get_outgoing_def (void) const noexcept;

    [[nodiscard]]
    optional_ref<ir_def>
    maybe_get_outgoing_def (void) noexcept;

    [[nodiscard]]
    optional_cref<ir_def>
    maybe_get_outgoing_def (void) const noexcept;

    void
    splice_local (use_timelines_const_iterator pos, ir_def_timeline& other);

    void
    splice_local (use_timelines_const_iterator pos, ir_def_timeline& other,
                  use_timelines_const_iterator first, use_timelines_const_iterator last);

    use_timelines_iterator
    emplace_local (use_timelines_const_iterator pos, ir_instruction_iter instr_pos);

    ir_use_timeline&
    emplace_back_local (ir_instruction_iter instr_pos);

    use_timelines_iterator
    erase_local (use_timelines_const_iterator pos);

    use_timelines_iterator
    erase_local (use_timelines_const_iterator first, use_timelines_const_iterator last);

    [[nodiscard]]
    intrusive_tracker<ir_use_timeline, remote::intrusive_reporter<ir_use>>::iterator
    first_use_after (ir_instruction_citer pos, use_timelines_iterator ut_it) const noexcept;

    [[nodiscard]]
    intrusive_tracker<ir_use_timeline, remote::intrusive_reporter<ir_use>>::const_iterator
    first_use_after (ir_instruction_citer pos, use_timelines_const_iterator ut_it) const noexcept;

    [[nodiscard]]
    ir_block&
    get_block (void) noexcept;

    [[nodiscard]]
    const ir_block&
    get_block (void) const noexcept;

    [[nodiscard]]
    ir_variable&
    get_variable (void) noexcept;

    [[nodiscard]]
    const ir_variable&
    get_variable (void) const noexcept;

    [[nodiscard]]
    std::size_t
    num_timelines (void) const noexcept;

    successors_iterator
    add_successor (ir_incoming_node&& node);

    successors_iterator
    remove_successor (ir_incoming_node& node);

    void
    transfer_successors (ir_def_timeline& src);

    template <typename ...Args>
    ir_incoming_node&
    append_incoming (ir_block& incoming_block, Args&&... args)
    {
      // if we don't have any incoming yet we need to start up the use-timeline
      if (! has_incoming_timeline ())
        create_incoming_timeline ();

      try
      {
        auto [pos, inserted] = m_incoming.emplace (
                                 std::piecewise_construct,
                                 std::forward_as_tuple (incoming_block),
                                 std::forward_as_tuple (std::forward<Args> (args)...));

        assert (inserted && "tried to emplace an incoming node for a key which already exists");
        return std::get<ir_incoming_node> (*pos);
      }
      catch (...)
      {
        destroy_incoming_timeline ();
        throw;
      }
    }

    void
    remove_incoming (incoming_citer pos);

    void
    remove_incoming (const ir_block& block)
    {
      return remove_incoming (find_incoming (block));
    }

    [[nodiscard]]
    incoming_iterator
    find_incoming (const ir_block& block) noexcept;

    [[nodiscard]]
    incoming_const_iterator
    find_incoming (const ir_block& block) const noexcept;

    ir_use_timeline&
    create_incoming_timeline (void);

    void
    destroy_incoming_timeline (void);

  private:
    nonnull_ptr<ir_block>    m_block;
    nonnull_ptr<ir_variable> m_var;
    incoming_map_type        m_incoming;
    use_timelines_list       m_use_timelines;
  };


}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_DEF_TIMELINE_HPP

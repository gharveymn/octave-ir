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

#include "utilities/ir-common-util.hpp"
#include "ir-component.hpp"
#include "values/types/ir-type.hpp"
#include "values/ir-instruction.hpp"
#include "values/ir-constant.hpp"
#include "utilities/ir-allocator-util.hpp"

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

namespace gch
{

  class ir_block;
  class ir_structure;
  class ir_variable;
  class ir_def;
  class ir_use;

  class ir_def_timeline;
  class ir_incoming_node;

  template <>
  struct ir_subcomponent_type_t<ir_block>
  {
    explicit ir_subcomponent_type_t (void) = default;
  };

  enum class ir_instruction_range : std::size_t
  {
    all  = base_subrange_index,
    phi  = 0,
    body = 1,
  };

  // On def_timeline architecture
  // A def_timeline contains a sequence of defs which were used in this block.
  //
  // There is a distinction between defs which were locally created and those
  // which were imported from a predecessor block. We refer to those as
  // 'incoming blocks' and 'incoming defs'.
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

  class ir_incoming_node
    : public intrusive_tracker<ir_incoming_node, remote::intrusive_tracker<ir_def_timeline>>
  {
    using base = intrusive_tracker<ir_incoming_node, remote::intrusive_tracker<ir_def_timeline>>;

  public:
    using element_allocator = strongly_linked_allocator<ir_incoming_node, ir_def_timeline>;

    ir_incoming_node            (void)                        = delete;
    ir_incoming_node            (const ir_incoming_node&)     = delete;
    ir_incoming_node            (ir_incoming_node&&) noexcept = delete;
    ir_incoming_node& operator= (const ir_incoming_node&)     = delete;
    ir_incoming_node& operator= (ir_incoming_node&&) noexcept;
    ~ir_incoming_node           (void)                        = default;

    ir_incoming_node (ir_def_timeline& parent, ir_incoming_node&& other) noexcept;

    ir_incoming_node (ir_def_timeline& parent, ir_block& incoming_block);
    ir_incoming_node (ir_def_timeline& parent, ir_block& incoming_block, ir_def_timeline& pred);

    template <typename Iterator>
    ir_incoming_node (ir_def_timeline& parent, ir_block& incoming_block,
                      Iterator first_pred, Iterator last_pred)
      : m_parent         (parent),
        m_incoming_block (incoming_block)
    {
      std::for_each (first_pred, last_pred, [&](auto&& ptr) { bind (*ptr); });
    }

    void
    set_parent (ir_def_timeline& dt) noexcept
    {
      m_parent.emplace (dt);
    }

    [[nodiscard]] constexpr
    ir_def_timeline&
    get_parent (void) noexcept
    {
      return *m_parent;
    }

    [[nodiscard]] constexpr
    const ir_def_timeline&
    get_parent (void) const noexcept
    {
      return *m_parent;
    }

    [[nodiscard]] constexpr
    ir_block&
    get_incoming_block (void) noexcept
    {
      return *m_incoming_block;
    }

    [[nodiscard]] constexpr
    const ir_block&
    get_incoming_block (void) const noexcept
    {
      return *m_incoming_block;
    }

    [[nodiscard]]
    ir_block&
    get_parent_block (void) noexcept;

    [[nodiscard]]
    const ir_block&
    get_parent_block (void) const noexcept;

    template <typename ...Args>
    iter
    add_predecessor (Args&&... args)
    {
      return base::bind (std::forward<Args> (args)...);
    }

    // return true if the removal caused a erasure
    iter
    remove_predecessor (const ir_def_timeline& dt);

    void
    clear (void) noexcept;

    void swap (ir_incoming_node& other) noexcept
    {
      base::swap (other);

      using std::swap;
      swap (m_incoming_block, other.m_incoming_block);
    }

  private:
    nonnull_ptr<ir_def_timeline> m_parent;
    nonnull_ptr<ir_block>        m_incoming_block;
  };

  void swap (ir_incoming_node& lhs, ir_incoming_node& rhs) noexcept
  {
    lhs.swap (rhs);
  }

  class ir_def_timeline
    : public intrusive_tracker<ir_def_timeline, remote::intrusive_tracker<ir_incoming_node>>
  {
  public:
    using range = ir_instruction_range;

    using use_timeline_list = list_partition<ir_use_timeline, 2>;
    using iter              = use_timeline_list::data_iterator;
    using citer             = use_timeline_list::data_const_iterator;
    using riter             = use_timeline_list::data_reverse_iterator;
    using criter            = use_timeline_list::data_const_reverse_iterator;
    using ref               = ir_use_timeline&;
    using cref              = const ir_use_timeline&;

    using incoming_container = std::vector<ir_incoming_node, ir_incoming_node::element_allocator>;
    using incoming_iter      = incoming_container::iterator;
    using incoming_citer     = incoming_container::const_iterator;
    using incoming_riter     = incoming_container::reverse_iterator;
    using incoming_criter    = incoming_container::const_reverse_iterator;
    using incoming_ref       = incoming_container::reference;
    using incoming_cref      = incoming_container::const_reference;

    using succ_tracker =
      intrusive_tracker<ir_def_timeline, remote::intrusive_tracker<ir_incoming_node>>;

    using succ_iter  = succ_tracker::iter;
    using succ_citer = succ_tracker::citer;

    using instr_iter   = ir_instruction_iter;
    using instr_citer  = ir_instruction_citer;
    using instr_riter  = ir_instruction_riter;
    using instr_criter = ir_instruction_criter;

    ir_def_timeline (void)                                  = delete;
    ir_def_timeline (const ir_def_timeline&)                = delete;
    ir_def_timeline (ir_def_timeline&&) noexcept;
    ir_def_timeline& operator= (const ir_def_timeline&)     = delete;
    ir_def_timeline& operator= (ir_def_timeline&&) noexcept;
    ~ir_def_timeline (void)                                 = default;

    ir_def_timeline (ir_block& block, ir_variable& var) noexcept
      : m_incoming (ir_incoming_node::element_allocator (*this)),
        m_block (block),
        m_var   (var)
    { }

    ir_def_timeline (ir_block& block, ir_block& incoming_block, ir_def_timeline& pred)
      : m_incoming (ir_incoming_node::element_allocator (*this)),
        m_block    (block),
        m_var      (pred.get_variable ())
    {
      m_incoming.emplace_back (incoming_block, pred);
    }

    void
    set_block (ir_block& block) noexcept
    {
      m_block.emplace (block);
    }

    /* clang-format off */
    [[nodiscard]] auto  incoming_begin   (void)       noexcept { return m_incoming.begin ();   }
    [[nodiscard]] auto  incoming_begin   (void) const noexcept { return m_incoming.begin ();   }
    [[nodiscard]] auto  incoming_cbegin  (void) const noexcept { return m_incoming.cbegin ();  }

    [[nodiscard]] auto  incoming_end     (void)       noexcept { return m_incoming.end ();     }
    [[nodiscard]] auto  incoming_end     (void) const noexcept { return m_incoming.end ();     }
    [[nodiscard]] auto  incoming_cend    (void) const noexcept { return m_incoming.cend ();    }

    [[nodiscard]] auto  incoming_rbegin  (void)       noexcept { return m_incoming.rbegin ();  }
    [[nodiscard]] auto  incoming_rbegin  (void) const noexcept { return m_incoming.rbegin ();  }
    [[nodiscard]] auto  incoming_crbegin (void) const noexcept { return m_incoming.crbegin (); }

    [[nodiscard]] auto  incoming_rend    (void)       noexcept { return m_incoming.rend ();    }
    [[nodiscard]] auto  incoming_rend    (void) const noexcept { return m_incoming.rend ();    }
    [[nodiscard]] auto  incoming_crend   (void) const noexcept { return m_incoming.crend ();   }

    [[nodiscard]] auto& incoming_front   (void)       noexcept { return m_incoming.front ();   }
    [[nodiscard]] auto& incoming_front   (void) const noexcept { return m_incoming.front ();   }

    [[nodiscard]] auto& incoming_back    (void)       noexcept { return m_incoming.back ();    }
    [[nodiscard]] auto& incoming_back    (void) const noexcept { return m_incoming.back ();    }

    [[nodiscard]] auto  incoming_size    (void) const noexcept { return m_incoming.size ();    }
    [[nodiscard]] auto  incoming_empty   (void) const noexcept { return m_incoming.empty ();   }

    template <range R>
    [[nodiscard]] constexpr
    auto&
    get_timelines (void) noexcept
    {
      return get_subrange<R> (m_use_timelines);
    }

    template <range R>
    [[nodiscard]] constexpr
    const auto&
    get_timelines (void) const noexcept
    {
      return get_subrange<R> (m_use_timelines);
    }

    template <range R> auto  timelines_begin   (void)       noexcept { return get_timelines<R> ().begin ();   }
    template <range R> auto  timelines_begin   (void) const noexcept { return get_timelines<R> ().begin ();   }
    template <range R> auto  timelines_cbegin  (void) const noexcept { return get_timelines<R> ().cbegin ();  }

    template <range R> auto  timelines_end     (void)       noexcept { return get_timelines<R> ().end ();     }
    template <range R> auto  timelines_end     (void) const noexcept { return get_timelines<R> ().end ();     }
    template <range R> auto  timelines_cend    (void) const noexcept { return get_timelines<R> ().cend ();    }

    template <range R> auto  timelines_rbegin  (void)       noexcept { return get_timelines<R> ().rbegin ();  }
    template <range R> auto  timelines_rbegin  (void) const noexcept { return get_timelines<R> ().rbegin ();  }
    template <range R> auto  timelines_crbegin (void) const noexcept { return get_timelines<R> ().crbegin (); }

    template <range R> auto  timelines_rend    (void)       noexcept { return get_timelines<R> ().rend ();    }
    template <range R> auto  timelines_rend    (void) const noexcept { return get_timelines<R> ().rend ();    }
    template <range R> auto  timelines_crend   (void) const noexcept { return get_timelines<R> ().crend ();   }

    template <range R> auto& timelines_front   (void)       noexcept { return get_timelines<R> ().front ();   }
    template <range R> auto& timelines_front   (void) const noexcept { return get_timelines<R> ().front ();   }

    template <range R> auto& timelines_back    (void)       noexcept { return get_timelines<R> ().back ();    }
    template <range R> auto& timelines_back    (void) const noexcept { return get_timelines<R> ().back ();    }

    template <range R> auto  timelines_size    (void) const noexcept { return get_timelines<R> ().size ();    }
    template <range R> auto  timelines_empty   (void) const noexcept { return get_timelines<R> ().empty ();   }

    [[nodiscard]] auto  succs_begin   (void)       noexcept { return succ_tracker::begin ();   }
    [[nodiscard]] auto  succs_begin   (void) const noexcept { return succ_tracker::begin ();   }
    [[nodiscard]] auto  succs_cbegin  (void) const noexcept { return succ_tracker::cbegin ();  }

    [[nodiscard]] auto  succs_end     (void)       noexcept { return succ_tracker::end ();     }
    [[nodiscard]] auto  succs_end     (void) const noexcept { return succ_tracker::end ();     }
    [[nodiscard]] auto  succs_cend    (void) const noexcept { return succ_tracker::cend ();    }

    [[nodiscard]] auto  succs_rbegin  (void)       noexcept { return succ_tracker::rbegin ();  }
    [[nodiscard]] auto  succs_rbegin  (void) const noexcept { return succ_tracker::rbegin ();  }
    [[nodiscard]] auto  succs_crbegin (void) const noexcept { return succ_tracker::crbegin (); }

    [[nodiscard]] auto  succs_rend    (void)       noexcept { return succ_tracker::rend ();    }
    [[nodiscard]] auto  succs_rend    (void) const noexcept { return succ_tracker::rend ();    }
    [[nodiscard]] auto  succs_crend   (void) const noexcept { return succ_tracker::crend ();   }

    [[nodiscard]] auto& succs_front   (void)       noexcept { return succ_tracker::front ();   }
    [[nodiscard]] auto& succs_front   (void) const noexcept { return succ_tracker::front ();   }

    [[nodiscard]] auto& succs_back    (void)       noexcept { return succ_tracker::back ();    }
    [[nodiscard]] auto& succs_back    (void) const noexcept { return succ_tracker::back ();    }

    [[nodiscard]] auto  succs_empty   (void) const noexcept { return succ_tracker::empty ();   }
    [[nodiscard]] auto  succs_size    (void) const noexcept { return succ_tracker::size ();    }
    /* clang-format on */

    [[nodiscard]] static instr_iter   instr_begin   (iter  pos)       noexcept;
    [[nodiscard]] static instr_citer  instr_begin   (citer pos)       noexcept;
    [[nodiscard]] static instr_citer  instr_cbegin  (citer pos)       noexcept;

    [[nodiscard]]        instr_iter   instr_end     (iter  pos) const noexcept;
    [[nodiscard]]        instr_citer  instr_end     (citer pos) const noexcept;
    [[nodiscard]]        instr_citer  instr_cend    (citer pos) const noexcept;

    [[nodiscard]]        instr_riter  instr_rbegin  (iter  pos) const noexcept;
    [[nodiscard]]        instr_criter instr_rbegin  (citer pos) const noexcept;
    [[nodiscard]]        instr_criter instr_crbegin (citer pos) const noexcept;

    [[nodiscard]] static instr_riter  instr_rend    (iter  pos)       noexcept;
    [[nodiscard]] static instr_criter instr_rend    (citer pos)       noexcept;
    [[nodiscard]] static instr_criter instr_crend   (citer pos)       noexcept;

    [[nodiscard]] static       auto& instr_front (iter  pos) noexcept { return *instr_begin (pos); }
    [[nodiscard]] static const auto& instr_front (citer pos) noexcept { return *instr_begin (pos); }

    [[nodiscard]]       auto& instr_back (iter  pos) const noexcept { return *instr_rbegin (pos); }
    [[nodiscard]] const auto& instr_back (citer pos) const noexcept { return *instr_rbegin (pos); }

    [[nodiscard]]
    std::ptrdiff_t
    num_instrs (citer pos) const noexcept;

    [[nodiscard]]
    bool
    has_instrs (citer pos) const noexcept;

    [[nodiscard]]
    bool
    has_incoming (void) const noexcept;

    [[nodiscard]]
    bool
    has_timelines (void) const noexcept;

    [[nodiscard]]
    bool
    has_incoming_timeline (void) const noexcept;

    [[nodiscard]]
    bool
    has_local_timelines (void) const noexcept;

    [[nodiscard]]
    ir_use_timeline&
    get_incoming_timeline (void) noexcept;

    [[nodiscard]]
    const ir_use_timeline&
    get_incoming_timeline (void) const noexcept;

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

    template <range R>
    void
    splice (iter pos, ir_def_timeline& other);

    template <range R>
    void
    splice (iter pos, ir_def_timeline& other, iter first, iter last);

    template <range R>
    iter
    emplace_before (citer pos, instr_iter instr_pos);

    template <range R>
    ir_use_timeline&
    emplace_back (instr_iter instr_pos);

    template <range R>
    iter
    erase (citer pos);

    template <range R>
    iter
    erase (citer first, citer last);

    [[nodiscard]]
    ir_use_timeline::iter
    find_first_after (iter ut_it, instr_citer pos) const noexcept;

    [[nodiscard]]
    ir_use_timeline::citer
    find_first_after (citer ut_it, instr_citer pos) const noexcept;

    [[nodiscard]] constexpr
    ir_block&
    get_block (void) noexcept
    {
      return *m_block;
    }

    [[nodiscard]] constexpr
    const ir_block&
    get_block (void) const noexcept
    {
      return *m_block;
    }

    [[nodiscard]] constexpr
    ir_variable&
    get_variable (void) noexcept
    {
      return *m_var;
    }

    [[nodiscard]] constexpr
    const ir_variable&
    get_variable (void) const noexcept
    {
      return *m_var;
    }

    [[nodiscard]]
    std::size_t
    num_timelines (void) const noexcept
    {
      return get_timelines<range::all> ().size ();
    }

    succ_iter
    add_successor (ir_incoming_node& node);

    succ_iter
    remove_successor (ir_incoming_node& node);

    void
    transfer_successors (ir_def_timeline& src);

    template <typename ...Args>
    ir_incoming_node&
    append_incoming (ir_block& incoming_block, Args&&... args)
    {
      // if we don't have any incoming yet we need to start up the use-timeline
      if (incoming_empty ())
        create_incoming_timeline ();

      try
      {
        return m_incoming.emplace_back (incoming_block, std::forward<Args> (args)...);
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
    incoming_iter
    find_incoming (const ir_block& block) noexcept;

    [[nodiscard]]
    incoming_citer
    find_incoming (const ir_block& block) const noexcept
    {
      return as_mutable (*this).find_incoming (block);
    }

    ir_use_timeline&
    create_incoming_timeline (void);

    void
    destroy_incoming_timeline (void);

  private:
    nonnull_ptr<ir_block>    m_block;
    nonnull_ptr<ir_variable> m_var;
    incoming_container       m_incoming;
    use_timeline_list        m_use_timelines;
  };

  class ir_block
    : public ir_subcomponent,
      public visitable<ir_block, ir_component_visitors>
  {
  public:
    using instruction_container = std::list<ir_instruction>;

    using iter   = instruction_container::iterator;
    using citer  = instruction_container::const_iterator;
    using riter  = instruction_container::reverse_iterator;
    using criter = instruction_container::const_reverse_iterator;
    using ref    = instruction_container::reference;
    using cref   = instruction_container::const_reference;
    using size_t = instruction_container::size_type;
    using diff_t = instruction_container::difference_type;

    using dt_map_type            = std::unordered_map<const ir_variable *, ir_def_timeline>;
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

    template <range R> auto  begin   (void)       noexcept { return get<R> ().begin ();   }
    template <range R> auto  begin   (void) const noexcept { return get<R> ().begin ();   }
    template <range R> auto  cbegin  (void) const noexcept { return get<R> ().cbegin ();  }

    template <range R> auto  end     (void)       noexcept { return get<R> ().end ();     }
    template <range R> auto  end     (void) const noexcept { return get<R> ().end ();     }
    template <range R> auto  cend    (void) const noexcept { return get<R> ().cend ();    }

    template <range R> auto  rbegin  (void)       noexcept { return get<R> ().rbegin ();  }
    template <range R> auto  rbegin  (void) const noexcept { return get<R> ().rbegin ();  }
    template <range R> auto  crbegin (void) const noexcept { return get<R> ().crbegin (); }

    template <range R> auto  rend    (void)       noexcept { return get<R> ().rend ();    }
    template <range R> auto  rend    (void) const noexcept { return get<R> ().rend ();    }
    template <range R> auto  crend   (void) const noexcept { return get<R> ().crend ();   }

    template <range R> auto& front   (void)       noexcept { return get<R> ().front ();   }
    template <range R> auto& front   (void) const noexcept { return get<R> ().front ();   }

    template <range R> auto& back    (void)       noexcept { return get<R> ().back ();    }
    template <range R> auto& back    (void) const noexcept { return get<R> ().back ();    }

    template <range R> auto  size    (void) const noexcept { return get<R> ().size ();    }
    template <range R> auto  empty   (void) const noexcept { return get<R> ().empty ();   }

    /* clang-format on */

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
    prepare_operand (const citer&, const ir_constant& c) noexcept;

    static
    ir_constant&&
    prepare_operand (const citer&, ir_constant&& t) noexcept;

    ir_use_timeline&
    track_def_at (ir_variable& var, iter pos);

  public:
    iter
    create_phi (ir_variable& var);

    // unsafe
    iter
    erase_phi (ir_variable& var);

    ir_block&
    split_into (citer pivot, ir_block& dest);

    ir_def_timeline
    split (ir_def_timeline& dt, citer pivot, ir_block& dest);

    [[nodiscard]]
    ir_def_timeline::riter
    find_latest_timeline (ir_def_timeline& dt) const;

    [[nodiscard]]
    ir_def_timeline::riter
    find_latest_timeline_before (ir_def_timeline& dt, citer pos) const;

    [[nodiscard]]
    ir_use_timeline&
    get_latest_timeline (ir_def_timeline& dt);

    [[nodiscard]]
    ir_use_timeline&
    get_latest_timeline (ir_variable& var);

    [[nodiscard]]
    ir_use_timeline&
    get_latest_timeline_before (ir_def_timeline& dt, citer pos);

    [[nodiscard]]
    ir_use_timeline&
    get_latest_timeline_before (ir_variable& var, citer pos);

    iter
    remove_instruction (citer pos);

    iter
    remove_range (citer first, citer last) noexcept;

    ir_use_timeline
    split_uses (ir_use_timeline& src, citer pivot, citer last);

    ir_use_timeline::iter
    find_first_use_after (ir_use_timeline& tl, citer pos, citer last);

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
    // virtual from ir_component
    //

    bool
    reassociate_timelines (const ir_link_set<ir_def_timeline>& old_dts, ir_def_timeline& new_dt,
                           std::vector<nonnull_ptr<ir_block>>& until) override;

    void
    reset (void) noexcept override;

    //
    // templated functions
    //

    // with return (places instruction immediately before `pos`)
    template <ir_opcode Op, typename ...Args,
              std::enable_if_t<ir_instruction_traits<Op>::will_return> * = nullptr>
    ir_instruction&
    emplace_instruction (const citer pos, ir_variable& v, Args&&... args)
    {
      iter it = emplace<range::body, Op> (v, prepare_operand (pos, std::forward<Args> (args))...);
      try
      {
        track_def_at (v, it);
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
      std::enable_if_t<! ir_instruction_traits<Op>::will_return> * = nullptr>
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
    list_partition<ir_instruction, 2> m_instr_partition;
    dt_map_type                       m_def_timelines_map;
  };

  class ir_condition_block
    : public ir_block
  {
  public:
    using ir_block::ir_block;
  };

  class ir_loop_condition_block
    : public ir_block
  {
  public:
    using ir_block::ir_block;
  };

}

#endif

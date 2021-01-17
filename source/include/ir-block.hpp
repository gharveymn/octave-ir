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


#if ! defined (ir_block_h)
#define ir_block_h 1

#include "ir-common-util.hpp"
#include "ir-component.hpp"
#include "ir-type.hpp"
#include "ir-instruction.hpp"
#include "ir-constant.hpp"

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
  class ir_structure;
  class ir_variable;
  class ir_def;
  class ir_use;

  class ir_def_timeline;
  class ir_incoming_node;

  // On def_timeline architecture
  // A def_timeline contains a sequence of defs which were used in this block.
  //
  // There is a distinction between defs which were locally created and those
  // which were imported from a predecessor block. We refer to those as
  // 'incoming blocks' and 'incoming defs'.
  //
  // Locally created defs are trivial to organize, but the incoming defs are
  // troublesome. This is for a a few reasons, namely,
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
    ir_incoming_node            (void)                     = delete;
    ir_incoming_node            (const ir_incoming_node&)     = delete;
//  ir_incoming_node            (ir_incoming_node&&) noexcept = impl;
    ir_incoming_node& operator= (const ir_incoming_node&)     = delete;
//  ir_incoming_node& operator= (ir_incoming_node&&) noexcept = impl;
    ~ir_incoming_node           (void)                     = default;

    // ONLY FOR USE WITH ir_def_timeline
    // fix up m_parent after move!
    ir_incoming_node (ir_incoming_node&& other) noexcept;

    ir_incoming_node& operator= (ir_incoming_node&& other) noexcept;

    ir_incoming_node (ir_incoming_node&& other, ir_def_timeline& new_parent) noexcept;

    ir_incoming_node (ir_def_timeline& parent, ir_basic_block& incoming_block,
                      ir_def_timeline& pred);

    template <typename Iterator>
    ir_incoming_node (ir_def_timeline& parent, ir_basic_block& incoming_block,
                      Iterator first_pred, Iterator last_pred)
      : base (tag::bind, first_pred, last_pred),
        m_parent (parent),
        m_incoming_block (incoming_block)
    { }

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
    ir_basic_block&
    get_incoming_block (void) noexcept
    {
      return *m_incoming_block;
    }

    [[nodiscard]] constexpr
    const ir_basic_block&
    get_incoming_block (void) const noexcept
    {
      return *m_incoming_block;
    }

    [[nodiscard]] constexpr       ir_basic_block& get_parent_block (void)       noexcept;
    [[nodiscard]] constexpr const ir_basic_block& get_parent_block (void) const noexcept;

    iter
    add_predecessor (ir_def_timeline& dt);

    template <typename Iter>
    iter
    add_predecessor (Iter first, Iter last)
    {
      return base::bind (first, last);
    }

    // return true if the removal caused a erasure
    iter
    remove_predecessor (const ir_def_timeline& dt);

    void swap (ir_incoming_node& other) noexcept
    {
      base::swap (other);

      using std::swap;
      swap (m_incoming_block, other.m_incoming_block);
    }

  private:
    nonnull_ptr<ir_def_timeline> m_parent;
    nonnull_ptr<ir_basic_block>  m_incoming_block;
  };

  void swap (ir_incoming_node& lhs, ir_incoming_node& rhs) noexcept
  {
    lhs.swap (rhs);
  }

  class ir_def_timeline
    : public intrusive_tracker<ir_def_timeline, remote::intrusive_tracker<ir_incoming_node>>
  {
  public:
    using use_timeline_list = std::list<ir_use_timeline>;
    using iter              = use_timeline_list::iterator;
    using citer             = use_timeline_list::const_iterator;
    using riter             = use_timeline_list::reverse_iterator;
    using criter            = use_timeline_list::const_reverse_iterator;
    using ref               = ir_use_timeline&;
    using cref              = const ir_use_timeline&;

    using incoming_container = std::vector<ir_incoming_node>;
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

    explicit ir_def_timeline (ir_basic_block& block, ir_variable& var) noexcept
      : m_block (block),
        m_var   (var)
    { }

    ir_def_timeline (ir_basic_block& block, ir_basic_block& incoming_block, ir_def_timeline& pred)
      : m_incoming { { *this, incoming_block, pred } },
        m_block    (block),
        m_var      (pred.get_variable ())
    { }

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


    [[nodiscard]] auto  local_begin   (void)       noexcept { return m_use_timelines.begin ();   }
    [[nodiscard]] auto  local_begin   (void) const noexcept { return m_use_timelines.begin ();   }
    [[nodiscard]] auto  local_cbegin  (void) const noexcept { return m_use_timelines.cbegin ();  }

    [[nodiscard]] auto  local_end     (void)       noexcept { return m_use_timelines.end ();     }
    [[nodiscard]] auto  local_end     (void) const noexcept { return m_use_timelines.end ();     }
    [[nodiscard]] auto  local_cend    (void) const noexcept { return m_use_timelines.cend ();    }

    [[nodiscard]] auto  local_rbegin  (void)       noexcept { return m_use_timelines.rbegin ();  }
    [[nodiscard]] auto  local_rbegin  (void) const noexcept { return m_use_timelines.rbegin ();  }
    [[nodiscard]] auto  local_crbegin (void) const noexcept { return m_use_timelines.crbegin (); }

    [[nodiscard]] auto  local_rend    (void)       noexcept { return m_use_timelines.rend ();    }
    [[nodiscard]] auto  local_rend    (void) const noexcept { return m_use_timelines.rend ();    }
    [[nodiscard]] auto  local_crend   (void) const noexcept { return m_use_timelines.crend ();   }

    [[nodiscard]] auto& local_front   (void)       noexcept { return m_use_timelines.front ();   }
    [[nodiscard]] auto& local_front   (void) const noexcept { return m_use_timelines.front ();   }

    [[nodiscard]] auto& local_back    (void)       noexcept { return m_use_timelines.back ();    }
    [[nodiscard]] auto& local_back    (void) const noexcept { return m_use_timelines.back ();    }

    [[nodiscard]] auto  local_size    (void) const noexcept { return m_use_timelines.size ();    }
    [[nodiscard]] auto  local_empty   (void) const noexcept { return m_use_timelines.empty ();   }


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

    [[nodiscard]] auto& instr_front (iter  pos) const noexcept { return *instr_begin (pos); }
    [[nodiscard]] auto& instr_front (citer pos) const noexcept { return *instr_begin (pos); }

    [[nodiscard]] auto& instr_back  (iter  pos) const noexcept { return *instr_rbegin (pos); }
    [[nodiscard]] auto& instr_back  (citer pos) const noexcept { return *instr_rbegin (pos); }

    [[nodiscard]]
    auto
    num_instrs (citer pos) const noexcept
    {
      return std::distance (instr_begin (pos), instr_end (pos));
    }

    [[nodiscard]]
    bool
    has_instrs (citer pos) const noexcept
    {
      return instr_begin (pos) == instr_end (pos);
    }

    [[nodiscard]]
    bool
    has_incoming (void) const noexcept
    {
      return ! incoming_empty ();
    }

    [[nodiscard]]
    ir_use_timeline::iter
    find_first_after (iter ut_it, instr_citer pos) const noexcept;

    [[nodiscard]] ir_use_timeline::citer
    find_first_after (citer ut_it, instr_citer pos) const noexcept;

    [[nodiscard]] constexpr
    ir_basic_block&
    get_block (void) noexcept
    {
      return *m_block;
    }

    [[nodiscard]] constexpr
    const ir_basic_block&
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

    void
    splice (iter pos, ir_def_timeline& other)
    {
      m_use_timelines.splice (pos, other.m_use_timelines);
    }

    void
    splice (iter pos, ir_def_timeline& other, iter first, iter last)
    {
      m_use_timelines.splice (pos, other.m_use_timelines, first, last);
    }

    iter
    emplace_before (citer pos, instr_iter instr_pos);

    ir_use_timeline&
    emplace_back (instr_iter instr_pos);

    iter
    erase (citer pos)
    {
      return m_use_timelines.erase (pos);
    }

    iter
    erase (citer first, citer last)
    {
      return m_use_timelines.erase (first, last);
    }

    [[nodiscard]]
    std::size_t
    num_defs (void) const noexcept
    {
      return m_use_timelines.size ();
    }

    succ_iter
    add_successor (ir_incoming_node& node);

    succ_iter
    remove_successor (ir_incoming_node& node);

    void
    transfer_successors (ir_def_timeline& src);

    [[nodiscard]] constexpr
    optional_ref<ir_use_timeline>
    get_incoming_timeline (void) noexcept
    {
      if (has_incoming ())
        return m_use_timelines.front ();
      return nullopt;
    }

    [[nodiscard]] constexpr
    optional_ref<const ir_use_timeline>
    get_incoming_timeline (void) const noexcept
    {
      return const_cast<ir_def_timeline *> (this)->get_incoming_timeline ();
    }

    [[nodiscard]] constexpr
    optional_ref<ir_instruction>
    get_incoming_instruction (void) noexcept
    {
      if (auto tl = get_incoming_timeline ())
        return tl->get_def_instruction ();
      return nullopt;
    }

    [[nodiscard]] constexpr
    optional_ref<const ir_instruction>
    get_incoming_instruction (void) const noexcept
    {
      return const_cast<ir_def_timeline *> (this)->get_incoming_instruction ();
    }

    ir_incoming_node&
    append_incoming (ir_basic_block& incoming_block, ir_def_timeline& pred);

    ir_incoming_node&
    remove_incoming (ir_def_timeline& pred);

    incoming_iter  find_incoming (const ir_basic_block& block)       noexcept;
    incoming_citer find_incoming (const ir_basic_block& block) const noexcept;

  private:
    nonnull_ptr<ir_basic_block>    m_block;
    nonnull_ptr<ir_variable>       m_var;
    incoming_container             m_incoming;
    use_timeline_list              m_use_timelines;
  };

  constexpr ir_basic_block&
  ir_incoming_node::get_parent_block (void) noexcept
  {
    return get_parent ().get_block ();
  }

  constexpr const ir_basic_block&
  ir_incoming_node::get_parent_block (void) const noexcept
  {
    return get_parent ().get_block ();
  }

  class ir_basic_block : public ir_component
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

  private:
    using def_timeline_map = std::unordered_map<const ir_variable *, ir_def_timeline>;

  public:
    ir_basic_block            (void)                      = delete;
    ir_basic_block            (const ir_basic_block&)     = delete;
    ir_basic_block            (ir_basic_block&&) noexcept = default;
    ir_basic_block& operator= (const ir_basic_block&)     = delete;
    ir_basic_block& operator= (ir_basic_block&&) noexcept = delete;
    ~ir_basic_block           (void) noexcept override    = default;

    explicit ir_basic_block (ir_structure& parent)
      : ir_component (parent),
        m_instr_partition ()
    { }

    enum class range : std::size_t
    {
      phi  = 0,
      body = 1,
    };

    /* clang-format off */
    // all
    [[nodiscard]]
    auto get_data_view (void) const noexcept
    {
      return m_instr_partition.data_view ();
    }

    template <range Range>
    [[nodiscard]]
    constexpr auto& get (void) noexcept
    {
      return get_subrange<Range> (m_instr_partition);
    }

    template <range Range>
    [[nodiscard]]
    constexpr auto& get (void) const noexcept
    {
      return get_subrange<Range> (m_instr_partition);
    }

    template <range Range> [[nodiscard]] auto  begin   (void)       noexcept { return get<Range> ().begin ();   }
    template <range Range> [[nodiscard]] auto  begin   (void) const noexcept { return get<Range> ().begin ();   }
    template <range Range> [[nodiscard]] auto  cbegin  (void) const noexcept { return get<Range> ().cbegin ();  }

    template <range Range> [[nodiscard]] auto  end     (void)       noexcept { return get<Range> ().end ();     }
    template <range Range> [[nodiscard]] auto  end     (void) const noexcept { return get<Range> ().end ();     }
    template <range Range> [[nodiscard]] auto  cend    (void) const noexcept { return get<Range> ().cend ();    }

    template <range Range> [[nodiscard]] auto  rbegin  (void)       noexcept { return get<Range> ().rbegin ();  }
    template <range Range> [[nodiscard]] auto  rbegin  (void) const noexcept { return get<Range> ().rbegin ();  }
    template <range Range> [[nodiscard]] auto  crbegin (void) const noexcept { return get<Range> ().crbegin (); }

    template <range Range> [[nodiscard]] auto  rend    (void)       noexcept { return get<Range> ().rend ();    }
    template <range Range> [[nodiscard]] auto  rend    (void) const noexcept { return get<Range> ().rend ();    }
    template <range Range> [[nodiscard]] auto  crend   (void) const noexcept { return get<Range> ().crend ();   }

    template <range Range> [[nodiscard]] auto& front   (void)       noexcept { return get<Range> ().front ();   }
    template <range Range> [[nodiscard]] auto& front   (void) const noexcept { return get<Range> ().front ();   }

    template <range Range> [[nodiscard]] auto& back    (void)       noexcept { return get<Range> ().back ();    }
    template <range Range> [[nodiscard]] auto& back    (void) const noexcept { return get<Range> ().back ();    }

    template <range Range> [[nodiscard]] auto  size    (void) const noexcept { return get<Range> ().size ();    }
    template <range Range> [[nodiscard]] auto  empty   (void) const noexcept { return get<Range> ().empty ();   }

    /* clang-format on */

    ir_basic_block& split_into (citer pivot, ir_basic_block& dest);

    ir_def_timeline split (ir_def_timeline& dt, citer pivot, ir_basic_block& dest);

    [[nodiscard]]
    ir_def_timeline::riter find_latest_timeline_before (ir_def_timeline& dt, citer pos) const;

    [[nodiscard]]
    optional_ref<ir_use_timeline> get_latest_timeline (ir_variable& var);

    [[nodiscard]]
    optional_ref<ir_use_timeline> get_latest_timeline_before (citer pos, ir_variable& var);

    [[nodiscard]]
    std::optional<std::pair<nonnull_ptr<ir_use_timeline>, criter>>
    get_pair_latest_timeline_before (citer pos, ir_variable& var);

    [[nodiscard]]
    optional_ref<ir_def> get_latest_def (ir_variable& var)
    {
      if (optional_ref<ir_use_timeline> tl = get_latest_timeline (var))
        return tl->get_def ();
      return nullopt;
    }

    [[nodiscard]]
    optional_ref<ir_def> get_latest_def_before (citer pos, ir_variable& var)
    {
      if (optional_ref<ir_use_timeline> tl = get_latest_timeline_before (pos, var))
        return tl->get_def ();
      return nullopt;
    }

  private:
    template <range Range, ir_opcode Op, typename ...Args>
    decltype (auto)
    emplace_front (Args&&... args)
    {
      return get<Range> ().emplace_front (ir_instruction_type<Op>, std::forward<Args> (args)...);
    }

    template <range Range, ir_opcode Op, typename ...Args>
    decltype (auto)
    emplace (citer pos, Args&&... args)
    {
      return get<Range> ().emplace (pos, ir_instruction_type<Op>, std::forward<Args> (args)...);
    }

    template <range Range, ir_opcode Op, typename ...Args>
    decltype (auto)
    emplace_back (Args&&... args)
    {
      return get<Range> ().emplace_back (ir_instruction_type<Op>, std::forward<Args> (args)...);
    }

    template <range Range, typename ...Args>
    decltype (auto)
    splice (citer pos, ir_basic_block& other, Args&&... args)
    {
      return get<Range> ().splice (pos, other.get<Range> (), std::forward<Args> (args)...);
    }

    template <range Range>
    decltype (auto)
    pop_front (void)
    {
      return get<Range> ().pop_front ();
    }

    template <range Range>
    decltype (auto)
    pop_back (void)
    {
      return get<Range> ().pop_back ();
    }

    optional_ref<ir_def_timeline> maybe_join_incoming (ir_variable& var);

    std::vector<nonnull_ptr<ir_def_timeline>> collect_defs_incoming (ir_variable& var);
    std::vector<nonnull_ptr<ir_def_timeline>> collect_defs_outgoing (ir_variable& var);

    std::vector<std::pair<nonnull_ptr<ir_basic_block>, optional_ref<ir_def_timeline>>>
    collect_outgoing (ir_variable& var) override;

    ir_def_timeline& append_incoming (ir_variable& var, ir_def_timeline& dt,
                                   ir_basic_block& incoming_block, ir_def_timeline& pred);

    ir_def_timeline& append_incoming (ir_variable& var, ir_def_timeline& dt,
                                   ir_basic_block& incoming_block,
                                   const std::vector<nonnull_ptr<ir_def_timeline>>& preds);

    ir_def_timeline& resolve_undefined_incoming (ir_variable& undef_var, ir_def_timeline& var_dt);

    ir_def_timeline& resolve_undefined_outgoing (ir_variable& undef_var, ir_def_timeline& var_dt);

    void propagate_def_timeline (ir_variable& var, ir_basic_block& incoming_block,
                                 ir_def_timeline& remote);

    ir_def_timeline& set_undefined_state (ir_variable& undef_var, bool state);

  public:
    template <typename ...Args>
    iter
    create_phi (ir_variable& var, Args&&... args)
    {
      emplace_back<range::phi, ir_opcode::phi> (var, std::forward<Args> (args)...);
      return std::prev (end<range::phi> ());
    }

    // unsafe
    iter erase_phi (ir_variable& var)
    {
      auto pos = std::find_if (begin<range::phi> (), end<range::phi> (),
                               [&var] (const ir_instruction& instr)
                               {
                                 return &instr.get_return ().get_var () == &var;
                               });
      // error checking
      if (pos == get<range::phi> ().end ())
        throw ir_exception ("tried to erase a nonexistent phi instruction");
      return get<range::phi> ().erase (pos);
    }

    std::optional<ir_operand_pre::use_pair> prepare_operand (citer pos, ir_variable& var);

    static constexpr const ir_constant& prepare_operand (const citer&, const ir_constant& c)
      noexcept
    {
      return c;
    }

    static constexpr ir_constant&& prepare_operand (const citer&, ir_constant&& t) noexcept
    {
      return std::move (t);
    }

    // with return
    template <ir_opcode Op, typename ...Args,
              std::enable_if_t<ir_instruction_metadata::has_return<Op> ()>* = nullptr>
    ir_instruction& append_instruction (ir_variable& var, Args&&... args)
    {
      iter it = emplace<range::body, Op> (end<range::body> (), var,
        prepare_operand (end<range::body> (), std::forward<Args> (args))...);

      try
      {
        get_def_timeline (var).emplace_back (it);
      }
      catch (...)
      {
        pop_back<range::body> ();
        throw;
      }
      return *it;
    }

    // no return
    template <ir_opcode Op, typename ...Args,
      std::enable_if_t<! ir_instruction_metadata::has_return<Op> ()>* = nullptr>
    ir_instruction& append_instruction (Args&&... args)
    {
      return emplace_back<range::body, Op> (
        prepare_operand (end<range::body> (), std::forward<Args> (args))...);
    }

    // with return (places instruction at the front of the body)
    template <ir_opcode Op, typename ...Args,
      std::enable_if_t<ir_instruction_metadata::has_return<Op> ()>* = nullptr>
    ir_instruction& prepend_instruction (ir_variable& v, Args&&... args)
    {
      ir_instruction& instr = emplace_front<range::body, Op> (v,
        prepare_operand (begin<range::body> (), std::forward<Args> (args))...);
      try
      {
        ir_def_timeline& dt = get_def_timeline (v);
        dt.emplace_before (find_latest_timeline_before (dt, begin<range::body> ()).base (), instr);
      }
      catch (...)
      {
        pop_front<range::body> ();
        throw;
      }
      return instr;
    }

    // no return (places instruction at the front of the body)
    template <ir_opcode Op, typename ...Args,
      std::enable_if_t<! ir_instruction_metadata::has_return<Op> ()>* = nullptr>
    ir_instruction& prepend_instruction (Args&&... args)
    {
      return get<range::body> ().emplace_front (ir_instruction::create<Tag> (
        prepare_operand (begin<range::body> (), std::forward<Args> (args))...));
    }

    // with return (places instruction immediately before `pos`)
    template <ir_opcode Op, typename ...Args,
      std::enable_if_t<ir_instruction_metadata::has_return<Op> ()>* = nullptr>
    ir_instruction& emplace_instruction (const citer pos, ir_variable& v, Args&&... args)
    {
      iter it = get_body ().emplace (ir_instruction::create<Tag> (v,
        prepare_operand (pos, std::forward<Args> (args))...));
      try
      {
        ir_def_timeline& dt = get_def_timeline (v);
        dt.emplace_before (find_latest_timeline_before (dt, it).base (), *it);
      }
      catch (...)
      {
        get_body ().erase (it);
        throw;
      }
      return *it;
    }

    // no return (places instruction immediately before `pos`)
    template <ir_opcode Op, typename ...Args,
      std::enable_if_t<! ir_instruction_metadata::has_return<Op> ()>* = nullptr>
    ir_instruction& emplace_instruction (const citer pos, Args&&... args)
    {
      return *get_body ().emplace(pos, ir_instruction::create<Tag> (
        prepare_operand (pos, std::forward<Args> (args))...));
    }

    iter erase (citer pos);
    iter erase (citer first, citer last) noexcept;

    // predecessors
    link_iter   preds_begin (void);
    link_iter   preds_end   (void);
    std::size_t num_preds   (void);

    nonnull_ptr<ir_basic_block> preds_front (void) { return *preds_begin ();   }
    nonnull_ptr<ir_basic_block> preds_back  (void) { return *(--preds_end ()); }

    [[nodiscard]]
    bool has_preds (void) { return preds_begin () != preds_end (); }

    [[nodiscard]]
    bool has_preds (citer pos) { return pos != begin<range::body> () || has_preds (); }

    [[nodiscard]]
    bool has_multiple_preds (void) { return num_preds () > 1; }

    // successors
    link_iter   succs_begin (void);
    link_iter   succs_end   (void);
    std::size_t num_succs  (void);

    nonnull_ptr<ir_basic_block> succs_front (void) { return *succs_begin ();   }
    nonnull_ptr<ir_basic_block> succs_back  (void) { return *(--succs_end ()); }

    bool has_succs (void)            { return succs_begin () != succs_end (); }
    bool has_succs (citer pos) { return pos != end<range::body> () || has_succs (); }

    bool has_multiple_succs (void) { return num_succs () > 1; }

    link_iter leaf_begin (void) noexcept override
    {
      return value_begin<nonnull_ptr<ir_basic_block>> (*this);
    }

    link_iter leaf_end (void) noexcept override
    {
      return value_end<nonnull_ptr<ir_basic_block>> (*this);
    }

    ir_basic_block& get_entry_block (void) noexcept override { return *this; }

    ir_function& get_function (void) noexcept override;

    void reset (void) noexcept override;

    /**
     * Create a def before the specified position.
     *
     * @param var the variable the def will be associated with.
     * @param pos an iterator immediately the position of the def.
     * @return the created def.
     */
    auto create_def_before (ir_variable& var, citer pos);

    ir_use_timeline split_uses (ir_use_timeline& src, citer pivot, citer last);

    ir_use_timeline::iter find_first_use_after (ir_use_timeline& tl, citer pos,
                                                citer last);

  protected:

    optional_ref<const ir_def_timeline> find_def_timeline (const ir_variable& var) const
    {
      if (auto cit = m_timeline_map.find (&var); cit != m_timeline_map.end ())
        return std::get<ir_def_timeline> (*cit);
      return nullopt;
    }

    optional_ref<ir_def_timeline> find_def_timeline (const ir_variable& var)
    {
      if (auto cit = m_timeline_map.find (&var); cit != m_timeline_map.end ())
        return std::get<ir_def_timeline> (*cit);
      return nullopt;
    }

    ir_def_timeline& get_def_timeline (const ir_variable& var)
    {
      return std::get<ir_def_timeline> (*m_timeline_map.try_emplace (&var, *this).first);
    }

  private:
    list_partition<ir_instruction, 4> m_instr_partition;
    def_timeline_map                  m_timeline_map;
  };

  class ir_condition_block : public ir_basic_block
  {
  public:
    using ir_basic_block::ir_basic_block;
  };

  class ir_loop_condition_block : public ir_basic_block
  {
  public:

    using ir_basic_block::ir_basic_block;

//    ir_def * join_pred_defs (ir_variable& var) override;

  private:
  };

}

#endif

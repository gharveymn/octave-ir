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

#include <gch/tracker.hpp>
#include <gch/optional_ref.hpp>
#include <gch/partition/list_partition.hpp>

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

  class ir_use_timeline
    : public intrusive_tracker<ir_use_timeline, remote::intrusive_reporter<ir_use>>
  {
  public:
    using use_tracker  = intrusive_tracker<ir_use_timeline, remote::intrusive_reporter<ir_use>>;
    using instr_riter = std::list<ir_instruction>::reverse_iterator;
    
    ir_use_timeline (void)                                  = delete;
    ir_use_timeline (const ir_use_timeline&)                = delete;
    ir_use_timeline (ir_use_timeline&&) noexcept            = default;
    ir_use_timeline& operator= (const ir_use_timeline&)     = delete;
    ir_use_timeline& operator= (ir_use_timeline&&) noexcept = default;
    ~ir_use_timeline (void)                                 = default;
    
    constexpr ir_use_timeline (ir_instruction& origin)
      : m_origin_instr (origin)
    { }
  
    [[nodiscard]]
    constexpr ir_def& get_def (void) noexcept
    {
      return get_instruction ().get_return ();
    }

    [[nodiscard]]
    constexpr const ir_def& get_def (void) const noexcept
    {
      return get_instruction ().get_return ();
    }

    [[nodiscard]]
    constexpr ir_instruction& get_instruction (void) noexcept
    {
      return *m_origin_instr;
    }

    [[nodiscard]]
    constexpr const ir_instruction& get_instruction (void) const noexcept
    {
      return *m_origin_instr;
    }
    
    ir_use_timeline split (instr_riter rpivot, instr_riter rfirst, instr_riter rlast);
  
    riter find_latest_before (instr_riter rpos, instr_riter rfirst, instr_riter rlast);

  private:
    nonnull_ptr<ir_instruction> m_origin_instr;
  };

  class ir_basic_block : public ir_component
  {
  public:

    using instr_list      = std::list<ir_instruction>;
    using instr_iter      = instr_list::iterator;
    using instr_citer     = instr_list::const_iterator;
    using instr_riter     = std::reverse_iterator<instr_iter>;
    using instr_criter    = std::reverse_iterator<instr_citer>;
    using instr_ref       = instr_list::reference;
    using instr_cref      = instr_list::const_reference;

    class def_timeline
      : public intrusive_tracker<def_timeline, remote::standalone_tracker>
    {
    public:
      
      class phi_node
      {
      public:
  
        // the bool indicates whether the incoming def is indeterminate
        using incoming_tuple = std::tuple<nonnull_ptr<def_timeline>, ir_use_timeline, bool>;
        using incoming_list  = std::list<incoming_tuple>;
        using iter           = incoming_list::iterator;
        using citer          = incoming_list::const_iterator;
        using riter          = incoming_list::reverse_iterator;
        using criter         = incoming_list::const_reverse_iterator;
        using ref            = incoming_list::reference;
        using cref           = incoming_list::const_reference;
        
    
        phi_node (void)                           = delete;
        phi_node (const phi_node&)                = delete;
        phi_node (phi_node&&) noexcept            = default;
        phi_node& operator= (const phi_node&)     = delete;
        phi_node& operator= (phi_node&&) noexcept = default;
        ~phi_node (void)                          = default;
      
        explicit phi_node (ir_instruction& phi_instr)
          : m_phi_timeline (phi_instr)
        { }
  
        [[nodiscard]] auto  begin   (void)       noexcept { return m_incoming.begin ();   }
        [[nodiscard]] auto  begin   (void) const noexcept { return m_incoming.begin ();   }
        [[nodiscard]] auto  cbegin  (void) const noexcept { return m_incoming.cbegin ();  }
  
        [[nodiscard]] auto  end     (void)       noexcept { return m_incoming.end ();     }
        [[nodiscard]] auto  end     (void) const noexcept { return m_incoming.end ();     }
        [[nodiscard]] auto  cend    (void) const noexcept { return m_incoming.cend ();    }
  
        [[nodiscard]] auto  rbegin  (void)       noexcept { return m_incoming.rbegin ();  }
        [[nodiscard]] auto  rbegin  (void) const noexcept { return m_incoming.rbegin ();  }
        [[nodiscard]] auto  crbegin (void) const noexcept { return m_incoming.crbegin (); }
  
        [[nodiscard]] auto  rend    (void)       noexcept { return m_incoming.rend ();    }
        [[nodiscard]] auto  rend    (void) const noexcept { return m_incoming.rend ();    }
        [[nodiscard]] auto  crend   (void) const noexcept { return m_incoming.crend ();   }
  
        [[nodiscard]] auto& front   (void)       noexcept { return m_incoming.front ();   }
        [[nodiscard]] auto& front   (void) const noexcept { return m_incoming.front ();   }
  
        [[nodiscard]] auto& back    (void)       noexcept { return m_incoming.back ();    }
        [[nodiscard]] auto& back    (void) const noexcept { return m_incoming.back ();    }
    
        void append (def_timeline& tl, ir_instruction& origin, bool is_indet)
        {
           m_incoming.emplace_back (tl, origin, is_indet);
           m_num_indet += static_cast<std::size_t> (is_indet);
        }
  
        iter repoint (nonnull_ptr<def_timeline> from, def_timeline& to);
    
        iter remove (const nonnull_ptr<def_timeline> dt);
        
        iter set_indeterminate (const nonnull_ptr<def_timeline> dt, bool indet = true);
    
        [[nodiscard]] std::size_t num_indet (void) const noexcept { return m_num_indet; }
    
        [[nodiscard]] bool has_indet (void) const noexcept { return m_num_indet != 0; }
  
        [[nodiscard]]
        constexpr ir_def& get_def (void) noexcept
        {
          return get_timeline ().get_def ();
        }
  
        [[nodiscard]]
        constexpr const ir_def& get_def (void) const noexcept
        {
          return get_timeline ().get_def ();
        }
  
        [[nodiscard]]
        constexpr ir_instruction& get_instruction (void) noexcept
        {
          return get_timeline ().get_instruction ();
        }
  
        [[nodiscard]]
        constexpr const ir_instruction& get_instruction (void) const noexcept
        {
          return get_timeline ().get_instruction ();
        }
  
        [[nodiscard]]
        constexpr ir_use_timeline& get_timeline (void) noexcept
        {
          return m_phi_timeline;
        }
  
        [[nodiscard]]
        constexpr const ir_use_timeline& get_timeline (void) const noexcept
        {
          return m_phi_timeline;
        }
        
        [[nodiscard]]
        iter find (def_timeline& dt)
        {
          return std::find_if (begin (), end (),
                               [&dt](const auto& tup)
                               {
                                 return dt == std::get<nonnull_ptr<def_timeline>> (tup);
                               });
        }
  
        [[nodiscard]]
        citer find (const def_timeline& dt) const
        {
          return const_cast<phi_node *> (this)->find (const_cast<def_timeline&> (dt));
        }
  
      private:
    
        // blocks where the variable was undefined
        std::list<incoming_tuple> m_incoming;
        std::size_t               m_num_indet;
        ir_use_timeline           m_phi_timeline;
      };
      
      using use_timeline_list = std::list<ir_use_timeline>;
      using iter              = use_timeline_list::iterator;
      using citer             = use_timeline_list::const_iterator;
      using riter             = use_timeline_list::reverse_iterator;
      using criter            = use_timeline_list::const_reverse_iterator;
      using ref               = ir_use_timeline&;
      using cref              = const ir_use_timeline&;
      
  
      using pred_tracker = intrusive_tracker<def_timeline, remote::standalone_tracker>;
      using succ_tracker = standalone_tracker<remote::intrusive_tracker<def_timeline>>;
  
      using pred_iter  = pred_tracker::iter;
      using pred_citer = pred_tracker::citer;
  
      using succ_iter  = succ_tracker::iter;
      using succ_citer = succ_tracker::citer;

      def_timeline (void)                               = delete;
      def_timeline (const def_timeline&)                = delete;
      def_timeline (def_timeline&&) noexcept;
      def_timeline& operator= (const def_timeline&)     = delete;
      def_timeline& operator= (def_timeline&&) noexcept;
      ~def_timeline (void)                              = default;

      explicit def_timeline (ir_basic_block& blk) noexcept
        : m_block (blk)
      { }
  
      def_timeline (ir_basic_block& block, succ_tracker& remote_tr)
      : pred_tracker (tag::bind, { remote_tr }),
        m_block (block)
      { }
      
      [[nodiscard]]  iter  local_begin  (void)       noexcept { return m_timelines.begin ();   }
      [[nodiscard]] citer  local_begin  (void) const noexcept { return m_timelines.begin ();   }
      [[nodiscard]] citer  local_cbegin (void) const noexcept { return m_timelines.cbegin ();  }

      [[nodiscard]]  iter  local_end  (void)       noexcept { return m_timelines.end ();     }
      [[nodiscard]] citer  local_end  (void) const noexcept { return m_timelines.end ();     }
      [[nodiscard]] citer  local_cend (void) const noexcept { return m_timelines.cend ();    }

      [[nodiscard]]  riter local_rbegin  (void)       noexcept { return m_timelines.rbegin ();  }
      [[nodiscard]] criter local_rbegin  (void) const noexcept { return m_timelines.rbegin ();  }
      [[nodiscard]] criter local_crbegin (void) const noexcept { return m_timelines.crbegin (); }

      [[nodiscard]]  riter local_rend  (void)       noexcept { return m_timelines.rend ();     }
      [[nodiscard]] criter local_rend  (void) const noexcept { return m_timelines.rend ();     }
      [[nodiscard]] criter local_crend (void) const noexcept { return m_timelines.crend ();    }
  
      [[nodiscard]]  ref local_front   (void)       noexcept { return m_timelines.front ();   }
      [[nodiscard]] cref local_front   (void) const noexcept { return m_timelines.front ();   }
  
      [[nodiscard]]  ref local_back    (void)       noexcept { return m_timelines.back ();    }
      [[nodiscard]] cref local_back    (void) const noexcept { return m_timelines.back ();    }
  
      [[nodiscard]] bool local_empty (void) const noexcept { return m_timelines.empty (); }
      [[nodiscard]] std::size_t local_size (void) const noexcept { return m_timelines.size (); }
      
      [[nodiscard]] bool has_timelines (void) const noexcept { return ! m_timelines.empty (); }

      void splice (iter pos, def_timeline& other, iter first, iter last)
      {
        m_timelines.splice (pos, other.m_timelines, first, last);
      }
      
      def_timeline split (instr_riter rpivot, ir_basic_block& dest);

      iter emplace_before (citer pos, ir_def& d)
      {
        return m_timelines.emplace (pos, d.get_instruction ());
      }

      ir_use_timeline& emplace_front (ir_def& d)
      {
        return m_timelines.emplace_front (d.get_instruction ());
      }

      ir_use_timeline& emplace_back (ir_def& d)
      {
        return m_timelines.emplace_back (d.get_instruction ());
      }
  
      iter emplace_before (citer pos, ir_instruction& instr)
      {
        return m_timelines.emplace (pos, instr);
      }
  
      ir_use_timeline& emplace_back (ir_instruction& instr)
      {
        return m_timelines.emplace_back (instr);
      }
  
      iter erase (citer pos)
      {
        return m_timelines.erase (pos);
      }
  
      iter erase (citer first, citer last)
      {
        return m_timelines.erase (first, last);
      }

      void erase (const ir_instruction& instr)
      {
        m_timelines.erase (find (instr));
      }

      [[nodiscard]] citer find (const ir_instruction& instr_cit) const;

      [[nodiscard]]
      std::size_t num_defs (void) const noexcept
      {
        return m_timelines.size ();
      }

      [[nodiscard]] optional_ref<ir_def> get_latest (void);
      
      [[nodiscard]] optional_ref<const ir_def> get_latest (void) const
      {
        return const_cast<def_timeline *> (this)->get_latest ();
      }
  
      [[nodiscard]]
      riter find_latest_before (instr_citer pos);
  
      [[nodiscard]]
      criter find_latest_before (instr_citer pos) const
      {
        return const_cast<def_timeline *> (this)->find_latest_before (pos);
      }
      
      [[nodiscard]]
      optional_ref<ir_use_timeline> get_latest_timeline_before (instr_citer pos);
  
      [[nodiscard]]
      optional_ref<ir_use_timeline> get_latest_timeline_before (instr_citer pos) const
      {
        return const_cast<def_timeline *> (this)->get_latest_timeline_before (pos);
      }
      
      [[nodiscard]]
      optional_ref<ir_def> get_latest_before (instr_citer pos)
      {
        if (optional_ref<ir_use_timeline> tl = get_latest_timeline_before (pos))
          return tl->get_def ();
        return nullopt;
      }

      [[nodiscard]]
      constexpr ir_basic_block& get_block (void) const noexcept
      {
        return *m_block;
      }
  
      def_timeline create_successor (ir_basic_block& block)
      {
        return { *m_block, m_succs };
      }
  
      succ_iter add_successor (def_timeline& d)
      {
        return m_succs.bind (d);
      }
  
      void remove_successor (def_timeline& d)
      {
        m_succs.debind (d);
      }
  
      pred_iter add_predecessor (def_timeline& d)
      {
        return pred_tracker::bind (d.m_succs);
      }
  
      void remove_predecessor (def_timeline& d)
      {
        pred_tracker::debind (d.m_succs);
      }
      
      [[nodiscard]]
      constexpr bool has_phi (void) const noexcept
      {
        return std::holds_alternative<phi_node> (m_head);
      }
      
      [[nodiscard]]
      constexpr ir_use_timeline& get_phi (void) noexcept
      {
        if (! has_phi ())
          throw ir_exception ("requested a nonexistent phi node");
        return std::get_if<phi_node> (&m_head)->get_timeline ();
      }
  
      [[nodiscard]]
      constexpr bool has_single_incoming (void) const noexcept
      {
        return std::holds_alternative<ir_use_timeline> (m_head);
      }
  
      [[nodiscard]]
      constexpr ir_use_timeline& get_single_incoming (void) noexcept
      {
        if (! has_single_incoming ())
          throw ir_exception ("def_timeline does not have exactly one incoming");
        return *std::get_if<ir_use_timeline> (&m_head);
      }
  
      [[nodiscard]]
      constexpr bool has_incoming (void) const noexcept
      {
        return ! std::holds_alternative<std::monostate> (m_head);
      }

    private:

      nonnull_ptr<ir_basic_block> m_block;
      
      // either a phi pair or an incoming timeline
      std::variant<std::monostate, phi_node, ir_use_timeline> m_head;

      //! A timeline of defs in this block.
      use_timeline_list m_timelines;
      succ_tracker m_succs;
    };

  private:

    using def_timeline_map = std::unordered_map<ir_variable *, def_timeline>;

  public:

    ir_basic_block            (void)                      = delete;
    ir_basic_block            (const ir_basic_block&)     = delete;
    ir_basic_block            (ir_basic_block&&) noexcept = default;
    ir_basic_block& operator= (const ir_basic_block&)     = delete;
    ir_basic_block& operator= (ir_basic_block&&) noexcept = delete;
    ~ir_basic_block           (void) noexcept override;

    explicit ir_basic_block (ir_structure& parent);
    
    ir_basic_block& split_into (instr_iter pivot, ir_basic_block& dest);
  
    [[nodiscard]]
    optional_ref<ir_def> get_latest_def (ir_variable& var);
    
    [[nodiscard]]
    optional_ref<ir_def> get_latest_def_before (instr_citer pos, ir_variable& var);
    
    optional_ref<ir_def> join_incoming_defs (ir_variable& var);

  private:
  
    ir_def& create_def (instr_citer pos, ir_variable& var);

  public:

    void set_cached_def (ir_def& d);
    void track_phi_def (ir_def& d);

    // all
  
    [[nodiscard]] instr_iter   begin (void)         noexcept { return m_instr_partition.data_begin ();   }
    [[nodiscard]] instr_citer  begin (void)   const noexcept { return m_instr_partition.data_cbegin ();   }
    [[nodiscard]] instr_citer  cbegin (void)  const noexcept { return m_instr_partition.data_cbegin ();  }
  
    [[nodiscard]] instr_iter   end (void)           noexcept { return m_instr_partition.data_begin ();     }
    [[nodiscard]] instr_citer  end (void)     const noexcept { return m_instr_partition.data_cbegin ();     }
    [[nodiscard]] instr_citer  cend (void)    const noexcept { return m_instr_partition.data_cbegin ();    }
  
    [[nodiscard]] instr_riter  rbegin (void)        noexcept { return m_instr_partition.data_rbegin ();  }
    [[nodiscard]] instr_criter rbegin (void)  const noexcept { return m_instr_partition.data_crbegin ();  }
    [[nodiscard]] instr_criter crbegin (void) const noexcept { return m_instr_partition.data_crbegin (); }
  
    [[nodiscard]] instr_riter  rend (void)          noexcept { return m_instr_partition.data_rend ();    }
    [[nodiscard]] instr_criter rend (void)    const noexcept { return m_instr_partition.data_crend ();    }
    [[nodiscard]] instr_criter crend (void)   const noexcept { return m_instr_partition.data_crend ();   }
  
    [[nodiscard]] instr_ref    front (void)         noexcept { return m_instr_partition.data_front ();   }
    [[nodiscard]] instr_cref   front (void)   const noexcept { return m_instr_partition.data_front ();   }
  
    [[nodiscard]] instr_ref    back (void)          noexcept { return m_instr_partition.data_back ();    }
    [[nodiscard]] instr_cref   back (void)    const noexcept { return m_instr_partition.data_back ();    }
  
    [[nodiscard]] size_t       size (void)    const noexcept { return m_instr_partition.data_size ();    }
    [[nodiscard]] bool         empty (void)   const noexcept { return m_instr_partition.data_empty ();   }
  
    [[nodiscard]] constexpr auto& get_phi_range (void)       noexcept { return get_subrange<0> (m_instr_partition); }
    [[nodiscard]] constexpr auto& get_phi_range (void) const noexcept { return get_subrange<0> (m_instr_partition); }
    
    [[nodiscard]] instr_iter   phi_begin   (void)       noexcept { return get_phi_range ().begin ();   }
    [[nodiscard]] instr_citer  phi_begin   (void) const noexcept { return get_phi_range ().begin ();   }
    [[nodiscard]] instr_citer  phi_cbegin  (void) const noexcept { return get_phi_range ().cbegin ();  }
    
    [[nodiscard]] instr_iter   phi_end     (void)       noexcept { return get_phi_range ().end ();     }
    [[nodiscard]] instr_citer  phi_end     (void) const noexcept { return get_phi_range ().end ();     }
    [[nodiscard]] instr_citer  phi_cend    (void) const noexcept { return get_phi_range ().cend ();    }
    
    [[nodiscard]] instr_riter  phi_rbegin  (void)       noexcept { return get_phi_range ().rbegin ();  }
    [[nodiscard]] instr_criter phi_rbegin  (void) const noexcept { return get_phi_range ().rbegin ();  }
    [[nodiscard]] instr_criter phi_crbegin (void) const noexcept { return get_phi_range ().crbegin (); }
    
    [[nodiscard]] instr_riter  phi_rend    (void)       noexcept { return get_phi_range ().rend ();    }
    [[nodiscard]] instr_criter phi_rend    (void) const noexcept { return get_phi_range ().rend ();    }
    [[nodiscard]] instr_criter phi_crend   (void) const noexcept { return get_phi_range ().crend ();   }
    
    [[nodiscard]] instr_ref    phi_front   (void)       noexcept { return get_phi_range ().front ();   }
    [[nodiscard]] instr_cref   phi_front   (void) const noexcept { return get_phi_range ().front ();   }
    
    [[nodiscard]] instr_ref    phi_back    (void)       noexcept { return get_phi_range ().back ();    }
    [[nodiscard]] instr_cref   phi_back    (void) const noexcept { return get_phi_range ().back ();    }
  
    [[nodiscard]] size_t       phi_size    (void)    const noexcept { return get_phi_range ().size (); }
    [[nodiscard]] bool         phi_empty   (void)   const noexcept { return get_phi_range ().empty (); }
    
    [[nodiscard]] constexpr auto& get_body (void)       noexcept { return get_subrange<1> (m_instr_partition); }
    [[nodiscard]] constexpr auto& get_body (void) const noexcept { return get_subrange<1> (m_instr_partition); }
    
    [[nodiscard]] instr_iter   body_begin   (void)       noexcept { return get_body ().begin ();   }
    [[nodiscard]] instr_citer  body_begin   (void) const noexcept { return get_body ().begin ();   }
    [[nodiscard]] instr_citer  body_cbegin  (void) const noexcept { return get_body ().cbegin ();  }
    
    [[nodiscard]] instr_iter   body_end     (void)       noexcept { return get_body ().end ();     }
    [[nodiscard]] instr_citer  body_end     (void) const noexcept { return get_body ().end ();     }
    [[nodiscard]] instr_citer  body_cend    (void) const noexcept { return get_body ().cend ();    }
    
    [[nodiscard]] instr_riter  body_rbegin  (void)       noexcept { return get_body ().rbegin ();  }
    [[nodiscard]] instr_criter body_rbegin  (void) const noexcept { return get_body ().rbegin ();  }
    [[nodiscard]] instr_criter body_crbegin (void) const noexcept { return get_body ().crbegin (); }
    
    [[nodiscard]] instr_riter  body_rend    (void)       noexcept { return get_body ().rend ();    }
    [[nodiscard]] instr_criter body_rend    (void) const noexcept { return get_body ().rend ();    }
    [[nodiscard]] instr_criter body_crend   (void) const noexcept { return get_body ().crend ();   }
    
    [[nodiscard]] instr_ref    body_front   (void)       noexcept { return get_body ().front ();   }
    [[nodiscard]] instr_cref   body_front   (void) const noexcept { return get_body ().front ();   }
    
    [[nodiscard]] instr_ref    body_back    (void)       noexcept { return get_body ().back ();    }
    [[nodiscard]] instr_cref   body_back    (void) const noexcept { return get_body ().back ();    }
  
    [[nodiscard]] size_t       body_size    (void)    const noexcept { return get_body ().size (); }
    [[nodiscard]] bool         body_empty   (void)   const noexcept { return get_body ().empty (); }

    template <typename ...Args>
    ir_phi& create_phi (ir_variable& var, Args&&... args)
    {
      std::unique_ptr<ir_phi> u = create_instruction<ir_phi> (var, std::forward<Args> (args)...);
      ir_phi& ret = *u;
      auto& ret_ptr = get_phi_range ().emplace_back (std::move (u));
      try
      {
        get_timeline (var).emplace_front (ret.get_def ());
      }
      catch (...)
      {
        get_phi_range ().pop_back ();
        throw;
      }
      return ret;
    }
    
    def_timeline::iter erase_phi (ir_variable& var)
    {
      def_timeline& dt = get_timeline (var);
      // erase use_timelines until we get to the defining use_timeline
      auto pos = std::find_if (dt.begin (), dt.end (),
                               [] (ir_use_timeline& ut) { return ut.is_def_block (); });
      
      // error checking
      if (pos == dt.end () || ! is_a<ir_opcode::phi> (pos->get_instruction ()))
        throw ir_exception ("tried to erase a nonexistent phi instruction");
      
      get_phi_range ().erase (std::find_if (get_phi_range ().begin (), get_phi_range ().end (),
                                            [pos] (auto&& uptr)
                                            {
                                              return uptr.get () == &pos->get_instruction ();
                                            }));
      return dt.erase (dt.begin (), ++pos);
    }
    
    ir_use_timeline& prepare_operand (instr_citer pos, ir_variable& var);
    
    template <typename T>
    constexpr T&& prepare_operand (const instr_citer&, T&& t)
    {
      return std::forward<T> (t);
    }
    
  
    template <ir_opcode Tag>
    static constexpr bool is_phi = ir_instruction::metadata::is_a<Tag, ir_opcode::phi> ();
  
    template <ir_opcode Tag>
    static constexpr bool has_return = ir_instruction::has_return_v<Tag>;
  
    // with return
    template <ir_opcode Tag, typename ...Args,
              std::enable_if_t<ir_instruction::has_return_v<Tag>>* = nullptr>
    ir_instruction& append_instruction (ir_variable& v, Args&&... args)
    {
      ir_instruction& instr = get_body ().emplace_back (v,
        prepare_operand (end (), std::forward<Args> (args))...);
      try
      {
        get_timeline (v).emplace_back (instr);
      }
      catch (...)
      {
        get_body ().pop_back ();
        throw;
      }
      return instr;
    }
  
    // no return
    template <ir_opcode Tag, typename ...Args,
      std::enable_if_t<! ir_instruction::has_return_v<Tag>>* = nullptr>
    ir_instruction& append_instruction (Args&&... args)
    {
      return get_body ().emplace_back (prepare_operand (end (), std::forward<Args> (args))...);
    }
  
    // with return (places instruction at the front of the body)
    template <ir_opcode Tag, typename ...Args,
      std::enable_if_t<ir_instruction::has_return_v<Tag>>* = nullptr>
    ir_instruction& prepend_instruction (ir_variable& v, Args&&... args)
    {
      ir_instruction& instr = get_body ().emplace_front (v,
        prepare_operand (begin (), std::forward<Args> (args))...);
      try
      {
        def_timeline& dt = get_timeline (v);
        dt.emplace_before(dt.find_latest_before (body_begin ()).base (), instr);
      }
      catch (...)
      {
        get_body ().pop_front ();
        throw;
      }
      return instr;
    }
  
    // no return (places instruction at the front of the body)
    template <ir_opcode Tag, typename ...Args,
      std::enable_if_t<! ir_instruction::has_return_v<Tag>>* = nullptr>
    ir_instruction& prepend_instruction (Args&&... args)
    {
      return get_body ().emplace_front (prepare_operand (begin (), std::forward<Args> (args))...);
    }
  
    // with return (places instruction immediately before `pos`)
    template <ir_opcode Tag, typename ...Args,
      std::enable_if_t<ir_instruction::has_return_v<Tag>>* = nullptr>
    ir_instruction& emplace_instruction (const instr_citer pos, ir_variable& v, Args&&... args)
    {
      instr_iter it = get_body ().emplace (v, prepare_operand (begin (),
                                                               std::forward<Args> (args))...);
      try
      {
        def_timeline& dt = get_timeline (v);
        dt.emplace_before(dt.find_latest_before (it).base (), *it);
      }
      catch (...)
      {
        get_body ().erase (it);
        throw;
      }
      return *it;
    }
  
    // no return (places instruction immediately before `pos`)
    template <ir_opcode Tag, typename ...Args,
      std::enable_if_t<! ir_instruction::has_return_v<Tag>>* = nullptr>
    ir_instruction& emplace_instruction (const instr_citer pos, Args&&... args)
    {
      return *get_body ().emplace(pos, prepare_operand (pos, std::forward<Args> (args))...);
    }
    
    instr_iter erase (instr_citer pos);
    instr_iter erase (instr_citer first, instr_citer last) noexcept;

    // predecessors

    link_iter   pred_begin (void);
    link_iter   pred_end   (void);
    std::size_t num_preds  (void);

    nonnull_ptr<ir_basic_block> pred_front (void) { return *pred_begin ();   }
    nonnull_ptr<ir_basic_block> pred_back  (void) { return *(--pred_end ()); }

    [[nodiscard]]
    bool has_preds (void) { return pred_begin () != pred_end (); }

    [[nodiscard]]
    bool has_preds (instr_citer pos) { return pos != get_body ().begin () || has_preds (); }

    [[nodiscard]]
    bool has_multiple_preds (void) { return num_preds () > 1; }

    // successors
    link_iter   succ_begin (void);
    link_iter   succ_end   (void);
    std::size_t num_succs  (void);

    nonnull_ptr<ir_basic_block> succ_front (void) { return *succ_begin ();   }
    nonnull_ptr<ir_basic_block> succ_back  (void) { return *(--succ_end ()); }

    bool has_succs (void)            { return succ_begin () != succ_end (); }
    bool has_succs (instr_citer pos) { return pos != get_body ().end () || has_succs (); }

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

    const ir_function& get_function (void) const noexcept override;

    void reset (void) noexcept override;

    /**
     * Create a def before the specified position.
     *
     * @param var the variable the def will be associated with.
     * @param pos an iterator immediately the position of the def.
     * @return the created def.
     */
    auto create_def_before (ir_variable& var, instr_citer pos);

  protected:

    void def_emplace_before (instr_citer pos, ir_def& d);
    void def_emplace_front  (ir_def& d);
    void def_emplace_back   (ir_def& d);

    void def_emplace_before (instr_citer pos, ir_def_instruction& d);
    void def_emplace_front  (ir_def_instruction& d);
    void def_emplace_back   (ir_def_instruction& d);
    
    optional_ref<const def_timeline> find_timeline (ir_variable& var) const
    {
      if (auto cit = m_timeline_map.find (&var); cit != m_timeline_map.end ())
        return std::get<def_timeline> (*cit);
      return nullopt;
    }

    optional_ref<def_timeline> find_timeline (ir_variable& var)
    {
      if (auto cit = m_timeline_map.find (&var); cit != m_timeline_map.end ())
        return std::get<def_timeline> (*cit);
      return nullopt;
    }
    
    def_timeline& get_timeline (ir_variable& var)
    {
      return std::get<def_timeline> (*m_timeline_map.try_emplace (&var, *this).first);
    }

  private:

    template <typename T, typename ...Args>
    ir_instruction create_instruction (Args&&... args)
    {
      return ir_instruction::create<T> (*this, std::forward<Args> (args)...);
    }

    ir_structure& m_parent;
    
    list_partition<ir_instruction, 2> m_instr_partition;

    // map of variables to the ir_def timeline for this block

    // predecessors and successors to this block

    // map from ir_variable to ir_def-timeline structs
    def_timeline_map m_timeline_map;

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

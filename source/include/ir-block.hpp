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

#include <tracker.hpp>
#include <optional_ref.hpp>
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
  class ir_phi;
  class ir_structure;
  class ir_instruction;
  class ir_def_instruction;

  class ir_variable;
  class ir_def;
  class ir_use;

  class ir_use_timeline
    : public intrusive_reporter<ir_use_timeline, remote::intrusive_tracker<ir_def>>,
      public intrusive_tracker<ir_use_timeline, remote::intrusive_reporter<ir_use>>
  {

  public:
  
    using base_reporter = intrusive_reporter<ir_use_timeline, remote::intrusive_tracker<ir_def>>;
    using base_tracker  = intrusive_tracker<ir_use_timeline, remote::intrusive_reporter<ir_use>>;

    ir_use_timeline (void)                    = delete;
    ir_use_timeline (const ir_use_timeline&)     = delete;
    ir_use_timeline (ir_use_timeline&&) noexcept = delete;
    ir_use_timeline& operator= (const ir_use_timeline&)     = delete;
    ir_use_timeline& operator= (ir_use_timeline&&) noexcept;
    ~ir_use_timeline (void)                    = default;

    explicit ir_use_timeline (ir_basic_block& block);
    ir_use_timeline (ir_basic_block& block, ir_def& def);
    ir_use_timeline (ir_use_timeline&& other, ir_basic_block& block) noexcept;

    void reset (void)
    {
      base_tracker::clear ();
    }

    void rebind (ir_def& d);
  
    [[nodiscard]]
    constexpr ir_def& get_def (void) noexcept { return base_reporter::get_remote (); }

    [[nodiscard]]
    constexpr const ir_def& get_def (void) const noexcept { return base_reporter::get_remote (); }

    [[nodiscard]]
    ir_instruction& get_instruction (void) noexcept;

    [[nodiscard]]
    const ir_instruction& get_instruction (void) const noexcept;
    
    [[nodiscard]]
    constexpr bool is_def_block (void) const noexcept
    {
      return m_block == &get_def ().get_block ();
    }
  
    void propagate_type (ir_type ty);

  private:

    nonnull_ptr<ir_basic_block>               m_block;
    std::vector<nonnull_ptr<ir_use_timeline>> m_succs;

  };

  class ir_basic_block : public ir_component
  {
  public:
  
    using phi_list        = std::list<ir_phi>;
    using phi_iter        = phi_list::iterator;
    using phi_citer       = phi_list::const_iterator;
    using phi_riter       = phi_list::reverse_iterator;
    using phi_criter      = phi_list::const_reverse_iterator;
    using phi_ref         = phi_list::reference;
    using phi_cref        = phi_list::const_reference;

    using instr_list      = std::list<std::unique_ptr<ir_instruction>>;
    using instr_iter      = instr_list::iterator;
    using instr_citer     = instr_list::const_iterator;
    using instr_riter     = std::reverse_iterator<instr_iter>;
    using instr_criter    = std::reverse_iterator<instr_citer>;
    using instr_ref       = instr_list::reference;
    using instr_cref      = instr_list::const_reference;

    class def_timeline
    {

    public:
      // never nullptr ----------------------------- v
      using use_timelines = std::list<ir_use_timeline>;
      using use_tl_iter         = use_timelines::iterator;
      using use_tl_citer        = use_timelines::const_iterator;
      using use_tl_riter        = use_timelines::reverse_iterator;
      using use_tl_criter       = use_timelines::const_reverse_iterator;

      def_timeline (void)                         = delete;
      def_timeline (const def_timeline&)     = delete;
      def_timeline (def_timeline&&) noexcept = delete;
      def_timeline& operator= (const def_timeline&)     = delete;
      def_timeline& operator= (def_timeline&&) noexcept;
      ~def_timeline (void)                             = default;

      explicit def_timeline (ir_basic_block& blk) noexcept
        : m_block (blk)
      { }

      [[nodiscard]] use_tl_iter   begin (void)       noexcept { return m_instances.begin ();   }
      [[nodiscard]] use_tl_citer  begin (void) const noexcept { return m_instances.begin ();   }
      [[nodiscard]] use_tl_citer  cbegin (void) const noexcept { return m_instances.cbegin ();  }

      [[nodiscard]] use_tl_iter   end (void)       noexcept { return m_instances.end ();     }
      [[nodiscard]] use_tl_citer  end (void) const noexcept { return m_instances.end ();     }
      [[nodiscard]] use_tl_citer  cend (void) const noexcept { return m_instances.cend ();    }

      [[nodiscard]] use_tl_riter  rbegin (void)       noexcept { return m_instances.rbegin ();  }
      [[nodiscard]] use_tl_criter rbegin (void) const noexcept { return m_instances.rbegin ();  }
      [[nodiscard]] use_tl_criter crbegin (void) const noexcept { return m_instances.crbegin (); }

      [[nodiscard]] use_tl_riter  rend (void)       noexcept { return m_instances.rend ();     }
      [[nodiscard]] use_tl_criter rend (void) const noexcept { return m_instances.rend ();     }
      [[nodiscard]] use_tl_criter crend (void) const noexcept { return m_instances.crend ();    }

      [[nodiscard]] bool has_use_timelines (void) const noexcept { return ! m_instances.empty (); }

      void splice (const use_tl_iter pos, def_timeline& other, const use_tl_iter first, const use_tl_iter last)
      {
        auto pivot = m_instances.emplace (pos, std::move (*first), *m_block);
        try
        {
          std::for_each (std::make_move_iterator (std::next (first)),
                         std::make_move_iterator (last),
                         [this, pos] (ir_use_timeline&& dt)
                         {
                           m_instances.emplace (pos, std::move (dt), *m_block);
                         });
        }
        catch (...)
        {
          std::for_each (pivot, pos,
                         [curr = first, &other] (auto&& dt) mutable
                         {
                           curr->move_bindings (dt);
                         });
          throw;
        }
        other.m_instances.erase (first, last);
      }

      use_tl_iter emplace_before (use_tl_citer pos, ir_def& d)
      {
        return m_instances.emplace (pos, *m_block, d);
      }

      ir_use_timeline& emplace_front (ir_def& d)
      {
        return m_instances.emplace_front (*m_block, d);
      }

      ir_use_timeline& emplace_back (ir_def& d)
      {
        return m_instances.emplace_back (*m_block, d);
      }

      void erase (const ir_instruction* instr)
      {
        m_instances.erase (find (instr));
      }

      [[nodiscard]]
      use_tl_citer find (const ir_instruction *instr_cit) const;

      [[nodiscard]]
      std::size_t num_defs (void) const noexcept
      {
        return m_instances.size ();
      }

      [[nodiscard]] optional_ref<ir_def> get_latest (void) noexcept;
      [[nodiscard]] optional_ref<const ir_def> get_latest (void) const noexcept;
      
      use_tl_riter find_latest_before (instr_citer pos, instr_criter rfirst, instr_criter rend);
      
      use_tl_criter
      find_latest_before (instr_citer pos, instr_criter rfirst, instr_criter rend) const
      {
        return const_cast<def_timeline *> (this)->find_latest_before (pos, rfirst, rend);
      }

      [[nodiscard]]
      constexpr ir_basic_block& get_block (void) const noexcept
      {
        return *m_block;
      }
      
      // [[nodiscard]]
      // constexpr bool has_phi (void) const noexcept { return m_phi.has_value (); }
      //
      // [[nodiscard]]
      // constexpr ir_use_timeline& get_phi (void) const noexcept { return *m_phi; }

    private:

      nonnull_ptr<ir_basic_block> m_block;

      //! A timeline of defs in this block.
      use_timelines m_instances;
      
      // optional_ref<ir_use_timeline> m_phi;

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

    ir_basic_block& split (instr_iter pivot, ir_basic_block& dest);
  
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
  
    [[nodiscard]] instr_iter   begin (void)         noexcept { return m_instructions.begin ();   }
    [[nodiscard]] instr_citer  begin (void)   const noexcept { return m_instructions.begin ();   }
    [[nodiscard]] instr_citer  cbegin (void)  const noexcept { return m_instructions.cbegin ();  }
  
    [[nodiscard]] instr_iter   end (void)           noexcept { return m_instructions.end ();     }
    [[nodiscard]] instr_citer  end (void)     const noexcept { return m_instructions.end ();     }
    [[nodiscard]] instr_citer  cend (void)    const noexcept { return m_instructions.cend ();    }
  
    [[nodiscard]] instr_riter  rbegin (void)        noexcept { return m_instructions.rbegin ();  }
    [[nodiscard]] instr_criter rbegin (void)  const noexcept { return m_instructions.rbegin ();  }
    [[nodiscard]] instr_criter crbegin (void) const noexcept { return m_instructions.crbegin (); }
  
    [[nodiscard]] instr_riter  rend (void)          noexcept { return m_instructions.rend ();    }
    [[nodiscard]] instr_criter rend (void)    const noexcept { return m_instructions.rend ();    }
    [[nodiscard]] instr_criter crend (void)   const noexcept { return m_instructions.crend ();   }
  
    [[nodiscard]] instr_ref    front (void)         noexcept { return m_instructions.front ();   }
    [[nodiscard]] instr_cref   front (void)   const noexcept { return m_instructions.front ();   }
  
    [[nodiscard]] instr_ref    back (void)          noexcept { return m_instructions.back ();    }
    [[nodiscard]] instr_cref   back (void)    const noexcept { return m_instructions.back ();    }
  
    [[nodiscard]] size_t       size (void)    const noexcept { return m_instructions.size ();    }
    [[nodiscard]] bool         empty (void)   const noexcept { return m_instructions.empty ();   }
    
    class subrange
    {
    public:
  
      using riter  = std::reverse_iterator<instr_iter>;
      using criter = std::reverse_iterator<instr_citer>;
    
      subrange            (void)                = default;
      subrange            (const subrange&)     = default;
      subrange            (subrange&&) noexcept = default;
      subrange& operator= (const subrange&)     = default;
      subrange& operator= (subrange&&) noexcept = default;
      ~subrange (void)                          = default;
    
      template <typename Functor1, typename Functor2>
      constexpr subrange (Functor1&& begin_func, Functor2&& end_func)
      noexcept
        : m_begin (std::forward<Functor1> (begin_func)),
          m_end   (std::forward<Functor2> (end_func))
      { }
    
      [[nodiscard]] auto  begin   (void)       noexcept { return m_begin (); }
      [[nodiscard]] auto  begin   (void) const noexcept { return instr_citer (m_begin ()); }
      [[nodiscard]] auto  cbegin  (void) const noexcept { return instr_citer (m_begin ()); }
    
      [[nodiscard]] auto  end     (void)       noexcept { return m_end (); }
      [[nodiscard]] auto  end     (void) const noexcept { return instr_citer (m_end ()); }
      [[nodiscard]] auto  cend    (void) const noexcept { return instr_citer (m_end ()); }
    
      [[nodiscard]] auto body_rbegin  (void)       noexcept { return riter (end ());  }
      [[nodiscard]] auto body_rbegin  (void) const noexcept { return criter (end ());  }
      [[nodiscard]] auto body_crbegin (void) const noexcept { return criter (cend ()); }
    
      [[nodiscard]] auto body_rend  (void)       noexcept { return riter (begin ()); }
      [[nodiscard]] auto body_rend  (void) const noexcept { return criter (begin ()); }
      [[nodiscard]] auto body_crend (void) const noexcept { return criter (cbegin ()); }
    
      [[nodiscard]] auto& front   (void)       noexcept { return *begin ();   }
      [[nodiscard]] auto& front   (void) const noexcept { return *begin ();   }
    
      [[nodiscard]] auto& back    (void)       noexcept { return *(--end ());    }
      [[nodiscard]] auto& back    (void) const noexcept { return *(--end ());    }
    
      [[nodiscard]] bool empty (void) const noexcept { return begin () == end (); }
  
    private:
    
      std::function<instr_iter()> m_begin;
      std::function<instr_iter()> m_end;
    };
  
    // phi
  
    [[nodiscard]] instr_iter  phi_begin  (void)       noexcept { return begin (); }
    [[nodiscard]] instr_citer phi_begin  (void) const noexcept { return begin (); }
    [[nodiscard]] instr_citer phi_cbegin (void) const noexcept { return cbegin (); }
  
    [[nodiscard]] instr_iter  phi_end  (void)       noexcept { return body_begin ();  }
    [[nodiscard]] instr_citer phi_end  (void) const noexcept { return body_begin ();  }
    [[nodiscard]] instr_citer phi_cend (void) const noexcept { return body_begin (); }
  
    [[nodiscard]] auto phi_rbegin  (void)       noexcept { return std::reverse_iterator (phi_end ());  }
    [[nodiscard]] auto phi_rbegin  (void) const noexcept { return std::reverse_iterator (phi_end ());  }
    [[nodiscard]] auto phi_crbegin (void) const noexcept { return std::reverse_iterator (phi_cend ()); }
  
    [[nodiscard]] auto phi_rend  (void)       noexcept { return std::reverse_iterator (phi_begin ()); }
    [[nodiscard]] auto phi_rend  (void) const noexcept { return std::reverse_iterator (phi_begin ()); }
    [[nodiscard]] auto phi_crend (void) const noexcept { return std::reverse_iterator (phi_cbegin ()); }
  
    [[nodiscard]] instr_ref  phi_front (void)       noexcept { return *phi_begin (); }
    [[nodiscard]] instr_cref phi_front (void) const noexcept { return *phi_begin (); }
  
    [[nodiscard]] instr_ref  phi_back (void)       noexcept { return *(--phi_end ());    }
    [[nodiscard]] instr_cref phi_back (void) const noexcept { return *(--phi_end ());    }
  
    [[nodiscard]] auto phi_size  (void) const noexcept { return std::distance (phi_begin (), phi_end ()); }
    [[nodiscard]] bool phi_empty (void) const noexcept { return phi_begin () == phi_end (); }

    // body
    
    [[nodiscard]] instr_iter  body_begin  (void)       noexcept { return m_body_begin; }
    [[nodiscard]] instr_citer body_begin  (void) const noexcept { return m_body_begin; }
    [[nodiscard]] instr_citer body_cbegin (void) const noexcept { return m_body_begin; }
  
    [[nodiscard]] instr_iter  body_end  (void)       noexcept { return end ();  }
    [[nodiscard]] instr_citer body_end  (void) const noexcept { return end ();  }
    [[nodiscard]] instr_citer body_cend (void) const noexcept { return cend (); }
  
    [[nodiscard]] auto body_rbegin  (void)       noexcept { return std::reverse_iterator (body_end ());  }
    [[nodiscard]] auto body_rbegin  (void) const noexcept { return std::reverse_iterator (body_end ());  }
    [[nodiscard]] auto body_crbegin (void) const noexcept { return std::reverse_iterator (body_cend ()); }
  
    [[nodiscard]] auto body_rend  (void)       noexcept { return std::reverse_iterator (body_begin ()); }
    [[nodiscard]] auto body_rend  (void) const noexcept { return std::reverse_iterator (body_begin ()); }
    [[nodiscard]] auto body_crend (void) const noexcept { return std::reverse_iterator (body_cbegin ()); }
  
    [[nodiscard]] instr_ref  body_front (void)       noexcept { return *body_begin (); }
    [[nodiscard]] instr_cref body_front (void) const noexcept { return *body_begin (); }
  
    [[nodiscard]] instr_ref  body_back (void)       noexcept { return *(--body_end ());    }
    [[nodiscard]] instr_cref body_back (void) const noexcept { return *(--body_end ());    }
  
    [[nodiscard]] auto body_size  (void) const noexcept { return size () - phi_size (); }
    [[nodiscard]] bool body_empty (void) const noexcept { return body_begin () == body_end (); }

    template <typename ...Args>
    ir_phi& create_phi (Args&&... args)
    {
      ir_phi& ret = m_phi_nodes.emplace_back (*this, std::forward<Args> (args)...);
      try
      {
        def_emplace_front (ret.get_def ());
      }
      catch (const std::exception& e)
      {
        m_phi_nodes.erase (--m_phi_nodes.end ());
        throw e;
      }
      return ret;
    }

    phi_iter erase_phi (phi_citer pos)
    {
      return m_phi_nodes.erase (pos);
    }

    template <typename T>
    using is_instruction = std::is_base_of<ir_instruction, T>;

    template <typename T>
    using is_phi = std::is_same<ir_phi, T>;

    template <typename T>
    using is_nonphi = std::conjunction<std::negation<is_phi<T>>, is_instruction<T>>;

    template <typename T>
    using has_return = std::is_base_of<ir_def_instruction, T>;

    template <typename T>
    using enable_ret_nonphi = std::enable_if_t<
      std::conjunction<is_nonphi<T>, has_return<T>>::value>;

    template <typename T>
    using enable_noret_nonphi = std::enable_if_t<std::conjunction<is_nonphi<T>,
      std::negation<has_return<T>>>::value>;

    template <typename T, typename ...Args, enable_ret_nonphi<T>* = nullptr>
    T& emplace_before (instr_citer pos, Args&&... args)
    {
      std::unique_ptr<T> u = create_instruction<T> (std::forward<Args> (args)...);
      T& ret = *u;
      auto it = m_instructions.emplace (pos, std::move (u));
      try
      {
        def_emplace_before (pos, ret.get_def ());
      }
      catch (const std::exception& e)
      {
        m_instructions.erase (it);
        throw e;
      }
      return ret;
    }

    template <typename T, typename ...Args, enable_ret_nonphi<T>* = nullptr>
    T& emplace_front (Args&&... args)
    {
      std::unique_ptr<T> u = create_instruction<T> (std::forward<Args> (args)...);
      T& ret = *u;
      m_instructions.emplace_front (std::move (u));
      try
      {
        def_emplace_front (ret.get_def ());
      }
      catch (const std::exception& e)
      {
        m_instructions.erase (m_instructions.begin ());
        throw e;
      }
      return ret;
    }

    template <typename T, typename ...Args, enable_ret_nonphi<T>* = nullptr>
    T& emplace_back (Args&&... args)
    {
      std::unique_ptr<T> u = create_instruction<T> (std::forward<Args> (args)...);
      T& ret = *u;
      m_instructions.emplace_back (std::move (u));
      try
      {
        def_emplace_back (ret.get_def ());
      }
      catch (const std::exception& e)
      {
        m_instructions.erase (--m_instructions.end ());
        throw e;
      }
      return ret;
    }

    template <typename T, typename ...Args, enable_noret_nonphi<T>* = nullptr>
    T& emplace_before (instr_citer pos, Args&&... args)
    {
      std::unique_ptr<T> u = create_instruction<T> (std::forward<Args> (args)...);
      T& ret = *u;
      m_instructions.emplace (pos, std::move (u));
      return ret;
    }

    template <typename T, typename ...Args, enable_noret_nonphi<T>* = nullptr>
    T& emplace_front (Args&&... args)
    {
      return emplace_before (m_instructions.begin (), std::forward<Args> (args)...);
    }

    template <typename T, typename ...Args, enable_noret_nonphi<T>* = nullptr>
    T& emplace_back (Args&&... args)
    {
      return emplace_before (m_instructions.end (), std::forward<Args> (args)...);
    }

    instr_iter erase (instr_citer pos);
    instr_iter erase (instr_citer first, instr_citer last) noexcept;

    // predecessors

    link_iter   pred_begin (void);
    link_iter   pred_end   (void);
    std::size_t num_preds  (void);

    nonnull_ptr<ir_basic_block> pred_front (void) { return *pred_begin ();   }
    nonnull_ptr<ir_basic_block> pred_back (void) { return *(--pred_end ()); }

    [[nodiscard]]
    bool has_preds (void) { return pred_begin () != pred_end (); }

    [[nodiscard]]
    bool has_preds (instr_citer pos) { return pos != body_begin () || has_preds (); }

    [[nodiscard]]
    bool has_multiple_preds (void) { return num_preds () > 1; }

    // successors
    link_iter   succ_begin (void);
    link_iter   succ_end   (void);
    std::size_t num_succs  (void);

    nonnull_ptr<ir_basic_block> succ_front (void) { return *succ_begin ();   }
    nonnull_ptr<ir_basic_block> succ_back  (void) { return *(--succ_end ()); }

    bool has_succs (void)            { return succ_begin () != succ_end (); }
    bool has_succs (instr_citer pos) { return pos != body_end () || has_succs (); }

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
    std::unique_ptr<T> create_instruction (Args&&... args)
    {
      return std::make_unique<T> (*this, std::forward<Args> (args)...);
    }

    ir_structure& m_parent;

    // list of instructions
    instr_list m_instructions;
    instr_iter m_body_begin;
    
    subrange m_phi_range;
    subrange m_body_range;
    
    std::size_t m_num_phi = 0;

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
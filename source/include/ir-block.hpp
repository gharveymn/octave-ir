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

  class def_timeline
    : public intrusive_tracker<def_timeline, remote::intrusive_reporter<ir_use>>
  {

  public:

    using base_tracker  = intrusive_tracker<def_timeline, remote::intrusive_reporter<ir_use>>;

    def_timeline            (void)                    = delete;
    def_timeline            (const def_timeline&)     = delete;
    def_timeline            (def_timeline&&) noexcept = delete;
    def_timeline& operator= (const def_timeline&)     = delete;
    def_timeline& operator= (def_timeline&&) noexcept;
    ~def_timeline           (void)                    = default;

    explicit def_timeline (ir_basic_block& block);
    def_timeline (ir_basic_block& block, ir_def& def);
    def_timeline (def_timeline&& other, ir_basic_block& block) noexcept;

    void reset (void)
    {
      base_tracker::clear ();
      m_def_link.debind ();
    }

    void rebind (ir_def& d);

    [[nodiscard]]
    constexpr bool has_def (void) const noexcept
    {
      return m_def_link.has_remote ();
    }

    [[nodiscard]]
    ir_def& get_def (void) const noexcept;

    [[nodiscard]]
    optional_ref<ir_def> get_maybe_def (void) const noexcept;

    [[nodiscard]]
    ir_instruction& get_instruction (void) noexcept;

    [[nodiscard]]
    const ir_instruction& get_instruction (void) const noexcept;

  private:

    reporter<ir_basic_block, remote::intrusive_tracker<ir_def>> m_def_link;

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

    class variable_timeline
    {

    public:
      // never nullptr ----------------------------- v
      using local_timelines = std::list<def_timeline>;
      using iter            = local_timelines::iterator;
      using citer           = local_timelines::const_iterator;
      using riter           = local_timelines::reverse_iterator;
      using criter          = local_timelines::const_reverse_iterator;

      variable_timeline            (void)                         = delete;
      variable_timeline            (const variable_timeline&)     = delete;
      variable_timeline            (variable_timeline&&) noexcept = delete;
      variable_timeline& operator= (const variable_timeline&)     = delete;
      variable_timeline& operator= (variable_timeline&&) noexcept;
      ~variable_timeline           (void)                         = default;

      explicit variable_timeline (ir_basic_block& blk) noexcept
        : m_block (blk),
          m_cache (blk),
          m_phi   (blk)
      { }

      [[nodiscard]] iter   begin   (void)       noexcept { return m_instances.begin ();   }
      [[nodiscard]] citer  begin   (void) const noexcept { return m_instances.begin ();   }
      [[nodiscard]] citer  cbegin  (void) const noexcept { return m_instances.cbegin ();  }

      [[nodiscard]] iter   end     (void)       noexcept { return m_instances.end ();     }
      [[nodiscard]] citer  end     (void) const noexcept { return m_instances.end ();     }
      [[nodiscard]] citer  cend    (void) const noexcept { return m_instances.cend ();    }

      [[nodiscard]] riter  rbegin  (void)       noexcept { return m_instances.rbegin ();  }
      [[nodiscard]] criter rbegin  (void) const noexcept { return m_instances.rbegin ();  }
      [[nodiscard]] criter crbegin (void) const noexcept { return m_instances.crbegin (); }

      [[nodiscard]] riter  rend   (void)       noexcept { return m_instances.rend ();     }
      [[nodiscard]] criter rend   (void) const noexcept { return m_instances.rend ();     }
      [[nodiscard]] criter crend  (void) const noexcept { return m_instances.crend ();    }

      [[nodiscard]] bool has_body_defs (void) const noexcept { return ! m_instances.empty (); }

      void splice (const iter pos, variable_timeline& other, const iter first, const iter last)
      {
        auto pivot = m_instances.emplace (pos, std::move (*first), *m_block);
        try
        {
          std::for_each (std::make_move_iterator (std::next (first)),
                         std::make_move_iterator (last),
                         [this, pos] (def_timeline&& dt)
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

      void emplace_before (citer pos, ir_def& d)
      {
        m_instances.emplace (pos, *m_block, d);
      }

      void emplace_front (ir_def& d)
      {
        m_instances.emplace_front (*m_block, d);
      }

      void emplace_back (ir_def& d)
      {
        m_instances.emplace_back (*m_block, d);
      }

      void erase (const ir_instruction* instr)
      {
        m_instances.erase (find (instr));
      }

      [[nodiscard]]
      citer find (const ir_instruction *instr_cit) const;

      [[nodiscard]]
      constexpr bool has_cache (void) const noexcept
      {
        return m_cache.has_def ();
      }

      [[nodiscard]]
      ir_def& get_cache (void) const noexcept;

      [[nodiscard]]
      optional_ref<ir_def> get_maybe_cache (void) const noexcept;

      void replace_cache (ir_def& latest) noexcept;

      void reset_cache (void) noexcept
      {
        m_cache.reset ();
      }

      [[nodiscard]]
      constexpr bool has_phi (void) const noexcept
      {
        return m_phi.has_def ();
      }

      [[nodiscard]]
      ir_def& get_phi (void) const noexcept;

      [[nodiscard]]
      optional_ref<ir_def> get_maybe_phi (void) const noexcept;

      void track_phi (ir_def& phi);

      void reset_phi (void)
      {
        m_phi.reset ();
      }

      [[nodiscard]]
      auto num_body_defs (void) const noexcept
      {
        return m_instances.size ();
      }

      [[nodiscard]] optional_ref<ir_def> get_latest (void) const noexcept;

      [[nodiscard]]
      constexpr ir_basic_block& get_block (void) const noexcept
      {
        return *m_block;
      }

    private:

      nonnull_ptr<ir_basic_block> m_block;

      def_timeline m_cache;

      def_timeline m_phi;

      //! A timeline of defs created in this block.
      local_timelines m_instances;

    };

  private:

    using var_timeline_map = std::unordered_map<ir_variable *, variable_timeline>;

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
    std::list<nonnull_ptr<ir_def>> get_latest_defs (ir_variable& var) noexcept override;
  
    [[nodiscard]]
    optional_ref<ir_def> get_latest_def (ir_variable& var) noexcept override;
  
    [[nodiscard]]
    optional_ref<ir_def> get_latest_def_before (ir_variable& var, instr_citer pos) const noexcept;

  private:

    variable_timeline::riter
    find_latest_def_before (instr_citer pos, variable_timeline& vt) const noexcept;

    variable_timeline::criter
    find_latest_def_before (instr_citer pos, const variable_timeline& vt) const noexcept;

  public:

    virtual gch::optional_ref<ir_def> join_preceding_defs (ir_variable& var);

    void set_cached_def (ir_def& d);
    void track_phi_def (ir_def& d);

    // all

    // front and back won't throw because the constructor will
    // always emplace a return instruction
    instr_iter   body_begin (void)         noexcept { return m_body.begin ();   }
    instr_citer  body_begin (void)   const noexcept { return m_body.begin ();   }
    instr_citer  body_cbegin (void)  const noexcept { return m_body.cbegin ();  }

    instr_iter   body_end (void)           noexcept { return m_body.end ();     }
    instr_citer  body_end (void)     const noexcept { return m_body.end ();     }
    instr_citer  body_cend (void)    const noexcept { return m_body.cend ();    }

    instr_riter  body_rbegin (void)        noexcept { return m_body.rbegin ();  }
    instr_criter body_rbegin (void)  const noexcept { return m_body.rbegin ();  }
    instr_criter body_crbegin (void) const noexcept { return m_body.crbegin (); }

    instr_riter  body_rend (void)          noexcept { return m_body.rend ();    }
    instr_criter body_rend (void)    const noexcept { return m_body.rend ();    }
    instr_criter body_crend (void)   const noexcept { return m_body.crend ();   }

    instr_ref    body_front (void)         noexcept { return m_body.front ();   }
    instr_cref   body_front (void)   const noexcept { return m_body.front ();   }

    instr_ref    body_back (void)          noexcept { return m_body.back ();    }
    instr_cref   body_back (void)    const noexcept { return m_body.back ();    }

    size_t       body_size (void)    const noexcept { return m_body.size ();    }
    bool         body_empty (void)   const noexcept { return m_body.empty ();   }

    // phi

    phi_iter   phi_begin (void)         noexcept { return m_phi_nodes.begin ();   }
    phi_citer  phi_begin (void)   const noexcept { return m_phi_nodes.begin ();   }
    phi_citer  phi_cbegin (void)  const noexcept { return m_phi_nodes.cbegin ();  }

    phi_iter   phi_end (void)           noexcept { return m_phi_nodes.end ();     }
    phi_citer  phi_end (void)     const noexcept { return m_phi_nodes.end ();     }
    phi_citer  phi_cend (void)    const noexcept { return m_phi_nodes.cend ();    }

    phi_riter  phi_rbegin (void)        noexcept { return m_phi_nodes.rbegin ();  }
    phi_criter phi_rbegin (void)  const noexcept { return m_phi_nodes.rbegin ();  }
    phi_criter phi_crbegin (void) const noexcept { return m_phi_nodes.crbegin (); }

    phi_riter  phi_rend (void)          noexcept { return m_phi_nodes.rend ();    }
    phi_criter phi_rend (void)    const noexcept { return m_phi_nodes.rend ();    }
    phi_criter phi_crend (void)   const noexcept { return m_phi_nodes.crend ();   }

    phi_ref    phi_front (void)         noexcept { return m_phi_nodes.front ();   }
    phi_cref   phi_front (void)   const noexcept { return m_phi_nodes.front ();   }

    phi_ref    phi_back (void)          noexcept { return m_phi_nodes.back ();    }
    phi_cref   phi_back (void)    const noexcept { return m_phi_nodes.back ();    }

    size_t     num_phi (void)     const noexcept { return m_phi_nodes.size ();    }
    bool       has_phi (void)     const noexcept { return m_phi_nodes.empty ();   }

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
      auto it = m_body.emplace (pos, std::move (u));
      try
      {
        def_emplace_before (pos, ret.get_def ());
      }
      catch (const std::exception& e)
      {
        m_body.erase (it);
        throw e;
      }
      return *ret;
    }

    template <typename T, typename ...Args, enable_ret_nonphi<T>* = nullptr>
    T& emplace_front (Args&&... args)
    {
      std::unique_ptr<T> u = create_instruction<T> (std::forward<Args> (args)...);
      T& ret = *u;
      m_body.emplace_front (std::move (u));
      try
      {
        def_emplace_front (ret.get_def ());
      }
      catch (const std::exception& e)
      {
        m_body.erase (m_body.begin ());
        throw e;
      }
      return *ret;
    }

    template <typename T, typename ...Args, enable_ret_nonphi<T>* = nullptr>
    T& emplace_back (Args&&... args)
    {
      std::unique_ptr<T> u = create_instruction<T> (std::forward<Args> (args)...);
      T& ret = *u;
      m_body.emplace_back (std::move (u));
      try
      {
        def_emplace_back (ret.get_def ());
      }
      catch (const std::exception& e)
      {
        m_body.erase (--m_body.end ());
        throw e;
      }
      return *ret;
    }

    template <typename T, typename ...Args, enable_noret_nonphi<T>* = nullptr>
    T& emplace_before (instr_citer pos, Args&&... args)
    {
      std::unique_ptr<T> u = create_instruction<T> (std::forward<Args> (args)...);
      T& ret = *u;
      m_body.emplace (pos, std::move (u));
      return ret;
    }

    template <typename T, typename ...Args, enable_noret_nonphi<T>* = nullptr>
    T& emplace_front (Args&&... args)
    {
      return emplace_before (m_body.begin (), std::forward<Args> (args)...);
    }

    template <typename T, typename ...Args, enable_noret_nonphi<T>* = nullptr>
    T& emplace_back (Args&&... args)
    {
      return emplace_before (m_body.end (), std::forward<Args> (args)...);
    }

    instr_iter erase (instr_citer pos);
    instr_iter erase (instr_citer first, instr_citer last) noexcept;

    // predecessors

    link_iter   pred_begin (void);
    link_iter   pred_end   (void);
    std::size_t num_preds  (void);

    nonnull_ptr<ir_basic_block> get_pred_front (void) { return *pred_begin ();   }
    nonnull_ptr<ir_basic_block> get_pred_back  (void) { return *(--pred_end ()); }

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
    
    optional_ref<const variable_timeline> find_timeline (ir_variable& var) const
    {
      if (auto cit = m_timeline_map.find (&var); cit != m_timeline_map.end ())
        return std::get<variable_timeline> (*cit);
      return nullopt;
    }

    optional_ref<variable_timeline> find_timeline (ir_variable& var)
    {
      if (auto cit = m_timeline_map.find (&var); cit != m_timeline_map.end ())
        return std::get<variable_timeline> (*cit);
      return nullopt;
    }
    
    variable_timeline& get_timeline (ir_variable& var)
    {
      return std::get<variable_timeline> (*m_timeline_map.try_emplace (&var, *this).first);
    }

  private:

    template <typename T, typename ...Args>
    std::unique_ptr<T> create_instruction (Args&&... args)
    {
      return std::make_unique<T> (*this, std::forward<Args> (args)...);
    }

    ir_structure& m_parent;

    phi_list m_phi_nodes;

    // list of instructions
    instr_list m_body;

    std::unique_ptr<ir_instruction> m_terminator;

    // map of variables to the ir_def timeline for this block

    // predecessors and successors to this block

    // map from ir_variable to ir_def-timeline structs
    var_timeline_map m_timeline_map;

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

  template <>
  struct ir_type::instance<ir_basic_block *>
  {
    using type = ir_basic_block;
    static constexpr
    impl m_impl = create_type<type> ("block_ptr");
  };
}

#endif

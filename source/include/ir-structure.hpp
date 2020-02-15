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

#if ! defined (ir_structure_h)
#define ir_structure_h 1

#include "ir-component.hpp"
#include "ir-block.hpp"
#include "ir-variable.hpp"

#include <algorithm>
#include <memory>
#include <list>
#include <stack>

namespace gch
{

  class ir_basic_block;
  class ir_condition_block;
  class ir_loop_condition_block;
  class ir_function;

  class ir_structure : public ir_component
  {
  public:
    ir_structure (void) noexcept = default;

    ~ir_structure (void) noexcept override = 0;
    
    virtual link_iter pred_begin (ir_component& c) = 0;
    virtual link_iter pred_end   (ir_component& c) = 0;
    virtual link_iter succ_begin (ir_component& c) = 0;
    virtual link_iter succ_end   (ir_component& c) = 0;
    
    virtual ir_basic_block& split (ir_basic_block& blk, instr_iter pivot) = 0;
  
    ir_basic_block& pred_front (ir_component& c) { return **pred_begin (c); }
    ir_basic_block& pred_back  (ir_component& c) { return **(--pred_end (c)); }
    ir_basic_block& succ_front (ir_component& c) { return **succ_begin (c); }
    ir_basic_block& succ_back  (ir_component& c) { return **(--succ_end (c)); }

    link_iter leaf_begin (void) override;
    link_iter leaf_end   (void) override;

    virtual void invalidate_leaf_cache (void) noexcept = 0;
  
  
    [[nodiscard]]
    virtual std::list<nonnull_ptr<ir_def>> get_latest_defs_before (ir_variable& v,
                                                                    component_handle c) = 0;
    
    [[nodiscard]]
    std::list<nonnull_ptr<ir_def>> get_latest_defs_before (ir_variable& var, component_handle comp,
                                                           instr_citer pos)
    {
      if (auto *block = dynamic_cast<ir_basic_block *> (comp->get ()))
      {
        if (auto ret = block->get_latest_def_before (var, pos))
          return { *ret };
        return get_latest_defs_before (var, comp);
      }
      throw ir_exception ("component was now a basic block");
    }

  protected:

    void clear_leaf_cache (void) noexcept
    {
      m_leaf_cache.clear ();
    }

    [[nodiscard]]
    constexpr bool leaf_cache_empty (void) const noexcept
    {
      return m_leaf_cache.empty ();
    }

    virtual bool is_leaf_component   (ir_component *c) noexcept = 0;
    virtual void generate_leaf_cache (void)                     = 0;

#if 0
    class leaf_iterator
    {
    public:
      using sub_iter = link_cache_iter;
      using sub_citer = link_cache_citer;

      using iterator_category = std::forward_iterator_tag;
      using value_type = sub_iter::value_type;
      using difference_type = sub_iter::difference_type;
      using pointer = sub_iter::pointer;
      using reference = sub_iter::reference;

    private:

      using stack_value_type = std::pair<sub_iter, sub_iter>;
      using stack_type       = std::stack<stack_value_type>;
      using stack_ref        = stack_type::reference;
      using stack_cref       = stack_type::const_reference;

    public:

      leaf_iterator (ir_structure& s)
        : m_leaf_stack (std::deque<stack_value_type> (
                        { std::make_pair (s.leaf_begin (), s.leaf_end ()) })),
          m_end (s.leaf_end ())
      {
        if (s.leaf_begin () == s.leaf_end ())
          pop ();
        expand_to_next ();
      }

      leaf_iterator& operator++ (void)
      {
        advance ();
        return *this;
      }

      bool operator== (const leaf_iterator& o) const
      {
        if (this->end () != o.end ())
          return false;
        if (this->empty ())
          return o.empty ();
        return this->current () == o.current ();
      }

      bool operator!= (const leaf_iterator& o) const
      {
        return operator== (o);
      }

      reference operator* (void) { return *current (); }
      pointer operator-> (void) { return &(*current ()); }

    private:

      bool empty (void) const noexcept
      {
        return m_leaf_stack.empty ();
      }

      sub_iter& current (void)       noexcept { return top ().first; }
      sub_citer current (void) const noexcept {return top ().first; }
      sub_citer end (void)     const noexcept { return m_end; }

      void advance (void)
      {
        if (empty ())
          throw ir_exception ("tried to advance an iterator out-of-bounds");
        ++current ();
        if (top_empty ())
          pop ();
        expand_to_next ();
      }

      void expand_to_next (void)
      {
        if (empty ())
          return;
        if (top_empty ())
          throw ir_exception ("top of stack was empty");
        while (ir_structure *s = dynamic_cast<ir_structure *> (*current ()))
          {
            if (s->has_leaves ())
              m_leaf_stack.emplace (s->leaf_begin (), s->leaf_end ());
            else
              return advance ();
          }
      }

      stack_ref top (void) { return m_leaf_stack.top (); }
      stack_cref top (void) const { return m_leaf_stack.top (); }

      void pop (void) { m_leaf_stack.pop (); }

      bool top_empty (void)
      {
        return top ().first == top ().second;
      }

      stack_type m_leaf_stack;
      sub_iter m_end;

    };
#endif

    void leaf_push_back (ir_basic_block& blk);

    template <typename It>
    void leaf_push_back (It first, It last);

  private:
    link_cache_vect m_leaf_cache;
  };
  
  template <typename Entry>
  class ir_component_list : public ir_structure
  {
  
  public:
    
    using entry_type = Entry;

  private:
    
    class find_cache
    {
    public:

      find_cache            (void)                  = delete;
      find_cache            (const find_cache&)     = default;
      find_cache            (find_cache&&) noexcept = default;
      find_cache& operator= (const find_cache&)     = default;
      find_cache& operator= (find_cache&&) noexcept = default;
      ~find_cache           (void)                  = default;

      explicit find_cache (comp_citer cit) noexcept
        : m_cit (cit)
      { }

      void emplace (comp_citer cit) noexcept
      {
        m_cit = cit;
      }

      [[nodiscard]]
      constexpr bool contains (const ir_component& p) const noexcept
      {
        return &p == m_cit->get ();
      }

      [[nodiscard]]
      constexpr const comp_citer& retrieve (void) const noexcept
      {
        return m_cit;
      }
      
    private:
      comp_citer m_cit;
    };
    
  public:

    explicit ir_component_list (void)
      : m_cache (emplace_before<entry_type> (end ()))
    { }
    
    ir_component_list (ir_component_list&& o) noexcept
      : m_body (std::move (o.m_body)),
        m_cache (std::move (o.m_cache))
    { }
    
    ir_component_list (const ir_component_list&) = delete;
    
    ir_component_list& operator= (const ir_component_list&) = delete;
    ir_component_list& operator= (ir_component_list&& o)    = delete;
    
    void reset (void) noexcept override
    {
      m_body.erase (++m_body.begin (), m_body.end ());
      front ()->reset ();
      set_cache (begin ());
      invalidate_leaf_cache ();
    }

    [[nodiscard]] comp_iter   begin (void)       noexcept { return m_body.begin ();   }
    [[nodiscard]] comp_citer  begin (void) const noexcept { return m_body.begin ();   }
    [[nodiscard]] comp_citer  cbegin (void) const noexcept { return m_body.cbegin ();  }

    [[nodiscard]] comp_iter   end (void)       noexcept { return m_body.end ();     }
    [[nodiscard]] comp_citer  end (void) const noexcept { return m_body.end ();     }
    [[nodiscard]] comp_citer  cend (void) const noexcept { return m_body.cend ();    }

    [[nodiscard]] comp_riter  rbegin (void)       noexcept { return m_body.rbegin ();  }
    [[nodiscard]] comp_criter rbegin (void) const noexcept { return m_body.rbegin ();  }
    [[nodiscard]] comp_criter crbegin (void) const noexcept { return m_body.crbegin (); }

    [[nodiscard]] comp_riter  rend (void)       noexcept { return m_body.rend ();    }
    [[nodiscard]] comp_criter rend (void) const noexcept { return m_body.rend ();    }
    [[nodiscard]] comp_criter crend (void) const noexcept { return m_body.crend ();   }

    [[nodiscard]] comp_ref    front (void)                { return m_body.front ();   }
    [[nodiscard]] comp_cref   front (void) const          { return m_body.front ();   }

    [[nodiscard]] comp_ref    back (void)                { return m_body.back ();    }
    [[nodiscard]] comp_cref   back (void) const          { return m_body.back ();    }

    [[nodiscard]] bool   empty         (void) const noexcept { return m_body.empty ();   }

    [[nodiscard]] comp_citer  last (void) const          { return --end ();          }

    comp_citer find (const ir_component& c)
    {
      if (is_cached (c))
        return retrieve_cache ();
      return set_cache (std::find_if (m_body.begin (), m_body.end (),
                                      [cmp = &c](auto&& u) { return u.get () == cmp; }));
    }
    
    template <typename S, typename ...Args,
              typename = std::enable_if_t<is_component<S>::value>>
    comp_citer emplace_before (comp_citer cit, Args&&... args)
    {
      return m_body.emplace (cit, create_component<S> (std::forward<Args> (args)...));
    }
  
    template <typename S, typename ...Args,
              typename = std::enable_if_t<is_component<S>::value>>
    S& emplace_front (Args&&... args)
    {
      auto u = create_component<S> (std::forward<Args> (args)...);
      S& ret = *u;
      m_body.emplace_front (std::move (u));
      invalidate_leaf_cache ();
      return ret;
    }

    template <typename S, typename ...Args,
              typename = std::enable_if_t<is_component<S>::value>>
    S& emplace_back (Args&&... args)
    {
      auto u = create_component<S> (std::forward<Args> (args)...);
      S& ret = *u;
      m_body.emplace_back (std::move (u));
      invalidate_leaf_cache ();
      return ret;
    }
  
    template <typename T, typename S>
    T& emplace_instruction_before (ir_basic_block& blk,
                                   std::initializer_list<std::reference_wrapper<S>>,
                                   ir_variable& var)
    {
      
      
      // find latest def before this one if this is not at the very end
      
    }
    
    template <typename T>
    T& emplace_instruction_before (ir_basic_block& blk, std::initializer_list<ir_operand>);
    
    
    
    //
    // virtual from ir_component
    //

    ir_basic_block& get_entry_block (void) noexcept override
    {
      return front ()->get_entry_block ();
    }

    //
    // virtual from ir_structure
    //
    
    ir_basic_block& split (ir_basic_block& blk, instr_iter pivot) override
    {
      auto  ret_it = emplace_before<ir_basic_block> (must_find (blk));
      auto& ret    = static_cast<ir_basic_block&> (**ret_it);
      
      // transfer instructions which occur after the pivot
      blk.split (pivot, ret);
    }

    // protected
    bool is_leaf_component (ir_component *c) noexcept override
    {
      return c == back ().get ();
    }
    
    void generate_leaf_cache (void) override
    {
      leaf_push_back (back ()->leaf_begin (), back ()->leaf_end ());
    }
  
    [[nodiscard]]
    std::list<nonnull_ptr<ir_def>> get_latest_defs (ir_variable& var) noexcept override
    {
      for (auto rit = rbegin (); rit != rend (); ++rit)
      {
        if (auto ret = (*rit)->get_latest_defs (var); ! ret.empty ())
          return ret;
      }
      return { };
    }
    
    [[nodiscard]]
    std::list<nonnull_ptr<ir_def>> get_latest_defs_before (ir_variable& var,
                                                           component_handle comp) override
    {
      if (! holds_alternative<comp_iter> (comp))
        throw ir_exception ("unexpected type held by component handle");
      auto it = get<comp_iter> (comp);
      if (it == end ())
        throw ir_exception ("component handle was at end");
      
      for (auto rit = comp_riter { it }; rit != rend (); ++rit)
      {
        if (auto ret = (*rit)->get_latest_defs (var); ! ret.empty ())
          return ret;
      }
      return { };
    }

  protected:

    comp_citer must_find (const ir_component& c)
    {
      if (auto cit = find (c); cit != end ())
        return cit;
      throw ir_exception ("Component not found in the structure.");
    }

  private:
    
    template <typename S, typename ...Args,
              typename = std::enable_if_t<is_component<S>::value>>
    std::unique_ptr<S> create_component (Args&&... args)
    {
      return std::make_unique<S> (*this, std::forward<Args> (args)...);
    }
    
    comp_citer set_cache (comp_citer cit) noexcept
    {
      if (cit != end ())
        m_cache.emplace (cit);
      return cit;
    }

    [[nodiscard]]
    constexpr bool is_cached (const ir_component& p) const noexcept
    {
      return m_cache.contains (p);
    }
    
    comp_citer retrieve_cache (void)
    {
      return m_cache.retrieve ();
    }

    component_list m_body;
    find_cache     m_cache;

  };
  
  template <typename Entry>
  class ir_sequence : public ir_component_list<Entry>
  {

    using base = ir_component_list<Entry>;

  public:

    ir_sequence            (void)                   = delete;
    ir_sequence            (const ir_sequence&)     = delete;
    ir_sequence            (ir_sequence&&) noexcept = delete;
    ir_sequence& operator= (const ir_sequence&)     = delete;
    ir_sequence& operator= (ir_sequence&&) noexcept = delete;
    ~ir_sequence           (void) noexcept override = default;

    explicit ir_sequence (ir_structure& parent)
      : m_parent (parent)
    { }

    //
    // virtual from ir_structure
    //

    ir_structure::link_iter pred_begin (ir_component& c) override
    {
      auto cit = base::must_find (c);
      if (cit == base::begin ())
        return m_parent.pred_begin (*this);
      return (*--cit)->leaf_begin ();
    }

    ir_structure::link_iter pred_end (ir_component& c) override
    {
      auto cit = base::must_find (c);
      if (cit == base::begin ())
        return m_parent.pred_end (*this);
      return (*--cit)->leaf_end ();
    }

    ir_structure::link_iter succ_begin (ir_component& c) override
    {
      using link_iter = ir_component::link_iter;
      auto cit = base::must_find (c);
      if (cit == base::last ())
        return m_parent.succ_begin (*this);
      return (*++cit)->get_entry_block ();
    }

    ir_structure::link_iter succ_end (ir_component& c) override
    {
      using link_iter = ir_component::link_iter;
      auto cit = base::must_find (c);
      if (cit == base::last ())
        return m_parent.succ_end (*this);
      return ++link_iter ((*++cit)->get_entry_block ());
    }

    void invalidate_leaf_cache (void) noexcept override
    {
      if (base::is_leaf_component (this))
        m_parent.invalidate_leaf_cache ();
    }

    [[nodiscard]]
    ir_function& get_function (void) noexcept override
    {
      return m_parent.get_function ();
    }

    [[nodiscard]]
    const ir_function& get_function (void) const noexcept override
    {
      return m_parent.get_function ();
    }

  private:

    ir_structure& m_parent;

  };

  class ir_fork_component : public ir_structure
  {
  public:

    ir_fork_component            (void)                         = delete;
    ir_fork_component            (const ir_fork_component&)     = delete;
    ir_fork_component            (ir_fork_component&&) noexcept = delete;
    ir_fork_component& operator= (const ir_fork_component&)     = delete;
    ir_fork_component& operator= (ir_fork_component&&) noexcept = delete;
    ~ir_fork_component           (void)       noexcept override;

    explicit ir_fork_component (ir_structure& parent)
      : m_parent (parent),
        m_condition (parent)
    { }

    link_iter pred_begin (ir_component& c) override;
    link_iter pred_end   (ir_component& c) override;
    link_iter succ_begin (ir_component& c) override;
    link_iter succ_end   (ir_component& c) override;

    ir_basic_block& get_entry_block (void) noexcept override
    {
      return m_condition;
    }

    bool is_leaf_component (ir_component *c) noexcept override;

    void generate_leaf_cache (void) override;

    void invalidate_leaf_cache (void) noexcept override
    {
      if (! leaf_cache_empty ())
        {
          clear_leaf_cache ();
          if (is_leaf_component (this))
            m_parent.invalidate_leaf_cache ();
        }
    }

    void reset (void) noexcept override
    {
      m_subcomponents.clear ();
    }

    ir_function& get_function (void) noexcept override
    {
      return m_parent.get_function ();
    }

    const ir_function& get_function (void) const noexcept override
    {
      return m_parent.get_function ();
    }

    constexpr bool is_condition (ir_component *c) const noexcept
    {
      return c == &m_condition;
    }
  
    [[nodiscard]]
    std::list<nonnull_ptr<ir_def>> get_latest_defs (ir_variable& var) noexcept override
    {
      bool condition_visited = false;
      std::list<nonnull_ptr<ir_def>> ret;
      
      for (auto&& comp : m_subcomponents)
      {
        auto defs = comp->get_latest_defs (var);
        if (defs.empty ())
        {
          if (! condition_visited)
          {
            ret.splice (ret.end (), m_condition.get_latest_defs (var));
            condition_visited = true;
          }
        }
        else
        {
          ret.splice (ret.end (), defs);
        }
      }
      return ret;
    }
  
    [[nodiscard]]
    std::list<nonnull_ptr<ir_def>> get_latest_defs_before (ir_variable& var,
                                                           component_handle comp) override
    {
      if (comp->get () == &m_condition)
        return { };
      return m_condition.get_latest_defs (var);
    }

  private:

    link_iter sub_entry_begin (void);

    link_iter sub_entry_end (void);

    void generate_sub_entry_cache (void);

    ir_structure& m_parent;

    ir_condition_block m_condition;
    component_list m_subcomponents;

    // holds entries for subcomponents
    link_cache_vect m_sub_entry_cache;

  };

  //!
  class ir_loop_component : public ir_structure
  {
  public:

    ir_loop_component            (void)                         = delete;
    ir_loop_component            (const ir_loop_component&)     = delete;
    ir_loop_component            (ir_loop_component&&) noexcept = delete;
    ir_loop_component& operator= (const ir_loop_component&)     = delete;
    ir_loop_component& operator= (ir_loop_component&&) noexcept = delete;
    ~ir_loop_component           (void)       noexcept override;

    explicit ir_loop_component (ir_structure& parent)
      : m_parent (parent),
        m_entry (*this),
        m_body (*this),
        m_condition (*this),
        m_exit (*this),
        m_cond_preds { m_entry, get_update_block () },
        m_succ_cache { m_body.get_entry_block (), m_exit }
    { }

    link_iter pred_begin (ir_component& c) override;
    link_iter pred_end   (ir_component& c) override;
    link_iter succ_begin (ir_component& c) override;
    link_iter succ_end   (ir_component& c) override;

    link_iter leaf_begin (void) override
    {
      return m_condition;
    }

    link_iter leaf_end (void) override
    {
      return ++leaf_begin ();
    }

    ir_basic_block& get_entry_block (void) noexcept override
    {
      return m_entry;
    }

    bool is_leaf_component (ir_component *c) noexcept override
    {
      return c == &m_condition;
    }

    void generate_leaf_cache (void) override
    {
      throw ir_exception ("this should not run");
    }

    ir_basic_block& get_update_block (void) const noexcept
    {
      return static_cast<ir_basic_block&> (*m_body.back ());
    }

    void invalidate_leaf_cache (void) noexcept override
    {
      if (! leaf_cache_empty ())
        {
          clear_leaf_cache ();
          if (is_leaf_component (this))
            m_parent.invalidate_leaf_cache ();
        }
    }
  
    [[nodiscard]]
    ir_function& get_function (void) noexcept override
    {
      return m_parent.get_function ();
    }

    [[nodiscard]]
    const ir_function& get_function (void) const noexcept override
    {
      return m_parent.get_function ();
    }
  
    [[nodiscard]]
    constexpr bool is_entry (ir_component *c) const noexcept
    {
      return c == &m_entry;
    }
  
    [[nodiscard]]
    constexpr bool is_condition (ir_component *c) const noexcept
    {
      return c == &m_condition;
    }
  
    [[nodiscard]]
    constexpr bool is_body (ir_component *c) const noexcept
    {
      return c == &m_body;
    }
  
    [[nodiscard]]
    constexpr bool is_exit (ir_component *c) const noexcept
    {
      return c == &m_exit;
    }
  
    [[nodiscard]]
    std::list<nonnull_ptr<ir_def>> get_latest_defs (ir_variable& var) noexcept override
    {
      
      if (auto opt_def = m_exit.get_latest_def (var))
        return { *opt_def };
  
      if (auto opt_def = m_condition.get_latest_def (var))
        return { *opt_def };
      
      std::list<nonnull_ptr<ir_def>> ret = m_body.get_latest_defs (var);
      ret.splice (ret.end (), m_entry.get_latest_defs (var));
      return ret;
    }
  
    [[nodiscard]]
    std::list<nonnull_ptr<ir_def>> get_latest_defs_before (ir_variable& var,
                                                           component_handle comp) override
    {
      if (is_entry (comp->get ()))
      {
        return { };
      }
      else if (is_body (comp->get ()))
      {
        return m_entry.get_latest_defs (var);
      }
      else if (is_condition (comp->get ()))
      {
        std::list<nonnull_ptr<ir_def>> ret = m_body.get_latest_defs (var);
        ret.splice (ret.end (), m_entry.get_latest_defs (var));
        return ret;
      }
      else if (is_exit (comp->get ()))
      {
        if (auto opt_def = m_condition.get_latest_def (var))
          return { *opt_def };
    
        std::list<nonnull_ptr<ir_def>> ret = m_body.get_latest_defs (var);
        ret.splice (ret.end (), m_entry.get_latest_defs (var));
        return ret;
      }
      else
      {
        throw ir_exception ("unexpected component handle");
      }
    }

  private:

    link_iter cond_succ_begin (void);

    link_iter cond_succ_end (void);

    ir_structure& m_parent;

    ir_basic_block m_entry;

    ir_sequence<ir_basic_block> m_body;

    // preds: entry, update
    ir_loop_condition_block m_condition;
    // succ: body, exit

    ir_basic_block m_exit;

    // does not change
    link_cache_vect m_cond_preds;

    link_cache_vect m_succ_cache;

  };

}

#endif

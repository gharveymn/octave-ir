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
  // class ir_structure_descender;
  // class ir_structure_ascender;
  
  class def_resolve_node
  {
    class incoming_pair
    {
      incoming_pair            (void)                     = delete;
      incoming_pair            (const incoming_pair&)     = default;
      incoming_pair            (incoming_pair&&) noexcept = default;
      incoming_pair& operator= (const incoming_pair&)     = default;
      incoming_pair& operator= (incoming_pair&&) noexcept = default;
      ~incoming_pair           (void)                     = default;
      
      constexpr ir_basic_block& get_block (void) const noexcept
      {
        return *m_block;
      }
  
      constexpr ir_def_timeline& get_timeline (void) const noexcept
      {
        return *m_timeline;
      }
      
      constexpr bool has_timeline (void) const noexcept
      {
        return m_timeline.has_value ();
      }
      
    private:
      nonnull_ptr<ir_basic_block> m_block;
      optional_ref<ir_def_timeline>  m_timeline;
    };
    
    nonnull_ptr<ir_basic_block> m_target;
    nonnull_ptr<ir_basic_block> m_join_at;
    std::vector<incoming_pair>  m_incoming;
  };
  

  class ir_structure : public ir_component
  {
  public:
    ir_structure            (void)                    = delete;
    ir_structure            (const ir_structure&)     = default;
    ir_structure            (ir_structure&&) noexcept = default;
    ir_structure& operator= (const ir_structure&)     = default;
    ir_structure& operator= (ir_structure&&) noexcept = default;
    ~ir_structure           (void)  override          = 0;
  
    explicit ir_structure (ir_structure& parent)
      : ir_component (parent)
    { }
  
    explicit ir_structure (nullopt_t)
      : ir_component (nullopt)
    { }
    
    virtual link_iter preds_begin (ir_component& c) = 0;
    virtual link_iter preds_end   (ir_component& c) = 0;
    virtual std::size_t num_preds (ir_component& c)
    {
      std::distance (preds_begin (c), preds_end (c));
    };
    
    virtual link_iter succs_begin (ir_component& c) = 0;
    virtual link_iter succs_end   (ir_component& c) = 0;
    virtual std::size_t num_succs (ir_component& c)
    {
      std::distance (succs_begin (c), succs_end (c));
    };
    
    virtual std::unique_ptr<ir_component>& get_pointer (const ir_component& c) = 0;
  
    const std::unique_ptr<ir_component>& get_pointer (const ir_component& c) const
    {
      return const_cast<ir_structure *> (this)->get_pointer (c);
    }
    
    // mutate a component inside a structure to a different type of component
    template <typename T>
    T& mutate (std::unique_ptr<ir_component>& ptr)
    {
      return *(ptr = std::make_unique<T> (std::move (ptr)));
    }
    
    // virtual ir_basic_block& split (ir_basic_block& blk, instr_iter pivot) = 0;
    
    ir_basic_block& preds_front (ir_component& c) { return **preds_begin (c); }
    ir_basic_block& preds_back  (ir_component& c) { return **(--preds_end (c)); }
    ir_basic_block& succs_front (ir_component& c) { return **succs_begin (c); }
    ir_basic_block& succs_back  (ir_component& c) { return **(--succs_end (c)); }

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
      throw ir_exception ("component was not a basic block");
    }
  
    // virtual ir_structure_descender create_descender () noexcept;
    // virtual ir_structure_descender create_descender (ir_component& loc) noexcept;
    
    // virtual ir_structure_ascender create_ascender () noexcept;
    // virtual ir_structure_ascender create_ascender (ir_component& loc) noexcept;

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
    
    template <typename S, typename ...Args,
              typename = std::enable_if_t<is_component<S>::value>>
    std::unique_ptr<S> create_component (Args&&... args)
    {
      return std::make_unique<S> (*this, std::forward<Args> (args)...);
    }

  private:
    link_cache_vect m_leaf_cache;
  };
  
  /*
  class ir_structure_descender
  {
  public:
    using comp_list = std::vector<nonnull_ptr<ir_component>>;
    using iter       = comp_list::iterator;
    using citer      = comp_list::const_iterator;
    using riter      = comp_list::reverse_iterator;
    using criter     = comp_list::const_reverse_iterator;
    using ref        = comp_list::reference;
    using cref       = comp_list::const_reference;
    
    [[nodiscard]] iter   begin (void)       noexcept   { return m_components.begin ();   }
    [[nodiscard]] citer  begin (void) const noexcept   { return m_components.begin ();   }
    [[nodiscard]] citer  cbegin (void) const noexcept  { return m_components.cbegin ();  }
    
    [[nodiscard]] iter   end (void)       noexcept     { return m_components.end ();     }
    [[nodiscard]] citer  end (void) const noexcept     { return m_components.end ();     }
    [[nodiscard]] citer  cend (void) const noexcept    { return m_components.cend ();    }
    
    [[nodiscard]] riter  rbegin (void)       noexcept  { return m_components.rbegin ();  }
    [[nodiscard]] criter rbegin (void) const noexcept  { return m_components.rbegin ();  }
    [[nodiscard]] criter crbegin (void) const noexcept { return m_components.crbegin (); }
    
    [[nodiscard]] riter  rend (void)       noexcept    { return m_components.rend ();    }
    [[nodiscard]] criter rend (void) const noexcept    { return m_components.rend ();    }
    [[nodiscard]] criter crend (void) const noexcept   { return m_components.crend ();   }
    
    [[nodiscard]] ref    front (void)                  { return m_components.front ();   }
    [[nodiscard]] cref   front (void) const            { return m_components.front ();   }
    
    [[nodiscard]] ref    back (void)                   { return m_components.back ();    }
    [[nodiscard]] cref   back (void) const             { return m_components.back ();    }

    ir_structure_descender (void)                              = delete;
    ir_structure_descender (const ir_structure_descender&)     = default;
    ir_structure_descender (ir_structure_descender&&) noexcept = default;
    ir_structure_descender& operator= (const ir_structure_descender&)     = default;
    ir_structure_descender& operator= (ir_structure_descender&&) noexcept = default;
    ~ir_structure_descender (void)                              = default;
    
    ir_structure_descender (comp_list&& l, iter start, bool is_fork)
      : m_components (std::move (l)),
        m_start      (start),
        m_is_fork    (is_fork)
    { }
    
    template <typename F, typename Ret, typename... Args>
    std::enable_if_t<std::is_same_v<std::invoke_result_t<F, ir_basic_block&, Args&&...>,
                                    bool>,
                     std::vector<Ret>>
    collect (F func, Args&&... args)
    {
      auto f = std::bind (func, std::placeholders::_1, std::forward<Args> (args)...);
      return select_collect (f);
    }
  
  private:
  
    template <typename F, typename Ret>
    constexpr std::pair<bool, std::vector<Ret>> select_collect (F f)
    {
      return m_is_fork ? sequence_collect (f) : fork_collect (f);
    }
    
    template <typename B, typename Ret>
    std::pair<bool, std::vector<Ret>> sequence_collect (B unary_func)
    {
      std::vector<Ret> ret_vec { };
      return { std::any_of (m_start, m_components.end (),
                             [&ret_vec, unary_func] (nonnull_ptr<ir_component> c)
                             {
                               auto curr_ret = single_collect (unary_func, c);
                               auto curr_ret_vec = std::get<std::vector<Ret>> (curr_ret);
                               if (! curr_ret_vec.empty ())
                                 std::move (curr_ret_vec.begin (), curr_ret_vec.end (),
                                            std::back_inserter (ret_vec));
                               return std::get<bool> (curr_ret);
                             }), std::move (ret_vec) };
    }
    
    template <typename B, typename Ret>
    std::pair<bool, std::vector<Ret>> fork_collect (B unary_func)
    {
      auto beg = m_start;
      if (m_components.empty ())
        return { false, { } };
  
      auto p = single_collect (unary_func, *beg);
      if (beg != m_components.begin () || std::get<bool> (p))
        return p;
  
      bool stop_branch = true;
      auto& ret_vec = std::get<std::vector<Ret>> (p);
      std::for_each (++beg, m_components.end (),
                     [&stop_branch, &ret_vec, unary_func] (nonnull_ptr<ir_component> c)
                     {
                       auto curr_ret = single_collect (unary_func, c);
                       auto curr_ret_vec = std::get<std::vector<Ret>> (curr_ret);
                       if (! curr_ret_vec.empty ())
                         std::move (curr_ret_vec.begin (), curr_ret_vec.end (),
                                    std::back_inserter (ret_vec));
                       stop_branch &= std::get<bool> (curr_ret);
                     });
      return { stop_branch, ret_vec };
    }
    
    template <typename B, typename Ret>
    static std::pair<bool, std::vector<Ret>>
    single_collect (B unary_func, nonnull_ptr<ir_component> c)
    {
      if (auto *block = dynamic_cast<ir_basic_block *> (c.get ()))
      {
        if (auto ret = unary_func (*block))
          return { true, { *ret } };
        else
          return { false, { } };
      }
      auto sub_des = static_cast<ir_structure&> (*c).create_descender ();
      return sub_des.select_collect (unary_func);
    }
    
    comp_list  m_components;
    iter       m_start;
    bool       m_is_fork;
  };
  
  class ir_structure_ascender
  {
  public:
    using comp_list = std::vector<nonnull_ptr<ir_component>>;
    using iter       = comp_list::iterator;
    using citer      = comp_list::const_iterator;
    using riter      = comp_list::reverse_iterator;
    using criter     = comp_list::const_reverse_iterator;
    using ref        = comp_list::reference;
    using cref       = comp_list::const_reference;
    
    [[nodiscard]] iter   begin (void)       noexcept   { return m_components.begin ();   }
    [[nodiscard]] citer  begin (void) const noexcept   { return m_components.begin ();   }
    [[nodiscard]] citer  cbegin (void) const noexcept  { return m_components.cbegin ();  }
    
    [[nodiscard]] iter   end (void)       noexcept     { return m_components.end ();     }
    [[nodiscard]] citer  end (void) const noexcept     { return m_components.end ();     }
    [[nodiscard]] citer  cend (void) const noexcept    { return m_components.cend ();    }
    
    [[nodiscard]] riter  rbegin (void)       noexcept  { return m_components.rbegin ();  }
    [[nodiscard]] criter rbegin (void) const noexcept  { return m_components.rbegin ();  }
    [[nodiscard]] criter crbegin (void) const noexcept { return m_components.crbegin (); }
    
    [[nodiscard]] riter  rend (void)       noexcept    { return m_components.rend ();    }
    [[nodiscard]] criter rend (void) const noexcept    { return m_components.rend ();    }
    [[nodiscard]] criter crend (void) const noexcept   { return m_components.crend ();   }
    
    [[nodiscard]] ref    front (void)                  { return m_components.front ();   }
    [[nodiscard]] cref   front (void) const            { return m_components.front ();   }
    
    [[nodiscard]] ref    back (void)                   { return m_components.back ();    }
    [[nodiscard]] cref   back (void) const             { return m_components.back ();    }
    
    ir_structure_ascender (void)                              = delete;
    ir_structure_ascender (const ir_structure_ascender&)     = default;
    ir_structure_ascender (ir_structure_ascender&&) noexcept = default;
    ir_structure_ascender& operator= (const ir_structure_ascender&)     = default;
    ir_structure_ascender& operator= (ir_structure_ascender&&) noexcept = default;
    ~ir_structure_ascender (void)                              = default;
    
    ir_structure_ascender (comp_list&& l, riter rstart, ir_structure& structure, bool is_fork)
      : m_components (std::move (l)),
        m_rstart     (rstart),
        m_structure  (structure),
        m_is_fork    (is_fork)
    { }
    
    template <typename F, typename Ret, typename... Args>
    std::enable_if_t<std::is_same_v<std::invoke_result_t<F, ir_basic_block&, Args&&...>,
                                    bool>,
                     std::vector<Ret>>
    collect (F func, Args&&... args)
    {
      auto f = std::bind (func, std::placeholders::_1, std::forward<Args> (args)...);
      return select_collect (f);
    }
  
  private:
    
    template <typename F, typename Ret>
    constexpr std::pair<bool, std::vector<Ret>> select_collect (F f)
    {
      return m_is_fork ? sequence_collect (f) : fork_collect (f);
    }
    
    template <typename B, typename Ret>
    std::pair<bool, std::vector<Ret>> sequence_collect (B unary_func)
    {
      std::vector<Ret> ret_vec { };
      bool branch_stop = std::any_of (m_rstart, m_components.rend (),
                                      [&ret_vec, unary_func] (nonnull_ptr<ir_component> c)
                                      {
                                        auto curr_ret = single_collect (unary_func, c);
                                        auto& curr_ret_vec = std::get<std::vector<Ret>> (curr_ret);
                                        if (! curr_ret_vec.empty ())
                                          std::move (curr_ret_vec.begin (), curr_ret_vec.end (),
                                                     std::back_inserter (ret_vec));
                                        return std::get<bool> (curr_ret);
                                      });
      if (! branch_stop && m_structure->has_parent ())
      {
        ir_structure& parent = m_structure->get_parent ();
        auto sub_asc = parent.create_ascender (*m_structure);
        auto parent_ret = sub_asc.select_collect (unary_func);
        branch_stop = std::get<bool> (parent_ret);
        auto& parent_ret_vec = std::get<std::vector<Ret>> (parent_ret);
        if (! parent_ret_vec.empty ())
          std::move (parent_ret_vec.begin (), parent_ret_vec.end (),
                     std::back_inserter (ret_vec));
      }
      return { branch_stop, std::move (ret_vec) };
    }
    
    template <typename B, typename Ret>
    std::pair<bool, std::vector<Ret>> fork_collect (B unary_func)
    {
      auto beg = m_rstart;
      if (m_components.empty())
        return { false, { } };
      
      auto p = single_descend_collect (unary_func, *beg);
      if (beg == --m_components.rend () || std::get<bool> (p))
        return p;
      
      auto& curr_ret_vec = std::get<std::vector<Ret>> (p);
      auto cond_ret = single_collect (unary_func, m_components.front ());
      auto& cond_ret_vec = std::get<std::vector<Ret>> (p);
      
      std::move (cond_ret_vec.begin (), cond_ret_vec.end (), std::back_inserter (curr_ret_vec));
      
      bool branch_stop = std::get<bool> (cond_ret);
      if (! branch_stop && m_structure->has_parent ())
      {
        ir_structure& parent = m_structure->get_parent ();
        auto sub_asc = parent.create_ascender (*m_structure);
        auto parent_ret = sub_asc.select_collect (unary_func);
        branch_stop = std::get<bool> (parent_ret);
        auto& parent_ret_vec = std::get<std::vector<Ret>> (parent_ret);
        if (! parent_ret_vec.empty ())
          std::move (parent_ret_vec.begin (), parent_ret_vec.end (),
                     std::back_inserter (curr_ret_vec));
      }
      return { branch_stop, std::move (curr_ret_vec) };
    }
    
    template <typename B, typename Ret>
    static std::pair<bool, std::vector<Ret>>
    single_collect (B unary_func, nonnull_ptr<ir_component> c)
    {
      if (auto *block = dynamic_cast<ir_basic_block *> (c.get ()))
      {
        if (auto ret = unary_func (*block))
          return { true, { *ret } };
        else
          return { false, { } };
      }
      auto sub_asc = static_cast<ir_structure&> (*c).create_ascender ();
      return sub_asc.select_collect (unary_func);
    }
    
    comp_list                 m_components;
    riter                     m_rstart;
    nonnull_ptr<ir_structure> m_structure;
    bool                      m_is_fork;
  };
  */
  
  class ir_sequence : public ir_structure
  {
    class find_cache
    {
    public:
    
      find_cache            (void)                  = default;
      find_cache            (const find_cache&)     = default;
      find_cache            (find_cache&&) noexcept = default;
      find_cache& operator= (const find_cache&)     = default;
      find_cache& operator= (find_cache&&) noexcept = default;
      ~find_cache           (void)                  = default;
    
      explicit find_cache (comp_iter it) noexcept
        : m_it (it)
      { }
    
      void emplace (comp_iter it) noexcept
      {
        m_it = it;
      }
    
      [[nodiscard]]
      constexpr bool contains (const ir_component& p) const noexcept
      {
        return &p == m_it->get ();
      }
    
      [[nodiscard]]
      comp_iter retrieve (void) const noexcept
      {
        return m_it;
      }
  
    private:
      comp_iter m_it;
    };
  
    explicit ir_sequence (ir_structure& parent)
      : ir_structure (parent)
    { }
    
  protected:
    explicit ir_sequence (nullopt_t)
      : ir_structure (nullopt)
    { }
    
  public:
    ir_sequence            (void)                   = delete;
    ir_sequence            (const ir_sequence&)     = delete;
    ir_sequence            (ir_sequence&&) noexcept = default;
    ir_sequence& operator= (const ir_sequence&)     = delete;
    ir_sequence& operator= (ir_sequence&&) noexcept = default;
    ~ir_sequence           (void) override          = default;
  
    [[nodiscard]] auto  begin   (void)       noexcept { return m_body.begin ();   }
    [[nodiscard]] auto  begin   (void) const noexcept { return m_body.begin ();   }
    [[nodiscard]] auto  cbegin  (void) const noexcept { return m_body.cbegin ();  }
  
    [[nodiscard]] auto  end     (void)       noexcept { return m_body.end ();     }
    [[nodiscard]] auto  end     (void) const noexcept { return m_body.end ();     }
    [[nodiscard]] auto  cend    (void) const noexcept { return m_body.cend ();    }
  
    [[nodiscard]] auto  rbegin  (void)       noexcept { return m_body.rbegin ();  }
    [[nodiscard]] auto  rbegin  (void) const noexcept { return m_body.rbegin ();  }
    [[nodiscard]] auto  crbegin (void) const noexcept { return m_body.crbegin (); }
  
    [[nodiscard]] auto  rend    (void)       noexcept { return m_body.rend ();    }
    [[nodiscard]] auto  rend    (void) const noexcept { return m_body.rend ();    }
    [[nodiscard]] auto  crend   (void) const noexcept { return m_body.crend ();   }
  
    [[nodiscard]] auto& front   (void)       noexcept { return m_body.front ();   }
    [[nodiscard]] auto& front   (void) const noexcept { return m_body.front ();   }
  
    [[nodiscard]] auto& back    (void)       noexcept { return m_body.back ();    }
    [[nodiscard]] auto& back    (void) const noexcept { return m_body.back ();    }
  
    [[nodiscard]] auto  size    (void) const noexcept { return m_body.size ();    }
    [[nodiscard]] auto  empty   (void) const noexcept { return m_body.empty ();   }
  
    [[nodiscard]] auto  last    (void)       noexcept { return --end ();          }
    [[nodiscard]] auto  last    (void) const noexcept { return --end ();          }
    [[nodiscard]] auto  clast   (void) const noexcept { return --cend ();         }
    
    template <typename Entry, typename ...Args>
    static ir_sequence create (ir_structure& parent, Args&&... args)
    {
      ir_sequence ret (parent);
      ret.m_body.push_back (ret.create_component (std::forward<Args> (args)...));
    }
  
    void reset (void) noexcept override
    {
      m_body.erase (++m_body.begin (), m_body.end ());
      front ()->reset ();
      m_cache.emplace (begin ());
      invalidate_leaf_cache ();
    }
  
    comp_iter find (const ir_component& c)
    {
      if (m_cache.contains (c))
        return m_cache.retrieve ();
      auto found = std::find_if (m_body.begin (), m_body.end (),
                                 [&c](const auto& u) { return u.get () == &c; });
      if (found != end ())
        m_cache.emplace (found);
      return found;
    }
  
    comp_citer find (const ir_component& c) const
    {
      return const_cast<ir_sequence *> (this)->find (c);
    }
  
    std::unique_ptr<ir_component>& get_pointer (const ir_component& c) override
    {
      return *find (c);
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
  
    //
    // virtual from ir_structure
    //
  
    ir_structure::link_iter preds_begin (ir_component& c) override
    {
      auto cit = must_find (c);
      if (cit == begin ())
        return get_parent ().preds_begin (*this);
      return (*--cit)->leaf_begin ();
    }
  
    ir_structure::link_iter preds_end (ir_component& c) override
    {
      auto cit = must_find (c);
      if (cit == begin ())
        return get_parent ().preds_end (*this);
      return (*--cit)->leaf_end ();
    }
  
    ir_structure::link_iter succs_begin (ir_component& c) override
    {
      using link_iter = ir_component::link_iter;
      auto cit = must_find (c);
      if (cit == last ())
        return get_parent ().succs_begin (*this);
      return (*++cit)->get_entry_block ();
    }
  
    ir_structure::link_iter succs_end (ir_component& c) override
    {
      using link_iter = ir_component::link_iter;
      auto cit = must_find (c);
      if (cit == last ())
        return get_parent ().succs_end (*this);
      return ++link_iter ((*++cit)->get_entry_block ());
    }
  
    void invalidate_leaf_cache (void) noexcept override
    {
      if (is_leaf_component (this))
        get_parent ().invalidate_leaf_cache ();
    }
  
    [[nodiscard]]
    ir_function& get_function (void) noexcept override
    {
      return get_parent ().get_function ();
    }
  
    [[nodiscard]]
    const ir_function& get_function (void) const noexcept override
    {
      return get_parent ().get_function ();
    }

  protected:
  
    comp_citer must_find (const ir_component& c)
    {
      if (auto cit = find (c); cit != end ())
        return cit;
      throw ir_exception ("Component not found in the structure.");
    }

  private:
    component_list            m_body;
    find_cache                m_cache;
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
      : ir_structure (parent),
        m_condition (create_component<ir_condition_block> (parent))
    { }

    link_iter preds_begin (ir_component& c) override;
    link_iter preds_end   (ir_component& c) override;
    link_iter succs_begin (ir_component& c) override;
    link_iter succs_end   (ir_component& c) override;

    ir_basic_block& get_entry_block (void) noexcept override
    {
      return m_condition->get_entry_block ();
    }

    bool is_leaf_component (ir_component *c) noexcept override;

    void generate_leaf_cache (void) override;

    void invalidate_leaf_cache (void) noexcept override
    {
      if (! leaf_cache_empty ())
        {
          clear_leaf_cache ();
          if (is_leaf_component (this))
            get_parent ().invalidate_leaf_cache ();
        }
    }

    void reset (void) noexcept override
    {
      m_subcomponents.clear ();
    }

    ir_function& get_function (void) noexcept override
    {
      return get_parent ().get_function ();
    }

    constexpr bool is_condition (const ir_component& c) const noexcept
    {
      return &c == m_condition.get ();
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
    link_iter sub_entry_end   (void);

    void generate_sub_entry_cache (void);

    std::unique_ptr<ir_component> m_condition;
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
      : ir_structure (parent),
        m_entry (*this),
        m_body (*this),
        m_condition (*this),
        m_exit (*this),
        m_cond_preds { m_entry, get_update_block () },
        m_succ_cache { m_body.get_entry_block (), m_exit }
    { }

    link_iter preds_begin (ir_component& c) override;
    link_iter preds_end   (ir_component& c) override;
    link_iter succs_begin (ir_component& c) override;
    link_iter succs_end   (ir_component& c) override;

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
            get_parent ().invalidate_leaf_cache ();
        }
    }
  
    [[nodiscard]]
    ir_function& get_function (void) noexcept override
    {
      return get_parent ().get_function ();
    }
  
    [[nodiscard]]
    constexpr bool is_entry (const ir_component& c) const noexcept
    {
      return &c == m_entry.get ();
    }
  
    [[nodiscard]]
    constexpr bool is_condition (const ir_component& c) const noexcept
    {
      return &c == m_condition.get ();
    }
  
    [[nodiscard]]
    constexpr bool is_body (const ir_component& c) const noexcept
    {
      return &c == static_cast<const ir_component *> (&m_body);
    }
  
    [[nodiscard]]
    constexpr bool is_exit (const ir_component& c) const noexcept
    {
      return &c == m_exit.get ();
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
    
    std::unique_ptr<ir_component> m_entry;
    ir_sequence                   m_body;
    std::unique_ptr<ir_component> m_condition; // preds: entry, update | succs: body, exit
    std::unique_ptr<ir_component> m_exit;

    // does not change
    link_cache_vect m_cond_preds;

    link_cache_vect m_succ_cache;

  };

}

#endif

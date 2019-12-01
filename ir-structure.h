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

#include "octave-config.h"

#include "ir-component.h"
#include "ir-block.h"
#include "ir-variable.h"

#include <algorithm>
#include <memory>
#include <list>
#include <stack>

namespace octave
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

    using comp_list = std::list<std::unique_ptr<ir_component>>;
    using comp_iter = comp_list::iterator;
    using comp_citer = comp_list::const_iterator;
    using comp_ref = comp_list::reference;
    using comp_cref = comp_list::const_reference;
    
    virtual link_iter pred_begin (ir_component *c) = 0;
    virtual link_iter pred_end   (ir_component *c) = 0;
    virtual link_iter succ_begin (ir_component *c) = 0;
    virtual link_iter succ_end   (ir_component *c) = 0;
  
    ir_basic_block * pred_front (ir_component *c) { return *pred_begin (c); }
    ir_basic_block * pred_back  (ir_component *c) { return *(--pred_end (c)); }
    ir_basic_block * succ_front (ir_component *c) { return *succ_begin (c); }
    ir_basic_block * succ_back  (ir_component *c) { return *(--succ_end (c)); }

    link_iter leaf_begin (void) override;
    link_iter leaf_end (void) override;

    virtual void invalidate_leaf_cache (void) noexcept = 0;

  protected:

    void clear_leaf_cache (void) noexcept
    {
      m_leaf_cache.clear ();
    }

    bool leaf_cache_empty (void) const noexcept
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

    void leaf_push_back (ir_basic_block *blk);

    template <typename It>
    void leaf_push_back (It first, It last);

  private:
    link_cache_vec m_leaf_cache;
  };
  
  template <typename T>
  class ir_sequence : public ir_structure
  {
  
  public:
    
    using entry_type = T;
  
    using iter  = comp_iter;
    using citer = comp_citer;
    using ref   = comp_ref;
    using cref  = comp_cref;
    
  private:
    
    class find_cache
    {
    public:
      
      explicit find_cache (citer cit) noexcept
        : m_ptr (cit->get ()),
          m_cit (cit)
      { }
      
      find_cache (const find_cache&) noexcept         = default;
      find_cache (find_cache&& o) noexcept            = default;
      
      find_cache& operator= (const find_cache& o)     = delete;
      find_cache& operator= (find_cache&& o)          = delete;
      
      void set (citer cit) noexcept
      {
        m_ptr = cit->get ();
        m_cit = cit;
      }
      
      constexpr bool compare (ir_component *p) const noexcept
      {
        return p == m_ptr;
      }
      
      constexpr const citer& retrieve (void) const noexcept
      {
        return m_cit;
      }
      
    private:
      const ir_component *m_ptr;
      citer               m_cit;
    };
    
  public:

    explicit ir_sequence (void)
      : m_cache (emplace<entry_type> (end ()))
    { }
    
    ir_sequence (ir_sequence&& o) noexcept
      : m_body (std::move (o.m_body)),
        m_cache (std::move (o.m_cache))
    { }
    
    ir_sequence (const ir_sequence&) = delete;
    
    ir_sequence& operator= (const ir_sequence&) = delete;
    ir_sequence& operator= (ir_sequence&& o)    = delete;
    
    void reset (void) noexcept override
    {
      m_body.erase (++m_body.begin (), m_body.end ());
      front ()->reset ();
      set_cache (begin ());
      invalidate_leaf_cache ();
    }

    citer begin (void) const noexcept { return m_body.begin (); }
    citer end   (void) const noexcept { return m_body.end ();   }
    cref  front (void) const          { return m_body.front (); }
    cref  back  (void) const          { return m_body.back ();  }

    bool empty (void) const           { return m_body.empty (); }

    citer last (void) const
    {
      return --end ();
    }

    citer find (ir_component *c)
    {
      if (! is_cached (c))
        set_cache (std::find_if (m_body.begin (), m_body.end (),
                                 [c](comp_cref u) {return u.get () == c; }));
      return retrieve_cache ();
    }
    
    template <typename S, typename ...Args,
              typename = enable_if_t<is_component<S>::value>>
    citer emplace (citer cit, Args&&... args)
    {
      return m_body.emplace (cit,
        create_component<S> (std::forward<Args> (args)...));
    }

    template <typename S, typename ...Args,
              typename = enable_if_t<is_component<S>::value>>
    S * emplace_back (Args&&... args)
    {
      std::unique_ptr<S> u =
        create_component<S> (std::forward<Args> (args)...);
      S *ret = u.get ();
      m_body.emplace_back (std::move (u));
      invalidate_leaf_cache ();
      return ret;
    }

    //
    // virtual from ir_component
    //

    ir_basic_block * get_entry_block (void) noexcept override
    {
      return front ()->get_entry_block ();
    }

    //
    // virtual from ir_structure
    //

    // protected
    bool is_leaf_component (ir_component *c) noexcept override
    {
      return c == back ().get ();
    }
    
    void generate_leaf_cache (void) override
    {
      leaf_push_back (back ()->leaf_begin (), back ()->leaf_end ());
    }

  protected:

    citer must_find (ir_component *c)
    {
      citer cit = find (c);
      if (cit == end ())
        throw ir_exception ("component not found in the structure");
      return cit;
    }

  private:
    
    template <typename S, typename ...Args,
              typename = enable_if_t<is_component<S>::value>>
    std::unique_ptr<S> create_component (Args&&... args)
    {
      return octave::make_unique<S> (*this, std::forward<Args> (args)...);
    }
    
    void set_cache (citer cit) noexcept
    {
      return m_cache.set (cit);
    }
    
    constexpr bool is_cached (ir_component *p) const
    {
      return m_cache.compare (p);
    }
    
    citer retrieve_cache (void)
    {
      return m_cache.retrieve ();
    }

    comp_list m_body;

    find_cache m_cache;

  };
  
  template <typename T>
  class ir_subsequence : public ir_sequence<T>
  {
    
    using base_type = ir_sequence<T>;
  
  public:
    
    explicit ir_subsequence (ir_structure& parent)
      : m_parent (parent)
    { }

    ~ir_subsequence (void) noexcept override = default;

    //
    // virtual from ir_structure
    //
  
    void reset (void) noexcept override
    {
      base_type::reset ();
      invalidate_leaf_cache ();
    }
  
    ir_structure::link_iter pred_begin (ir_component *c) override
    {
      ir_structure::comp_citer cit = base_type::must_find (c);
      if (cit == base_type::begin ())
        return m_parent.pred_begin (this);
      return (*--cit)->leaf_begin ();
    }
  
    ir_structure::link_iter pred_end (ir_component *c) override
    {
      ir_structure::comp_citer cit = base_type::must_find (c);
      if (cit == base_type::begin ())
        return m_parent.pred_end (this);
      return (*--cit)->leaf_end ();
    }
  
    ir_structure::link_iter succ_begin (ir_component *c) override
    {
      ir_structure::comp_citer cit = base_type::must_find (c);
      if (cit == base_type::last ())
        return m_parent.succ_begin (this);
      return ir_component::link_iter ((*++cit)->get_entry_block ());
    }
  
    ir_structure::link_iter succ_end (ir_component *c) override
    {
      ir_structure::comp_citer cit = base_type::must_find (c);
      if (cit == base_type::last ())
        return m_parent.succ_end (this);
      return ++ir_component::link_iter ((*++cit)->get_entry_block ());
    }
    
    void invalidate_leaf_cache (void) noexcept override
    {
      if (base_type::is_leaf_component (this))
        m_parent.invalidate_leaf_cache ();
    }
  
    ir_function& get_function (void) noexcept override
    {
      return m_parent.get_function ();
    }
  
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

    ir_fork_component (ir_structure& parent)
      : m_parent (parent),
        m_condition (parent)
    { }

    ~ir_fork_component () noexcept override;

    link_iter pred_begin (ir_component *c) override;
    link_iter pred_end   (ir_component *c) override;
    link_iter succ_begin (ir_component *c) override;
    link_iter succ_end   (ir_component *c) override;

    ir_basic_block * get_entry_block (void) noexcept override
    {
      return &m_condition;
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

  private:

    link_iter sub_entry_begin (void);

    link_iter sub_entry_end (void);

    void generate_sub_entry_cache (void);

    ir_structure& m_parent;

    ir_condition_block m_condition;
    comp_list m_subcomponents;

    // holds entries for subcomponents
    link_cache_vec m_sub_entry_cache;

  };

  //!
  class ir_loop_component : public ir_structure
  {
  public:

    ir_loop_component (ir_structure& parent)
      : m_parent (parent),
        m_entry (*this),
        m_body (*this),
        m_condition (*this),
        m_exit (*this),
        m_cond_preds { &m_entry, get_update_block () },
        m_succ_cache { m_body.get_entry_block (), &m_exit }
    { }

    ~ir_loop_component (void) noexcept override;

    link_iter pred_begin (ir_component *c) override;
    link_iter pred_end   (ir_component *c) override;
    link_iter succ_begin (ir_component *c) override;
    link_iter succ_end   (ir_component *c) override;
  
    link_iter leaf_begin (void) override
    {
      return link_iter (&m_condition);
    }
    
    link_iter leaf_end (void) override
    {
      return ++leaf_begin ();
    }

    ir_basic_block *get_entry_block (void) noexcept override
    {
      return &m_entry;
    }

    bool is_leaf_component (ir_component *c) noexcept override
    {
      return c == &m_condition;
    }

    void generate_leaf_cache (void) override
    {
      throw ir_exception ("this should not run");
    }

    ir_basic_block *get_update_block (void) const noexcept;
  
    void invalidate_leaf_cache (void) noexcept override
    {
      if (! leaf_cache_empty ())
        {
          clear_leaf_cache ();
          if (is_leaf_component (this))
            m_parent.invalidate_leaf_cache ();
        }
    }
  
    ir_function& get_function (void) noexcept override
    {
      return m_parent.get_function ();
    }
  
    const ir_function& get_function (void) const noexcept override
    {
      return m_parent.get_function ();
    }

  private:

    link_iter cond_succ_begin (void);

    link_iter cond_succ_end (void);

    ir_structure& m_parent;

    ir_basic_block m_entry;
  
    ir_subsequence<ir_basic_block> m_body;

    // preds: entry, update
    ir_loop_condition_block m_condition;
    // succ: body, exit

    ir_basic_block m_exit;

    // does not change
    link_cache_vec m_cond_preds;

    link_cache_vec m_succ_cache;

  };

}

#endif

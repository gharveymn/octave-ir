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

#if ! defined (ir_component_h)
#define ir_component_h 1

#include "octave-config.h"
#include "ir-component.h"
#include "ir-variable.h"

#include <stack>

namespace octave
{

  class ir_component;
  class ir_module;

  class ir_structure : public ir_component
  {
  public:
    explicit ir_structure (ir_module& mod)
      : ir_component (mod)
    { }

    ~ir_structure (void) noexcept override = 0;

    using comp_list = std::list<std::unique_ptr<ir_component>>;
    using comp_iter = comp_list::iterator;
    using comp_citer = comp_list::const_iterator;
    using comp_ref = comp_list::reference;
    using comp_cref = comp_list::const_reference;

    virtual link_iter pred_begin (ir_component *c)  = 0;

    virtual link_iter pred_end (ir_component *c) = 0;

    virtual link_iter succ_begin (ir_component *c) = 0;

    virtual link_iter succ_end (ir_component *c) = 0;

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

    virtual bool is_leaf_component (ir_component *c) noexcept = 0;

    virtual void generate_leaf_cache (void) = 0;

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

  class ir_sequence : public ir_structure
  {
  protected:

    template <typename T>
    struct init_wrapper
    { };

    template <typename T>
    ir_sequence (ir_module& mod, init_wrapper<T>)
      : ir_structure (mod)
    {
      m_find_cache.first  = emplace_back<T> ();
      m_find_cache.second = last ();
    }

  public:

    explicit ir_sequence (ir_module& mod)
      : ir_sequence (mod, init_wrapper<ir_basic_block>{})
    { }

    ~ir_sequence (void) noexcept override;

    comp_citer begin (void) const noexcept { return m_body.begin (); }
    comp_citer end (void)   const noexcept { return m_body.end (); }

    comp_cref front (void) const { return m_body.front (); }
    comp_cref back (void)   const { return m_body.back (); }

    bool empty (void) const { return m_body.empty (); }

    comp_citer last (void) const;

    comp_citer find (ir_component *c);

    template <typename T, typename ...Args>
    enable_if_t<std::is_base_of<ir_component, T>::value, T> *
    emplace_back (Args&&... args)
    {
      std::unique_ptr<T> u = octave::make_unique<T> (get_module (), *this,
                                                     std::forward<Args> (args)...);
      T *ret = u.get ();
      m_body.emplace_back (std::move (u));
      invalidate_leaf_cache ();
      return ret;
    }

    //
    // virtual from ir_component
    //

    ir_basic_block *get_entry_block (void) override;

    //
    // virtual from ir_structure
    //

    // public
    link_iter pred_begin (ir_component *c) override;
    link_iter pred_end   (ir_component *c) override;
    link_iter succ_begin (ir_component *c) override;
    link_iter succ_end   (ir_component *c) override;
    void invalidate_leaf_cache (void) noexcept override;

    // protected
    bool is_leaf_component (ir_component *c) noexcept override;
    void generate_leaf_cache (void) override;

  protected:

    comp_citer must_find (ir_component *c);

  private:

    comp_list m_body;

    std::pair<ir_component *, comp_citer> m_find_cache = {nullptr, { }};

  };

  class ir_subsequence : public ir_sequence
  {
  public:

    template <typename T>
    static ir_subsequence create (ir_module& mod, ir_structure& parent);

    ir_subsequence (ir_module& mod, ir_structure& parent)
      : ir_sequence (mod, init_wrapper<ir_basic_block>{}),
        m_parent (parent)
    { }

    ~ir_subsequence () noexcept override;

    //
    // virtual from ir_structure
    //

    link_iter pred_begin (ir_component *c) override;
    link_iter pred_end   (ir_component *c) override;
    link_iter succ_begin (ir_component *c) override;
    link_iter succ_end   (ir_component *c) override;
    void invalidate_leaf_cache (void) noexcept override;

  protected:

    template <typename T>
    ir_subsequence (ir_module& mod, ir_structure& parent, init_wrapper<T>)
      : ir_sequence (mod, init_wrapper<T>{}),
        m_parent (parent)
    { }

  private:

    ir_structure& m_parent;

  };

  class ir_fork_component : public ir_structure
  {
  public:

    ir_fork_component (ir_module& mod, ir_structure& parent)
      : ir_structure (mod),
        m_parent (parent),
        m_condition (mod, parent)
    { }

    ~ir_fork_component () noexcept override;

    link_iter pred_begin (ir_component *c) override;
    link_iter pred_end   (ir_component *c) override;
    link_iter succ_begin (ir_component *c) override;
    link_iter succ_end   (ir_component *c) override;

    ir_basic_block *get_entry_block (void) noexcept override;

    bool is_leaf_component (ir_component *c) noexcept override;
    void generate_leaf_cache (void) override;

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

    ir_loop_component (ir_module& mod, ir_structure& parent)
      : ir_structure (mod),
        m_parent (parent),
        m_entry (mod, *this),
        m_body (mod, *this),
        m_condition (mod, *this),
        m_exit (mod, *this),
        m_cond_preds { &m_entry, get_update_block () },
        m_succ_cache { m_body.get_entry_block (), &m_exit }
    { }

    ~ir_loop_component (void) noexcept override;

    link_iter pred_begin (ir_component *c) override;
    link_iter pred_end (ir_component *c) override;
    link_iter succ_begin (ir_component *c) override;
    link_iter succ_end (ir_component *c) override;

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
      leaf_push_back (&m_condition);
    }

    ir_basic_block *get_update_block (void) const noexcept
    {
      return static_cast<ir_basic_block *> (m_body.back ().get ());
    }

  private:

    link_iter cond_succ_begin (void);

    link_iter cond_succ_end (void);

    ir_structure& m_parent;

    ir_basic_block m_entry;

    ir_subsequence m_body;

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

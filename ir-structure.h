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

  class block;
  class ir_component;
  class ir_module;

  class ir_structure : public ir_component
  {
  public:
    ir_structure (ir_module& mod, ir_structure *parent)
      : ir_component (mod, parent)
    { }
  
    ~ir_structure (void) override = 0;

    virtual link_iter pred_begin (ir_component *c)  = 0;
    virtual link_iter pred_end (ir_component *c) = 0;

    virtual void invalidate_leaf_cache (void) noexcept { }
    
  private:
  };

  class ir_component_sequence : public ir_structure
  {
  public:

    ir_component_sequence (ir_module& mod, ir_structure& parent)
      : ir_structure (mod, &parent)
    { }

    bool empty (void) const { return m_body.empty (); }

    comp_citer begin (void) const noexcept { return m_body.begin (); }
    comp_citer end (void) const noexcept { return m_body.end (); }

    comp_cref front (void) const { return m_body.front (); }
    comp_cref back (void) const { return m_body.back (); }

    comp_citer last (void) const
    {
      if (m_body.empty ())
        throw ir_exception ("Component sequence was empty.");
      return --end ();
    }

    comp_citer find (ir_component *blk) const;

    link_iter pred_begin (ir_component *c) override;
    link_iter pred_end (ir_component *c) override;

    link_iter leaf_begin (void) noexcept override;
    link_citer leaf_begin (void) const noexcept override;
    link_iter leaf_end (void) noexcept override;
    link_citer leaf_end (void) const noexcept override;

    template <typename T, typename ...Args>
    void emplace_back (Args&&... args);

    std::stack<ir_component *>
    block_sequence (ir_component *before) override
    {
      std::stack<ir_component *> seq;
      for (comp_cref comp : m_body)
        {
          if (comp.get () == before)
            return seq;
          seq.push (comp.get ());
        }
      if (before != nullptr)
        throw ir_exception ("component not found in the parent.");
      return seq;
    }

    friend class ir_module;

  private:

    ir_component_sequence (ir_module& mod)
      : ir_structure (mod, nullptr)
    { }

    comp_list m_body;

  };

  class ir_fork_component : public ir_structure
  {
  public:

    ir_fork_component (ir_module& mod, ir_structure& parent)
      : ir_structure (mod, &parent),
        m_condition (mod, parent)
    { }

    link_iter pred_begin (ir_component *c) override;
    link_iter pred_end (ir_component *c) override;

    void invalidate_leaf_cache (void) noexcept override;
    void generate_leaf_cache (void);

    link_iter leaf_begin (void) override;
    link_citer leaf_begin (void) const noexcept override;
    link_iter leaf_end (void) override;
    link_citer leaf_end (void) const noexcept override;

    std::stack<ir_component *>
    block_sequence (ir_component *before) override
    {
      std::stack<ir_component *> seq;

      if (before == &m_condition)
        return seq;

      seq.push (&m_condition);

      if (before == nullptr)
        {
          for (comp_ref comp : m_subcomponents)
            seq.push (comp.get ());
          return seq;
        }

      // make sure 'before' is in the block list; this might be removed later.
      for (comp_cref comp : m_subcomponents)
        {
          if (comp.get () == before)
            return seq;
        }
      throw ir_exception ("component not found in the parent.");
    }

  private:

    ir_condition_block m_condition;
    comp_list m_subcomponents;

    link_vec m_leaf_cache;

  };

  //!
  class ir_loop_component : public ir_structure
  {
  public:

    ir_loop_component (ir_module& mod, ir_structure& parent)
      : ir_structure (mod, &parent),
        m_entry (mod, *this),
        m_body (mod, *this),
        m_update (mod, *this),
        m_condition (mod, *this)
    {
      m_leaf.push_back (&m_condition);

      m_cond_preds.push_back (&m_entry);
      m_cond_preds.push_back (&m_update);
    }

    link_iter pred_begin (ir_component *c) override;
    link_iter pred_end (ir_component *c) override;

    link_iter leaf_begin (void) noexcept override;
    link_citer leaf_begin (void) const noexcept override;
    link_iter leaf_end (void) noexcept override;
    link_citer leaf_end (void) const noexcept override;

    std::stack<ir_component *>
    block_sequence (ir_component *before) override
    {
      std::stack<ir_component *> seq;

      if (before == nullptr)
        {
          seq.push (&m_condition);
          seq.push (&m_body);
          seq.push (&m_update);
        }
      else if (before == &m_condition)
        {
          seq.push (&m_body);
          seq.push (&m_update);
        }
      else if (before == &m_body)
        {
          seq.push (&m_update);
          seq.push (&m_condition);
        }
      else if (before == &m_update)
        {
          seq.push (&m_condition);
          seq.push (&m_body);
        }
      else
        throw ir_exception ("component not found in the parent.");

      return seq;

    }

  private:

    ir_basic_block m_entry;

    ir_component_sequence m_body;

    // the loop update block; always unconditionally branches back to
    // condition block
    ir_basic_block m_update;


    // preds: entry, update
    // succ: body, exit
    ir_loop_condition_block m_condition;

    link_vec m_leaf;

    link_vec m_cond_preds;

  };

  class ir_block_stack
  {

  public:
    ir_block_stack (ir_basic_block& bl)
    {
      m_super_stack.push ({});
      m_super_stack.top ().push (&bl);
      pop ();
    }

    ir_basic_block * top (void)
    {
      if (m_super_stack.empty ())
        return nullptr;

      std::stack<ir_component *>& top_stack = m_super_stack.top ();

      if (top_stack.empty ())
        throw ir_exception ("Top of the block super-stack was unexpectedly empty");

      ir_basic_block *ret = dynamic_cast<ir_basic_block *> (top_stack.top ());
      if (ret == nullptr)
        throw ir_exception ("Top of the block stack was not ir_basic_block.");
      return ret;
    }

    void pop (void)
    {
      if (m_super_stack.empty ())
        return;

      std::stack<ir_component *>& top_stack = m_super_stack.top ();

      if (top_stack.empty ())
        throw ir_exception ("Top of the block stack was unexpectedly empty");

      if (m_super_stack.size () == 1 && top_stack.size () == 1)
        {
          ir_component *last = top_stack.top ();
          top_stack.pop ();
          for (ir_component *parent = last->get_parent ();
               parent != nullptr && top_stack.empty ();
               last = parent, parent = last->get_parent ())
            {
              top_stack = std::move (parent->block_sequence (last));
            }
        }
      else if (! top_stack.empty ())
        top_stack.pop ();

      if (top_stack.empty ())
        {
          m_super_stack.pop ();

          // pop the parent from the previous stack
          pop ();

          // return to avoid running expand_to_next unnecessarily
          return;
        }

      expand_to_next ();

    }

  private:

    void expand_to_next (void)
    {
      if (m_super_stack.empty ())
        return;

      std::stack<ir_component *>& top_stack = m_super_stack.top ();

      if (top_stack.empty ())
        throw ir_exception ("Top of the block stack was unexpectedly empty");

      if (! isa<ir_basic_block>(top_stack.top ()))
        {
          m_super_stack.emplace (top_stack.top ()->block_sequence ());
          expand_to_next ();
        }

    }

//    class ir_block_node
//    {
//    public:
//
//      ir_block_node (ir_block_node *parent, const ir_component& component)
//        : m_parent (parent),
//          m_component (component)
//      { }
//
//      ir_component * top (void)
//      {
//        return m_child_stack.top ();
//      }
//
//      void pop (void)
//      {
//        return m_child_stack.pop ();
//      }
//
//    private:
//      ir_block_node *m_parent;
//
//      const ir_component& m_component;
//
//      // lazy init
//      std::stack<ir_component *> m_child_stack;
//
//      std::unique_ptr<ir_block_node> m_curr_subnode = nullptr;
//
//    };

    // lazy init
    std::stack<std::stack<ir_component *>> m_super_stack;
  };

}

#endif

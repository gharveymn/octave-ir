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

  class ir_block;
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

      [[nodiscard]] constexpr
      ir_block&
      get_block (void) const noexcept
      {
        return *m_block;
      }

      [[nodiscard]] constexpr
      ir_def_timeline&
      get_timeline (void) const noexcept
      {
        return *m_timeline;
      }

      [[nodiscard]] constexpr
      bool
      has_timeline (void) const noexcept
      {
        return m_timeline.has_value ();
      }

    private:
      nonnull_ptr<ir_block> m_block;
      optional_ref<ir_def_timeline>  m_timeline;
    };

    nonnull_ptr<ir_block> m_target;
    nonnull_ptr<ir_block> m_join_at;
    std::vector<incoming_pair>  m_incoming;
  };

  class ir_structure : public ir_component
  {
  public:
    static constexpr struct construct_with_parent_tag { } construct_with_parent { };

    ir_structure            (void)                    = delete;
    ir_structure            (const ir_structure&)     = default;
    ir_structure            (ir_structure&&) noexcept = default;
    ir_structure& operator= (const ir_structure&)     = default;
    ir_structure& operator= (ir_structure&&) noexcept = default;
    ~ir_structure           (void)  override          = 0;

    explicit
    ir_structure (construct_with_parent_tag, ir_structure& parent)
      : ir_component (parent)
    { }

    explicit
    ir_structure (nullopt_t)
      : ir_component (nullopt)
    { }

    virtual
    link_vector
    get_preds (ir_component& c) = 0;

    virtual
    link_vector
    get_succs (ir_component& c) = 0;

    [[nodiscard]]
    virtual
    const ir_component_handle&
    get_handle (const ir_component& c) const = 0;

    [[nodiscard]]
    ir_component_handle&
    get_handle (ir_component& c) const
    {
      return const_cast<ir_component_handle&> (get_handle (const_cast<const ir_component&> (c)));
    }

    // mutate a component inside a structure to a different type of component
    template <typename T>
    T&
    mutate (ir_component_handle& handle)
    {
      return *(handle = make_ir_component<T> (std::move (*handle)));
    }

    virtual
    ir_use_timeline&
    join_incoming (ir_block& block, ir_def_timeline& dt) = 0;

    ir_use_timeline&
    join_incoming (ir_block& block, ir_variable& var);

    virtual
    void
    invalidate_leaf_cache (void) noexcept = 0;

    virtual
    bool
    is_leaf_component (const ir_component& c) noexcept = 0;

    [[nodiscard]]
    const link_vector&
    get_leaves (void) override;

  protected:
    void
    clear_leaf_cache (void) noexcept
    {
      m_leaf_cache.clear ();
    }

    [[nodiscard]] constexpr
    bool
    leaf_cache_empty (void) const noexcept
    {
      return m_leaf_cache.empty ();
    }

    [[nodiscard]] constexpr
    link_vector&
    get_leaf_cache (void)
    {
      return m_leaf_cache;
    }

    virtual
    void
    generate_leaf_cache (void) = 0;

    void
    leaves_append (ir_block& blk)
    {
      m_leaf_cache.emplace_back (blk);
    }

    void
    leaves_append_range (link_citer first, link_citer last)
    {
      m_leaf_cache.insert (m_leaf_cache.end (), first, last);
    }

    template <typename Component, typename ...Args,
              typename = std::enable_if_t<is_component<Component>::value>>
    ir_component_handle
    create_component (Args&&... args)
    {
      return make_ir_component<Component> (*this, std::forward<Args> (args)...);
    }

  private:
    link_vector m_leaf_cache;
  };

  class ir_sequence : public ir_structure
  {
  public:
    using component_container     = std::vector<ir_component_handle>;
    using iterator                = typename component_container::iterator;
    using const_iterator          = typename component_container::const_iterator;
    using reverse_iterator        = typename component_container::reverse_iterator;
    using const_reverse_iterator  = typename component_container::const_reverse_iterator;
    using reference               = typename component_container::reference;
    using const_reference         = typename component_container::const_reference;
    using value_type              = typename component_container::value_type;
    using allocator_type          = typename component_container::allocator_type;
    using size_type               = typename component_container::size_type;
    using difference_type         = typename component_container::difference_type;

    using iter    = iterator;
    using citer   = const_iterator;
    using riter   = reverse_iterator;
    using criter  = const_reverse_iterator;
    using ref     = reference;
    using cref    = const_reference;
    using val_t   = value_type;
    using alloc_t = allocator_type;
    using size_t  = size_type;
    using diff_t  = difference_type;

  private:
    class find_cache
    {
    public:

      find_cache            (void)                  = default;
      find_cache            (const find_cache&)     = default;
      find_cache            (find_cache&&) noexcept = default;
      find_cache& operator= (const find_cache&)     = default;
      find_cache& operator= (find_cache&&) noexcept = default;
      ~find_cache           (void)                  = default;

      explicit
      find_cache (iter it) noexcept
        : m_it (it)
      { }

      void
      emplace (iter it) noexcept
      {
        m_it = it;
      }

      [[nodiscard]] constexpr
      bool
      contains (const ir_component& c) const noexcept
      {
        return &c == *m_it;
      }

      [[nodiscard]]
      iter
      get (void) const noexcept
      {
        return m_it;
      }

    private:
      iter m_it;
    };

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

    template <typename Entry, typename ...Args>
    ir_sequence (std::in_place_type_t<Entry>, ir_structure& parent, Args&&... args)
      : ir_structure (construct_with_parent, parent)
    {
      m_body.push_back (create_component<Entry> (std::forward<Args> (args)...));
    }

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

    void
    reset (void) noexcept override
    {
      m_body.erase (++m_body.begin (), m_body.end ());
      front ()->reset ();
      m_find_cache.emplace (begin ());
      invalidate_leaf_cache ();
    }

    [[nodiscard]]
    iter
    find (const ir_component& c)
    {
      if (m_find_cache.contains (c))
        return m_find_cache.get ();

      iter found = std::find (m_body.begin (), m_body.end (), c);

      if (found != end ())
        m_find_cache.emplace (found);

      return found;
    }

    [[nodiscard]]
    citer
    find (const ir_component& c) const
    {
      return const_cast<ir_sequence *> (this)->find (c);
    }

    [[nodiscard]]
    const ir_component_handle&
    get_handle (const ir_component& c) const override;

    template <typename Component, typename ...Args,
              typename = std::enable_if_t<is_component<Component>::value>>
    citer
    emplace (citer cit, Args&&... args)
    {
      return m_body.emplace (cit, create_component<Component> (std::forward<Args> (args)...));
    }

    template <typename Component, typename ...Args,
              typename = std::enable_if_t<is_component<Component>::value>>
    Component&
    emplace_back (Args&&... args)
    {
      m_body.push_back (create_component<Component> (std::forward<Args> (args)...));
      invalidate_leaf_cache ();
      return *back ();
    }

    template <typename T, typename S>
    T&
    emplace_instruction_before (ir_block& blk,
                                std::initializer_list<std::reference_wrapper<S>>,
                                ir_variable& var)
    {


      // find latest def before this one if this is not at the very end

    }

    template <typename T>
    T&
    emplace_instruction_before (ir_block& blk, std::initializer_list<ir_operand>);

    //
    // virtual from ir_component
    //

    ir_block&
    get_entry_block (void) noexcept override
    {
      return front ()->get_entry_block ();
    }

    //
    // virtual from ir_structure
    //

    ir_block&
    split (ir_block& blk, ir_instruction_iter pivot) //override
    {
      auto  ret_it = emplace_before<ir_block> (must_find (blk));
      auto& ret    = static_cast<ir_block&> (**ret_it);

      // transfer instructions which occur after the pivot
      blk.split (pivot, ret);
    }

    // protected
    bool
    is_leaf_component (const ir_component& c) noexcept override
    {
      return c == back ();
    }

    void
    generate_leaf_cache (void) override;

    [[nodiscard]]
    std::list<nonnull_ptr<ir_def>>
    get_latest_defs (ir_variable& var) noexcept override
    {
      for (auto rit = rbegin (); rit != rend (); ++rit)
      {
        if (auto ret = (*rit)->get_latest_defs (var); ! ret.empty ())
          return ret;
      }
      return { };
    }

    //
    // virtual from ir_structure
    //

    void
    invalidate_leaf_cache (void) noexcept override;

    [[nodiscard]]
    ir_function&
    get_function (void) noexcept override
    {
      return get_parent ().get_function ();
    }

  protected:

    citer must_find (const ir_component& c)
    {
      if (citer cit = find (c); cit != end ())
        return cit;
      throw ir_exception ("Component not found in the structure.");
    }

  private:
    component_container m_body;
    find_cache          m_find_cache;
  };

  class ir_fork_component : public ir_structure
  {
  public:
    using component_container     = std::vector<ir_component_handle>;
    using iterator                = typename component_container::iterator;
    using const_iterator          = typename component_container::const_iterator;
    using reverse_iterator        = typename component_container::reverse_iterator;
    using const_reverse_iterator  = typename component_container::const_reverse_iterator;
    using reference               = typename component_container::reference;
    using const_reference         = typename component_container::const_reference;
    using value_type              = typename component_container::value_type;
    using allocator_type          = typename component_container::allocator_type;
    using size_type               = typename component_container::size_type;
    using difference_type         = typename component_container::difference_type;

    using iter    = iterator;
    using citer   = const_iterator;
    using riter   = reverse_iterator;
    using criter  = const_reverse_iterator;
    using ref     = reference;
    using cref    = const_reference;
    using val_t   = value_type;
    using alloc_t = allocator_type;
    using size_t  = size_type;
    using diff_t  = difference_type;

    ir_fork_component            (void)                         = delete;
    ir_fork_component            (const ir_fork_component&)     = delete;
    ir_fork_component            (ir_fork_component&&) noexcept = delete;
    ir_fork_component& operator= (const ir_fork_component&)     = delete;
    ir_fork_component& operator= (ir_fork_component&&) noexcept = delete;
    ~ir_fork_component           (void)       noexcept override;

    explicit ir_fork_component (ir_structure& parent)
      : ir_structure (parent),
        m_condition (create_component<ir_condition_block> (*this))
    { }

    ir_block&
    get_entry_block (void) noexcept override
    {
      return m_condition->get_entry_block ();
    }

    bool
    is_leaf_component (const ir_component& c) noexcept override
    {
      return &c != m_condition;
    }

    void
    generate_leaf_cache (void) override;

    void
    invalidate_leaf_cache (void) noexcept override;

    void
    reset (void) noexcept override
    {
      m_subcomponents.clear ();
    }

    ir_function&
    get_function (void) noexcept override
    {
      return get_parent ().get_function ();
    }

    [[nodiscard]] constexpr
    bool
    is_condition (const ir_component& c) const noexcept
    {
      return &c == m_condition.get ();
    }

    [[nodiscard]]
    std::list<nonnull_ptr<ir_def>>
    get_latest_defs (ir_variable& var) noexcept override
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
    std::list<nonnull_ptr<ir_def>>
    get_latest_defs_before (ir_variable& var, component_handle comp) override
    {
      if (comp->get () == &m_condition)
        return { };
      return m_condition.get_latest_defs (var);
    }

  private:

    link_iter sub_entry_begin (void);
    link_iter sub_entry_end   (void);

    void generate_sub_entry_cache (void);

    ir_component_handle m_condition;
    component_container m_subcomponents;

    // holds entry blocks for subcomponents
    link_vector m_sub_entry_cache;
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

    explicit
    ir_loop_component (ir_structure& parent);

    // link_iter preds_begin (ir_component& c) override;
    // link_iter preds_end   (ir_component& c) override;
    // link_iter succs_begin (ir_component& c) override;
    // link_iter succs_end   (ir_component& c) override;

    [[nodiscard]]
    link_vector
    get_leaves (void) override
    {
      return m_condition->get_leaves ();
    }

    ir_block&
    get_entry_block (void) noexcept override
    {
      return m_entry;
    }

    bool
    is_leaf_component (const ir_component& c) noexcept override
    {
      return &c == m_condition;
    }

    void
    generate_leaf_cache (void) override
    {
      throw ir_exception ("this should not run");
    }

    ir_block&
    get_update_block (void) const noexcept
    {
      return static_cast<ir_block&> (*m_body.back ());
    }

    void
    invalidate_leaf_cache (void) noexcept override
    {
      if (! leaf_cache_empty ())
        {
          clear_leaf_cache ();
          if (is_leaf_component (*this))
            get_parent ().invalidate_leaf_cache ();
        }
    }

    [[nodiscard]]
    ir_function&
    get_function (void) noexcept override
    {
      return get_parent ().get_function ();
    }

    [[nodiscard]] constexpr
    bool
    is_entry (const ir_component& c) const noexcept
    {
      return &c == m_entry;
    }

    [[nodiscard]] constexpr
    bool
    is_condition (const ir_component& c) const noexcept
    {
      return &c == m_condition;
    }

    [[nodiscard]] constexpr
    bool
    is_body (const ir_component& c) const noexcept
    {
      return &c == m_body;
    }

    [[nodiscard]] constexpr
    bool
    is_exit (const ir_component& c) const noexcept
    {
      return &c == m_exit;
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

    ir_component_handle m_entry;
    ir_component_handle m_body;
    ir_component_handle m_condition; // preds: entry, update | succs: body, exit
    ir_component_handle m_exit;

    // does not change
    link_vector m_cond_preds;

    link_vector m_succ_cache;

  };

}

#endif

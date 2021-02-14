/** ir-sequence.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_COMPONENT_SEQUENCE_HPP
#define OCTAVE_IR_IR_COMPONENT_SEQUENCE_HPP

#include "ir-structure.hpp"
#include "ir-instruction-fwd.hpp"

namespace gch
{

  class ir_component_sequence;

  template <>
  struct ir_subcomponent_type_t<ir_component_sequence>
  {
    explicit ir_subcomponent_type_t (void) = default;
  };

  class ir_component_sequence
    : public ir_substructure
  {
  public:
    using component_container = std::vector<ir_component_storage>;
    using container_iter      = typename component_container::iterator;
    using container_citer     = typename component_container::const_iterator;
    using container_riter     = typename component_container::reverse_iterator;
    using container_criter    = typename component_container::const_reverse_iterator;
    using container_ref       = typename component_container::reference;
    using container_cref      = typename component_container::const_reference;
    using container_val_t     = typename component_container::value_type;
    using container_alloc_t   = typename component_container::allocator_type;
    using container_size_t    = typename component_container::size_type;
    using container_diff_t    = typename component_container::difference_type;

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

      explicit
      find_cache (ir_component_handle it) noexcept
        : m_handle (it)
      { }

      void
      emplace (ir_component_handle it) noexcept
      {
        m_handle = it;
      }

      [[nodiscard]] constexpr
      bool
      contains (const ir_component& c) const noexcept
      {
        return &c == m_handle;
      }

      [[nodiscard]]
      ir_component_handle
      get (void) const noexcept
      {
        return m_handle;
      }

    private:
      ir_component_handle m_handle;
    };

    [[nodiscard]]
    container_iter
    get_iter (const cptr comp)
    {
      return std::next (m_body.begin (), std::distance (cbegin (), comp));
    }

    [[nodiscard]]
    container_citer
    get_iter (const cptr comp) const
    {
      return as_mutable (*this).get_iter (comp);
    }

    [[nodiscard]]
    ptr
    make_mutable (const cptr comp)
    {
      return std::next (begin (), std::distance (cbegin (), comp));
    }

  public:
    ir_component_sequence (void)                                        = delete;
    ir_component_sequence (const ir_component_sequence&)                = delete;
    ir_component_sequence (ir_component_sequence&&) noexcept            = default;
    ir_component_sequence& operator= (const ir_component_sequence&)     = delete;
    ir_component_sequence& operator= (ir_component_sequence&&) noexcept = default;
    ~ir_component_sequence (void) override                              = default;

    template <typename Entry, typename ...Args>
    ir_component_sequence (ir_structure& parent, ir_subcomponent_type_t<Entry>, Args&&... args)
      : ir_substructure { parent },
        m_body          { create_component<Entry> (std::forward<Args> (args)...) },
        m_find_cache    { make_handle (begin ()) }
    { }

    template <typename ...Args,
              std::enable_if_t<std::conjunction_v<
                std::is_same<ir_component_mover, std::decay_t<Args>>...>> * = nullptr>
    ir_component_sequence (ir_structure& parent, ir_component_mover m, Args&&... args)
      : ir_substructure { parent },
        m_body          { m, args... },
        m_find_cache    { make_handle (begin ()) }
    { }

    container_iter   container_begin   (void)       noexcept { return m_body.begin ();   }
    container_citer  container_begin   (void) const noexcept { return m_body.begin ();   }
    container_citer  container_cbegin  (void) const noexcept { return m_body.cbegin ();  }

    container_iter   container_end     (void)       noexcept { return m_body.end ();     }
    container_citer  container_end     (void) const noexcept { return m_body.end ();     }
    container_citer  container_cend    (void) const noexcept { return m_body.cend ();    }

    container_riter  container_rbegin  (void)       noexcept { return m_body.rbegin ();  }
    container_criter container_rbegin  (void) const noexcept { return m_body.rbegin ();  }
    container_criter container_crbegin (void) const noexcept { return m_body.crbegin (); }

    container_riter  container_rend    (void)       noexcept { return m_body.rend ();    }
    container_criter container_rend    (void) const noexcept { return m_body.rend ();    }
    container_criter container_crend   (void) const noexcept { return m_body.crend ();   }

    container_ref    container_front   (void)       noexcept { return m_body.front ();   }
    container_cref   container_front   (void) const noexcept { return m_body.front ();   }

    container_ref    container_back    (void)       noexcept { return m_body.back ();    }
    container_cref   container_back    (void) const noexcept { return m_body.back ();    }

    [[nodiscard]]
    ptr
    begin (void) noexcept
    {
      return make_ptr (m_body.begin ());
    }

    [[nodiscard]]
    cptr
    begin (void) const noexcept
    {
      return make_ptr (m_body.begin ());
    }

    [[nodiscard]]
    cptr
    cbegin (void) const noexcept
    {
      return begin ();
    }

    [[nodiscard]]
    ptr
    end (void) noexcept
    {
      return make_ptr (m_body.end ());
    }

    [[nodiscard]]
    cptr
    end (void) const noexcept
    {
      return make_ptr (m_body.end ());
    }

    [[nodiscard]]
    cptr
    cend (void) const noexcept
    {
      return end ();
    }

    [[nodiscard]]
    rptr
    rbegin (void) noexcept
    {
      return std::make_reverse_iterator (end ());
    }

    [[nodiscard]]
    crptr
    rbegin (void) const noexcept
    {
      return std::make_reverse_iterator (end ());
    }

    [[nodiscard]]
    crptr
    crbegin (void) const noexcept
    {
      return rbegin ();
    }

    [[nodiscard]]
    rptr
    rend (void) noexcept
    {
      return std::make_reverse_iterator (begin ());
    }

    [[nodiscard]]
    crptr
    rend (void) const noexcept
    {
      return std::make_reverse_iterator (begin ());
    }

    [[nodiscard]]
    crptr
    crend (void) const noexcept
    {
      return rend ();
    }

    [[nodiscard]]
    size_ty
    size (void) const noexcept
    {
      assert (m_body.size () != 0 && "sequence should not be empty");
      return m_body.size ();
    }

    // sequences cannot be empty
    [[nodiscard]]
    bool
    empty (void) const noexcept = delete;

    [[nodiscard]]
    ptr
    last (void) noexcept
    {
      return std::prev (end ());
    }

    [[nodiscard]]
    cptr
    last (void) const noexcept
    {
      return std::prev (end ());
    }

    [[nodiscard]]
    cptr
    clast (void) const noexcept
    {
      return last ();
    }

    template <typename Component = ir_component>
    [[nodiscard]]
    Component& front (void) noexcept
    {
      return static_cast<Component&> (*m_body.front ());
    }

    template <typename Component = ir_component>
    [[nodiscard]]
    const Component& front (void) const noexcept
    {
      return static_cast<const Component&> (*m_body.front ());
    }

    template <typename Component = ir_component>
    [[nodiscard]]
    Component& back (void) noexcept
    {
      return static_cast<Component&> (*m_body.back ());
    }

    template <typename Component = ir_component>
    [[nodiscard]]
    const Component& back (void) const noexcept
    {
      return static_cast<const Component&> (*m_body.back ());
    }

    [[nodiscard]]
    ptr
    find (ir_component& c) const;

    [[nodiscard]]
    cptr
    find (const ir_component& c) const
    {
      return find (as_mutable (c));
    }

    template <typename Component, typename ...Args,
              typename = std::enable_if_t<is_component<Component>::value>>
    cptr
    emplace_at (cptr pos, Args&&... args)
    {
      if (pos == end ())
      {
        emplace_back (std::forward<Args> (args)...);
        return last ();
      }
      const container_citer cit = get_iter (pos);
      return m_body.emplace (cit, create_component<Component> (std::forward<Args> (args)...));
    }

    template <typename Component, typename ...Args,
              typename = std::enable_if_t<is_component<Component>::value>>
    Component&
    emplace_back (Args&&... args)
    {
      m_body.push_back (create_component<Component> (std::forward<Args> (args)...));
      invalidate_leaf_cache ();
      return back<Component> ();
    }

    // returns one-past-end of inserted elements
    ptr
    flatten_element (ptr pos);

    // returns one-past-end of inserted elements
    ptr
    flatten_range (ptr first, cptr last);

    void
    flatten (void);

    //
    // virtual from ir_component
    //

    bool
    reassociate_timelines (const ir_link_set<ir_def_timeline>& old_dts, ir_def_timeline& new_dt,
                           std::vector<nonnull_ptr<ir_block>>& until) override;

    void
    reset (void) noexcept override;

    //
    // virtual from ir_structure
    //

    [[nodiscard]]
    ir_component_ptr
    get_ptr (ir_component& c) const override;

    [[nodiscard]]
    ir_component_ptr
    get_entry_ptr (void) override;

    [[nodiscard]]
    ir_link_set<ir_block>
    get_predecessors (ir_component_cptr comp) override;

    [[nodiscard]]
    ir_link_set<ir_block>
    get_successors (ir_component_cptr comp) override;

    [[nodiscard]]
    bool
    is_leaf (ir_component_cptr comp) noexcept override;

    void
    generate_leaf_cache (void) override;

    ir_use_timeline&
    join_incoming_at (ir_component_ptr pos, ir_def_timeline& dt) override;

    void
    recursive_flatten (void) override;

  protected:
    cptr
    must_find (const ir_component& c)
    {
      if (cptr cit = find (c); cit != end ())
        return cit;
      throw ir_exception ("Component not found in the structure.");
    }

  private:
    std::vector<ir_component_mover>&
    recursive_collect_components (std::vector<ir_component_mover>& collector);

    std::vector<ir_component_mover>
    recursive_collect_components (std::vector<ir_component_mover>&& collector = { })
    {
      return std::move (recursive_collect_components (collector));
    }

    component_container m_body;
    mutable find_cache  m_find_cache;
  };

}

#endif // OCTAVE_IR_IR_COMPONENT_SEQUENCE_HPP

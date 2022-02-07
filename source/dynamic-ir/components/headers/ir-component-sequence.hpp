/** ir-sequence.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_SEQUENCE_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_SEQUENCE_HPP

#include "ir-structure.hpp"
#include "ir-instruction-fwd.hpp"

namespace gch
{

  class ir_component_sequence final
    : public ir_substructure,
      public visitable<ir_component_sequence, consolidated_visitors_t<ir_substructure>>
  {
  public:
    using container_type = std::vector<ir_component_storage>;
    using iter           = ir_component_ptr;
    using citer          = ir_component_cptr;
    using riter          = std::reverse_iterator<iter>;
    using criter         = std::reverse_iterator<citer>;
    using ref            = ir_subcomponent&;
    using cref           = const ir_subcomponent&;
    using val_t          = typename container_type::value_type;
    using alloc_t        = typename container_type::allocator_type;
    using size_ty        = typename container_type::size_type;
    using diff_ty        = typename container_type::difference_type;

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
      find_cache (ir_component_handle it) noexcept;

      void
      emplace (ir_component_handle it) noexcept;

      [[nodiscard]]
      bool
      contains (const ir_subcomponent& sub) const noexcept;

      [[nodiscard]]
      ir_component_handle
      get (void) const noexcept;

    private:
      ir_component_handle m_handle;
    };

    [[nodiscard]]
    iter
    make_mutable (const citer cit);

    [[nodiscard]]
    typename container_type::iterator
    get_container_iter (iter it) const;

    [[nodiscard]]
    typename container_type::const_iterator
    get_container_iter (citer cit) const;

  public:
    ir_component_sequence (void)                                        = delete;
    ir_component_sequence (const ir_component_sequence&)                = delete;
    ir_component_sequence (ir_component_sequence&&) noexcept            = default;
    ir_component_sequence& operator= (const ir_component_sequence&)     = delete;
    ir_component_sequence& operator= (ir_component_sequence&&) noexcept = default;
    ~ir_component_sequence (void) override;

    template <typename Entry, typename ...Args,
              std::enable_if_t<std::is_constructible_v<Entry, ir_structure&, Args...>> * = nullptr>
    ir_component_sequence (ir_structure& parent, ir_subcomponent_type_t<Entry>, Args&&... args)
      : ir_substructure { parent },
        m_find_cache    { make_handle (end ()) }
    {
      m_body.push_back (allocate_subcomponent<Entry> (std::forward<Args> (args)...));
      m_find_cache.emplace (make_handle (begin ()));
    }

    explicit
    ir_component_sequence (ir_structure& parent);

    ir_component_sequence (ir_structure& parent, std::initializer_list<ir_component_mover> init);

    ir_component_sequence (ir_structure& parent, ir_component_mover init);

    [[nodiscard]]
    iter
    begin (void) noexcept;

    [[nodiscard]]
    citer
    begin (void) const noexcept;

    [[nodiscard]]
    citer
    cbegin (void) const noexcept;

    [[nodiscard]]
    iter
    end (void) noexcept;

    [[nodiscard]]
    citer
    end (void) const noexcept;

    [[nodiscard]]
    citer
    cend (void) const noexcept;

    [[nodiscard]]
    riter
    rbegin (void) noexcept;

    [[nodiscard]]
    criter
    rbegin (void) const noexcept;

    [[nodiscard]]
    criter
    crbegin (void) const noexcept;

    [[nodiscard]]
    riter
    rend (void) noexcept;

    [[nodiscard]]
    criter
    rend (void) const noexcept;

    [[nodiscard]]
    criter
    crend (void) const noexcept;

    [[nodiscard]]
    ref
    front (void);

    [[nodiscard]]
    cref
    front (void) const;

    [[nodiscard]]
    ref
    back (void);

    [[nodiscard]]
    cref
    back (void) const;

    [[nodiscard]]
    size_ty
    size (void) const noexcept;

    bool
    empty (void) = delete;

    [[nodiscard]]
    iter
    last (void) noexcept;

    [[nodiscard]]
    citer
    last (void) const noexcept;

    [[nodiscard]]
    citer
    clast (void) const noexcept;

    template <typename Component = ir_subcomponent>
    [[nodiscard]]
    Component& front (void) noexcept
    {
      return static_cast<Component&> (*m_body.front ());
    }

    template <typename Component = ir_subcomponent>
    [[nodiscard]]
    const Component& front (void) const noexcept
    {
      return static_cast<const Component&> (*m_body.front ());
    }

    template <typename Component = ir_subcomponent>
    [[nodiscard]]
    Component& back (void) noexcept
    {
      return static_cast<Component&> (*m_body.back ());
    }

    template <typename Component = ir_subcomponent>
    [[nodiscard]]
    const Component& back (void) const noexcept
    {
      return static_cast<const Component&> (*m_body.back ());
    }

    [[nodiscard]]
    iter
    find (ir_subcomponent& sub) const;

    [[nodiscard]]
    citer
    find (const ir_subcomponent& sub) const
    {
      return find (as_mutable (sub));
    }

    template <typename Component, typename ...Args,
              typename = std::enable_if_t<is_ir_component_v<Component>>>
    citer
    emplace_at (citer pos, Args&&... args)
    {
      if (pos == end ())
      {
        emplace_back (std::forward<Args> (args)...);
        return last ();
      }

      iter ret = m_body.emplace (
        pos,
        allocate_subcomponent<Component> (std::forward<Args> (args)...));

      m_find_cache.emplace (make_handle (ret));

      return ret;
    }

    template <typename Component, typename ...Args,
              typename = std::enable_if_t<is_ir_component_v<Component>>>
    Component&
    emplace_back (Args&&... args)
    {
      m_body.push_back (allocate_subcomponent<Component> (std::forward<Args> (args)...));
      invalidate_leaf_cache ();
      m_find_cache.emplace (make_handle (std::prev (end ())));
      return back<Component> ();
    }

    // returns one-past-end of inserted elements
    iter
    flatten_element (iter pos);

    // returns one-past-end of inserted elements
    iter
    flatten_range (iter first, citer last);

    void
    flatten_level (void);

    void
    recursive_flatten (void);

  protected:
    citer
    must_find (const ir_subcomponent& c);

  private:
    std::vector<ir_component_mover>&
    recursive_collect_components (std::vector<ir_component_mover>& collector);

    std::vector<ir_component_mover>
    recursive_collect_components (void);

    container_type     m_body;
    mutable find_cache m_find_cache;
  };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_SEQUENCE_HPP

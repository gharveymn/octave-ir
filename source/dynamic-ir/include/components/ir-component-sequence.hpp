/** ir-sequence.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_COMPONENT_SEQUENCE_HPP
#define OCTAVE_IR_IR_COMPONENT_SEQUENCE_HPP

#include "ir-structure.hpp"
#include "values/ir-instruction-fwd.hpp"

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
      contains (const ir_subcomponent& sub) const noexcept
      {
        return &sub == m_handle;
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
    iter
    make_mutable (const citer cit)
    {
      return std::next (begin (), std::distance (cbegin (), cit));
    }

    [[nodiscard]]
    typename container_type::iterator
    get_container_iter (iter it) const
    {
      return std::next (as_mutable (*this).m_body.begin (),
                        std::distance (as_mutable (*this).begin (), it));
    }

    [[nodiscard]]
    typename container_type::const_iterator
    get_container_iter (citer cit) const
    {
      return std::next (m_body.cbegin (), std::distance (cbegin (), cit));
    }

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

    ir_component_sequence (ir_structure& parent, std::initializer_list<ir_component_mover> init);

    [[nodiscard]]
    iter
    begin (void) noexcept
    {
      return make_ptr (m_body.begin ());
    }

    [[nodiscard]]
    citer
    begin (void) const noexcept
    {
      return as_mutable (*this).begin ();
    }

    [[nodiscard]]
    citer
    cbegin (void) const noexcept
    {
      return begin ();
    }

    [[nodiscard]]
    iter
    end (void) noexcept
    {
      return make_ptr (m_body.end ());
    }

    [[nodiscard]]
    citer
    end (void) const noexcept
    {
      return as_mutable (*this).end ();
    }

    [[nodiscard]]
    citer
    cend (void) const noexcept
    {
      return end ();
    }

    [[nodiscard]]
    riter
    rbegin (void) noexcept
    {
      return riter { end () };
    }

    [[nodiscard]]
    criter
    rbegin (void) const noexcept
    {
      return as_mutable (*this).rbegin ();
    }

    [[nodiscard]]
    criter
    crbegin (void) const noexcept
    {
      return rbegin ();
    }

    [[nodiscard]]
    riter
    rend (void) noexcept
    {
      return riter { begin () };
    }

    [[nodiscard]]
    criter
    rend (void) const noexcept
    {
      return as_mutable (*this).rend ();
    }

    [[nodiscard]]
    criter
    crend (void) const noexcept
    {
      return rend ();
    }

    [[nodiscard]]
    ref
    front (void)
    {
      return *begin ();
    }

    [[nodiscard]]
    cref
    front (void) const
    {
      return as_mutable (*this).front ();
    }

    [[nodiscard]]
    ref
    back (void)
    {
      return *rbegin ();
    }

    [[nodiscard]]
    cref
    back (void) const
    {
      return as_mutable (*this).back ();
    }

    [[nodiscard]]
    size_ty
    size (void) const noexcept
    {
     return m_body.size ();
    }

    // sequences cannot be empty
    [[nodiscard]]
    bool
    empty (void) = delete;

    [[nodiscard]]
    iter
    last (void) noexcept
    {
      return std::prev (end ());
    }

    [[nodiscard]]
    citer
    last (void) const noexcept
    {
      return as_mutable (*this).last ();
    }

    [[nodiscard]]
    citer
    clast (void) const noexcept
    {
      return last ();
    }

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
              typename = std::enable_if_t<is_ir_component<Component>::value>>
    citer
    emplace_at (citer pos, Args&&... args)
    {
      if (pos == end ())
      {
        emplace_back (std::forward<Args> (args)...);
        return last ();
      }
      return m_body.emplace (pos, allocate_subcomponent<Component> (std::forward<Args> (args)...));
    }

    template <typename Component, typename ...Args,
              typename = std::enable_if_t<is_ir_component<Component>::value>>
    Component&
    emplace_back (Args&&... args)
    {
      m_body.push_back (allocate_subcomponent<Component> (std::forward<Args> (args)...));
      invalidate_leaf_cache ();
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

#endif // OCTAVE_IR_IR_COMPONENT_SEQUENCE_HPP

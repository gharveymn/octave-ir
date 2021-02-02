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
  class ir_component_sequence : public ir_structure
  {
  public:
    using component_container     = std::vector<ir_component_storage>;
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
    explicit ir_component_sequence (nullopt_t)
      : ir_structure (nullopt)
    { }

  public:
    ir_component_sequence (void)                                        = delete;
    ir_component_sequence (const ir_component_sequence&)                = delete;
    ir_component_sequence (ir_component_sequence&&) noexcept            = default;
    ir_component_sequence& operator= (const ir_component_sequence&)     = delete;
    ir_component_sequence& operator= (ir_component_sequence&&) noexcept = default;
    ~ir_component_sequence (void) override                              = default;

    template <typename Entry, typename ...Args>
    ir_component_sequence (ir_structure& parent,
                           std::in_place_type_t<Entry> = std::in_place_type<ir_block>,
                           Args&&... args)
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

    [[nodiscard]]
    iter
    find (const ir_component& c);

    [[nodiscard]]
    citer
    find (const ir_component& c) const
    {
      return const_cast<ir_component_sequence *> (this)->find (c);
    }

    [[nodiscard]]
    iter
    get_pos (ir_component_handle comp);

    [[nodiscard]]
    citer
    get_pos (ir_component_handle comp) const
    {
      return const_cast<ir_component_sequence *> (this)->get_pos (comp);
    }

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

    ir_component_handle
    split (ir_component_handle block_handle, ir_instruction_iter pivot);

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
    // virtual from ir_component
    //

    void
    reset (void) noexcept override;

    //
    // virtual from ir_structure
    //

    [[nodiscard]]
    ir_component_handle
    get_entry_component (void) override;

    [[nodiscard]]
    ir_component_handle
    get_handle (const ir_component& c) const override;

    link_vector
    get_preds (ir_component_handle comp) override;

    link_vector
    get_succs (ir_component_handle comp) override;

    ir_use_timeline&
    join_incoming_at (ir_component_handle& block_handle, ir_def_timeline& dt) override;

    void
    generate_leaf_cache (void) override;

    [[nodiscard]]
    bool
    is_leaf_component (ir_component_handle comp) noexcept override;

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
}

#endif // OCTAVE_IR_IR_COMPONENT_SEQUENCE_HPP

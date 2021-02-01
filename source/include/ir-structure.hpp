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

#include <gch/small_vector.hpp>

#include "ir-component.hpp"

#include <vector>

namespace gch
{

  class ir_block;
  class ir_condition_block;
  class ir_loop_condition_block;
  class ir_function;
  class ir_use_timeline;
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

    nonnull_ptr<ir_block>      m_target;
    nonnull_ptr<ir_block>      m_join_at;
    std::vector<incoming_pair> m_incoming;
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

    [[nodiscard]] auto  leaves_begin   (void)       noexcept { return get_leaves ().begin ();   }
    [[nodiscard]] auto  leaves_begin   (void) const noexcept { return get_leaves ().begin ();   }
    [[nodiscard]] auto  leaves_cbegin  (void) const noexcept { return get_leaves ().cbegin ();  }

    [[nodiscard]] auto  leaves_end     (void)       noexcept { return get_leaves ().end ();     }
    [[nodiscard]] auto  leaves_end     (void) const noexcept { return get_leaves ().end ();     }
    [[nodiscard]] auto  leaves_cend    (void) const noexcept { return get_leaves ().cend ();    }

    [[nodiscard]] auto  leaves_rbegin  (void)       noexcept { return get_leaves ().rbegin ();  }
    [[nodiscard]] auto  leaves_rbegin  (void) const noexcept { return get_leaves ().rbegin ();  }
    [[nodiscard]] auto  leaves_crbegin (void) const noexcept { return get_leaves ().crbegin (); }

    [[nodiscard]] auto  leaves_rend    (void)       noexcept { return get_leaves ().rend ();    }
    [[nodiscard]] auto  leaves_rend    (void) const noexcept { return get_leaves ().rend ();    }
    [[nodiscard]] auto  leaves_crend   (void) const noexcept { return get_leaves ().crend ();   }

    [[nodiscard]] auto& leaves_front   (void)       noexcept { return get_leaves ().front ();   }
    [[nodiscard]] auto& leaves_front   (void) const noexcept { return get_leaves ().front ();   }

    [[nodiscard]] auto& leaves_back    (void)       noexcept { return get_leaves ().back ();    }
    [[nodiscard]] auto& leaves_back    (void) const noexcept { return get_leaves ().back ();    }

    [[nodiscard]] auto  leaves_size    (void) const noexcept { return get_leaves ().size ();    }
    [[nodiscard]] auto  leaves_empty   (void) const noexcept { return get_leaves ().empty ();   }

  public:
    [[nodiscard]]
    const link_vector&
    get_leaves (void);

    [[nodiscard]]
    const link_vector&
    get_leaves (void) const
    {
      return const_cast<ir_structure *> (this)->get_leaves ();
    }

    // mutate a component inside a structure to a different type of component
    template <typename T>
    T&
    mutate (ir_component_handle& handle)
    {
      return *(handle = make_ir_component<T> (std::move (*handle)));
    }

    void
    invalidate_leaf_cache (void) noexcept;

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

    void
    leaves_append (const ir_component_handle& comp);

    template <typename Component, typename ...Args,
              typename = std::enable_if_t<is_component<Component>::value>>
    ir_component_handle
    create_component (Args&&... args)
    {
      return make_ir_component<Component> (*this, std::forward<Args> (args)...);
    }

    //
    // virtual functions
    //

  public:
    [[nodiscard]]
    virtual
    const ir_component_handle&
    get_entry_component (void) = 0;

    [[nodiscard]]
    virtual
    const ir_component_handle&
    get_handle (const ir_component& c) const = 0;

    virtual
    link_vector
    get_preds (const ir_component_handle& comp) = 0;

    virtual
    link_vector
    get_succs (const ir_component_handle& comp) = 0;

    virtual
    ir_use_timeline&
    join_incoming_at (ir_component_handle& block_handle, ir_def_timeline& dt) = 0;

    virtual
    void
    generate_leaf_cache (void) = 0;

    [[nodiscard]]
    virtual
    bool
    is_leaf_component (const ir_component_handle& comp) noexcept = 0;

    //
    // virtual function accessories
    //

    [[nodiscard]]
    ir_component_handle&
    get_handle (ir_component& c) const
    {
      return const_cast<ir_component_handle&> (get_handle (const_cast<const ir_component&> (c)));
    }

    ir_use_timeline&
    join_incoming_at (ir_block& block, ir_variable& var);

    link_vector
    get_preds (const ir_component& c)
    {
      return get_preds (get_handle (c));
    }

    link_vector
    get_succs (const ir_component& c)
    {
      return get_succs (get_handle (c));
    }

    bool
    is_leaf_component (const ir_component& c) noexcept
    {
      return is_leaf_component (get_handle (c));
    }

    //
    // functions related to virtual functions
    //

    [[nodiscard]]
    bool
    is_entry_component (const ir_component_handle& comp) noexcept
    {
      return comp == get_entry_component ();
    }

    [[nodiscard]]
    bool
    is_entry_component (const ir_component& c) noexcept
    {
      return &c == get_entry_component ();
    }

    [[nodiscard]]
    virtual
    ir_block&
    get_entry_block (void) noexcept;

  private:
    link_vector m_leaf_cache;
  };

  [[nodiscard]]
  ir_block&
  get_entry_block (ir_structure& s);

  [[nodiscard]] inline
  const ir_block&
  get_entry_block (const ir_structure& s)
  {
    return get_entry_block (const_cast<ir_structure&> (s));
  }

  [[nodiscard]]
  ir_structure::link_vector
  copy_leaves (const ir_component_handle& comp);

  template <typename ...Args>
  [[nodiscard]] inline
  ir_structure::link_vector
  copy_leaves (const ir_component_handle& comp, Args&&... args)
  {
    auto concatenate = [](ir_structure::link_vector& l, ir_structure::link_vector&& r)
                       { l.insert (l.end (), r.begin (), r.end ()); };

    ir_structure::link_vector ret = copy_leaves (comp);
    (concatenate (ret, copy_leaves (std::forward<Args> (args))), ...);
    return std::move (ret);
  }

}

#endif

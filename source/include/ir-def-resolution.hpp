/** ir-def-resolution.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_DEF_RESOLUTION_HPP
#define OCTAVE_IR_IR_DEF_RESOLUTION_HPP

#include "ir-link-set.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>

#include <stack>
#include <vector>

namespace gch
{

  class ir_block;
  class ir_def_timeline;
  class ir_use_timeline;
  class ir_def_resolution_stack;

  class ir_def_resolution_frame
  {
  public:
    using substack_ptr   = std::unique_ptr<ir_def_resolution_stack>;

    ir_def_resolution_frame            (void)                               = delete;
    ir_def_resolution_frame            (const ir_def_resolution_frame&)     = default;
    ir_def_resolution_frame            (ir_def_resolution_frame&&) noexcept = default;
    ir_def_resolution_frame& operator= (const ir_def_resolution_frame&)     = default;
    ir_def_resolution_frame& operator= (ir_def_resolution_frame&&) noexcept = default;
    ~ir_def_resolution_frame           (void)                               = default;

    explicit
    ir_def_resolution_frame (ir_block& join_block, const ir_link_set<ir_block>& v);

    [[nodiscard]] auto  begin   (void)       noexcept { return m_incoming.begin ();   }
    [[nodiscard]] auto  begin   (void) const noexcept { return m_incoming.begin ();   }
    [[nodiscard]] auto  cbegin  (void) const noexcept { return m_incoming.cbegin ();  }

    [[nodiscard]] auto  end     (void)       noexcept { return m_incoming.end ();     }
    [[nodiscard]] auto  end     (void) const noexcept { return m_incoming.end ();     }
    [[nodiscard]] auto  cend    (void) const noexcept { return m_incoming.cend ();    }

    [[nodiscard]] auto  rbegin  (void)       noexcept { return m_incoming.rbegin ();  }
    [[nodiscard]] auto  rbegin  (void) const noexcept { return m_incoming.rbegin ();  }
    [[nodiscard]] auto  crbegin (void) const noexcept { return m_incoming.crbegin (); }

    [[nodiscard]] auto  rend    (void)       noexcept { return m_incoming.rend ();    }
    [[nodiscard]] auto  rend    (void) const noexcept { return m_incoming.rend ();    }
    [[nodiscard]] auto  crend   (void) const noexcept { return m_incoming.crend ();   }

    [[nodiscard]] auto& front   (void)       noexcept { return m_incoming.front ();   }
    [[nodiscard]] auto& front   (void) const noexcept { return m_incoming.front ();   }

    [[nodiscard]] auto& back    (void)       noexcept { return m_incoming.back ();    }
    [[nodiscard]] auto& back    (void) const noexcept { return m_incoming.back ();    }

    [[nodiscard]] auto  size    (void) const noexcept { return m_incoming.size ();    }
    [[nodiscard]] auto  empty   (void) const noexcept { return m_incoming.empty ();   }

    [[nodiscard]]
    bool
    is_joinable (void) const noexcept;

    [[nodiscard]]
    ir_link_set<ir_def_timeline>
    join (void);

    [[nodiscard]]
    ir_link_set<ir_def_timeline>
    join_with (ir_link_set<ir_def_timeline>&& c);

  private:
    nonnull_ptr<ir_block>     m_join_block;
    std::vector<substack_ptr> m_incoming;
  };

  class ir_def_resolution_stack
  {
  public:
    [[nodiscard]]
    bool
    is_resolvable (void) const noexcept;

    [[nodiscard]]
    const ir_link_set<ir_def_timeline>&
    resolve (void);

    [[nodiscard]]
    const ir_link_set<ir_def_timeline>&
    resolve_with (ir_link_set<ir_def_timeline> c);

    [[nodiscard]]
    bool
    is_resolved (void) const noexcept;

    [[nodiscard]]
    bool
    is_resolved_nonempty (void) const noexcept;

    [[nodiscard]]
    const ir_link_set<ir_def_timeline>&
    get_resolution (void) const noexcept;

    [[nodiscard]]
    bool
    check_all_same_nonnull_outgoing (void) const;

    [[nodiscard]]
    ir_def&
    get_resolved_def (void) const;

    [[nodiscard]]
    optional_ref<ir_def>
    maybe_get_resolved_def (void) const;

  private:
    nonnull_ptr<ir_block>                       m_leaf_block;
    std::stack<ir_def_resolution_frame>         m_stack;
    std::optional<ir_link_set<ir_def_timeline>> m_resolution;
  };

}

#endif // OCTAVE_IR_IR_DEF_RESOLUTION_HPP

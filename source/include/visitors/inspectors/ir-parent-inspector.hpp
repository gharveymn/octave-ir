/** ir-parent-inspector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_PARENT_INSPECTOR_HPP
#define OCTAVE_IR_IR_PARENT_INSPECTOR_HPP

#include "utilities/ir-link-set.hpp"
#include "visitors/ir-visitor.hpp"

namespace gch
{
  class ir_component;

  class ir_subcomponent;
  class ir_block;

  class ir_structure;
  class ir_function;

  class ir_substructure;
  class ir_component_fork;
  class ir_component_loop;
  class ir_component_sequence;

  template <typename Result>
  class ir_parent_inspector
  {
  public:
    ir_parent_inspector            (void)                           = default;
    ir_parent_inspector            (const ir_parent_inspector&)     = default;
    ir_parent_inspector            (ir_parent_inspector&&) noexcept = default;
    ir_parent_inspector& operator= (const ir_parent_inspector&)     = default;
    ir_parent_inspector& operator= (ir_parent_inspector&&) noexcept = default;
    ~ir_parent_inspector           (void)                           = default;

  protected:
    const ir_subcomponent&
    set_subcomponent (const ir_subcomponent& sub) noexcept
    {
      return *(m_subcomponent = &sub);
    }

    const ir_subcomponent&
    get_subcomponent (void) noexcept
    {
      return *m_subcomponent;
    }

    void
    set_result (Result&& result) noexcept
    {
      m_result = std::move (result);
    }

    Result&&
    release_result (void) noexcept
    {
      return std::move (m_result);
    }

  private:
    const ir_subcomponent * m_subcomponent;
    Result                  m_result;
  };

  class ir_predecessor_collector
    : private ir_parent_inspector<ir_link_set<ir_block>>
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    ir_predecessor_collector            (void)                                = default;
    ir_predecessor_collector            (const ir_predecessor_collector&)     = delete;
    ir_predecessor_collector            (ir_predecessor_collector&&) noexcept = delete;
    ir_predecessor_collector& operator= (const ir_predecessor_collector&)     = delete;
    ir_predecessor_collector& operator= (ir_predecessor_collector&&) noexcept = delete;
    ~ir_predecessor_collector           (void)                                = default;

    ir_link_set<ir_block>
    operator() (const ir_subcomponent& sub);

  private:
    void
    visit (const ir_component_fork& fork);

    void
    visit (const ir_component_loop& loop);

    void
    visit (const ir_component_sequence& seq);

    void
    visit (const ir_function& func);
  };

  ir_link_set<ir_block>
  get_predecessors (const ir_subcomponent& c);

  class ir_successor_collector
    : private ir_parent_inspector<ir_link_set<ir_block>>
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    ir_successor_collector            (void)                              = default;
    ir_successor_collector            (const ir_successor_collector&)     = delete;
    ir_successor_collector            (ir_successor_collector&&) noexcept = delete;
    ir_successor_collector& operator= (const ir_successor_collector&)     = delete;
    ir_successor_collector& operator= (ir_successor_collector&&) noexcept = delete;
    ~ir_successor_collector           (void)                              = default;

    ir_link_set<ir_block>
    operator() (const ir_subcomponent& sub);

  private:
    void
    visit (const ir_component_fork& fork);

    void
    visit (const ir_component_loop& loop);

    void
    visit (const ir_component_sequence& seq);

    void
    visit (const ir_function& func);
  };

  ir_link_set<ir_block>
  get_successors (const ir_subcomponent& sub);

  class ir_leaf_inspector
    : private ir_parent_inspector<bool>
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    ir_leaf_inspector            (void)                         = default;
    ir_leaf_inspector            (const ir_leaf_inspector&)     = delete;
    ir_leaf_inspector            (ir_leaf_inspector&&) noexcept = delete;
    ir_leaf_inspector& operator= (const ir_leaf_inspector&)     = delete;
    ir_leaf_inspector& operator= (ir_leaf_inspector&&) noexcept = delete;
    ~ir_leaf_inspector           (void)                         = default;

    bool
    operator() (const ir_subcomponent& sub);

  private:
    void
    visit (const ir_component_fork& fork);

    void
    visit (const ir_component_loop& loop);

    void
    visit (const ir_component_sequence& seq);

    void
    visit (const ir_function& func);
  };

  bool
  is_leaf (const ir_subcomponent& sub);

}



#endif // OCTAVE_IR_IR_PARENT_INSPECTOR_HPP

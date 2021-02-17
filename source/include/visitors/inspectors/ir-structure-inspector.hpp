/** ir-structure-inspector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_STRUCTURE_INSPECTOR_HPP
#define OCTAVE_IR_IR_STRUCTURE_INSPECTOR_HPP

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

  class ir_entry_collector
  {
  public:
    template <typename, typename>
    friend struct acceptor;

    ir_entry_collector            (void)                          = default;
    ir_entry_collector            (const ir_entry_collector&)     = delete;
    ir_entry_collector            (ir_entry_collector&&) noexcept = delete;
    ir_entry_collector& operator= (const ir_entry_collector&)     = delete;
    ir_entry_collector& operator= (ir_entry_collector&&) noexcept = delete;
    ~ir_entry_collector           (void)                          = default;

    const ir_subcomponent&
    operator() (const ir_structure& s);

  protected:
    void
    visit (const ir_component_fork& fork);

    void
    visit (const ir_component_loop& loop);

    void
    visit (const ir_component_sequence& seq);

    void
    visit (const ir_function& func);

  private:
    const ir_subcomponent *m_result;
  };

  ir_subcomponent&
  get_entry_component (ir_structure& s);

  const ir_subcomponent&
  get_entry_component (const ir_structure& s);

  ir_block&
  get_entry_block (ir_structure& s);

  const ir_block&
  get_entry_block (const ir_structure& s);

}

#endif // OCTAVE_IR_IR_STRUCTURE_INSPECTOR_HPP

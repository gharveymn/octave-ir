/** ir-structure-flattener.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_STRUCTURE_FLATTENER_HPP
#define OCTAVE_IR_IR_STRUCTURE_FLATTENER_HPP

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

  class ir_structure_flattener
  {
    public:
    template <typename, typename>
    friend struct acceptor;

    ir_structure_flattener            (void)                              = default;
    ir_structure_flattener            (const ir_structure_flattener&)     = delete;
    ir_structure_flattener            (ir_structure_flattener&&) noexcept = delete;
    ir_structure_flattener& operator= (const ir_structure_flattener&)     = delete;
    ir_structure_flattener& operator= (ir_structure_flattener&&) noexcept = delete;
    ~ir_structure_flattener           (void)                              = default;

    void
    operator() (ir_structure& s);

  private:
    void
    visit (ir_component_fork& fork);

    void
    visit (ir_component_loop& loop);

    void
    visit (ir_component_sequence& seq);

    void
    visit (ir_function& func);

    void
    maybe_recurse (ir_subcomponent& sub);
  };

  void
  flatten (ir_structure& s);

}

#endif // OCTAVE_IR_IR_STRUCTURE_FLATTENER_HPP

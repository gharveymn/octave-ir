/** ir-structure-flattener.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_FLATTENER_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_FLATTENER_HPP

#include "ir-structure-mutators-fwd.hpp"

namespace gch
{

  class ir_structure_flattener
    : public visitor_traits<ir_structure_flattener>
  {
    public:
    friend acceptor_type<ir_component_fork>;
    friend acceptor_type<ir_component_loop>;
    friend acceptor_type<ir_component_sequence>;
    friend acceptor_type<ir_function>;

    ir_structure_flattener            (void)                              = default;
    ir_structure_flattener            (const ir_structure_flattener&)     = delete;
    ir_structure_flattener            (ir_structure_flattener&&) noexcept = delete;
    ir_structure_flattener& operator= (const ir_structure_flattener&)     = delete;
    ir_structure_flattener& operator= (ir_structure_flattener&&) noexcept = delete;
    ~ir_structure_flattener           (void)                              = default;

    void
    operator() (ir_structure& s) const;

  private:
    void
    visit (ir_component_fork& fork) const;

    void
    visit (ir_component_loop& loop) const;

    void
    visit (ir_component_sequence& seq) const;

    void
    visit (ir_function& func) const;

    void
    maybe_recurse (ir_subcomponent& sub) const;
  };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_STRUCTURE_FLATTENER_HPP

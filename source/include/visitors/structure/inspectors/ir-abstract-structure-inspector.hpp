/** ir-abstract-structure-inspector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_ABSTRACT_STRUCTURE_INSPECTOR_HPP
#define OCTAVE_IR_IR_ABSTRACT_STRUCTURE_INSPECTOR_HPP

#include "ir-structure-inspectors-fwd.hpp"
#include "visitors/ir-abstract-inspector.hpp"
#include "visitors/ir-visitor.hpp"

namespace gch
{

  class ir_abstract_structure_inspector
    : public visitor_traits<ir_abstract_structure_inspector>,
      public abstract_inspector<ir_component_fork,
                                ir_component_loop,
                                ir_component_sequence,
                                ir_function>
  {
  public:
    ir_abstract_structure_inspector            (void)                                       = default;
    ir_abstract_structure_inspector            (const ir_abstract_structure_inspector&)     = default;
    ir_abstract_structure_inspector            (ir_abstract_structure_inspector&&) noexcept = default;
    ir_abstract_structure_inspector& operator= (const ir_abstract_structure_inspector&)     = default;
    ir_abstract_structure_inspector& operator= (ir_abstract_structure_inspector&&) noexcept = default;
    ~ir_abstract_structure_inspector           (void) override                              = 0;
  };

}

#endif // OCTAVE_IR_IR_ABSTRACT_STRUCTURE_INSPECTOR_HPP

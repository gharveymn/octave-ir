/** ir-visitor-fwd.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_VISITOR_FWD_HPP
#define OCTAVE_IR_IR_VISITOR_FWD_HPP

#include "utilities/ir-common.hpp"

namespace gch
{

  template <typename ...Visitors>
  struct visitor_types;

  //
  // component visitors
  //

  class ir_def_resolution_build_descender;
  class ir_def_resolution_builder;
  class ir_leaf_collector;

  using component_inspector_types = inspector_types<ir_leaf_collector>;

  using component_mutator_types = mutator_types<ir_def_resolution_build_descender,
                                                ir_def_resolution_builder>

  // aggregate
  using ir_component_visitors = visitor_types<component_inspector_types>;

  //
  // structure visitors
  //

  class ir_predecessor_collector;
  class ir_successor_collector;
  class ir_leaf_inspector;

  using parent_inspector_types = inspector_types<ir_predecessor_collector,
                                                 ir_successor_collector,
                                                 ir_leaf_inspector>;

  class ir_entry_collector;

  using structure_inspector_types = inspector_types<ir_entry_collector>;

  class ir_structure_flattener;

  using structure_mutator_types = mutator_types<ir_structure_flattener>;

  // aggregate
  using ir_structure_visitors = visitor_types<ir_component_visitors,
                                              structure_inspector_types,
                                              parent_inspector_types,
                                              structure_mutator_types>;

}

#endif // OCTAVE_IR_IR_VISITOR_FWD_HPP

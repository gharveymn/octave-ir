/** ir-visitor-fwd.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_VISITOR_FWD_HPP
#define OCTAVE_IR_IR_VISITOR_FWD_HPP

#include "components/ir-component-fwd.hpp"
#include "utilities/ir-common.hpp"

namespace gch
{
  template <typename T>
  class ir_link_set;

  template <typename ...Visitors>
  struct visitor_types;

  //
  // component visitors
  //

  class ir_leaf_collector;

  using component_inspector_types = visitor_types<inspector<const ir_leaf_collector,
                                                            ir_link_set<ir_block>>>;

  class ir_def_resolution_build_result;
  class ir_def_resolution_build_descender;
  class ir_def_resolution_build_ascender;

  using component_mutator_types = visitor_types<mutator<const ir_def_resolution_build_descender,
                                                        ir_def_resolution_build_result>,
                                                mutator<const ir_def_resolution_build_ascender,
                                                        ir_def_resolution_build_result>>;

  // aggregate
  using ir_component_visitors = visitor_types<component_inspector_types,
                                              component_mutator_types>;

  //
  // structure visitors
  //

  class ir_predecessor_collector;
  class ir_successor_collector;
  class ir_leaf_inspector;

  using parent_inspector_types = visitor_types<inspector<const ir_predecessor_collector,
                                                         ir_link_set<ir_block>>,
                                               inspector<const ir_successor_collector,
                                                         ir_link_set<ir_block>>,
                                               inspector<const ir_leaf_inspector,
                                                         bool>>;

  class ir_entry_collector;

  using structure_inspector_types = visitor_types<inspector<const ir_entry_collector,
                                                            ir_subcomponent&>>;

  class ir_structure_flattener;

  using structure_mutator_types = visitor_types<mutator<ir_structure_flattener>>;

  // aggregate
  using ir_structure_visitors = visitor_types<ir_component_visitors,
                                              structure_inspector_types,
                                              parent_inspector_types,
                                              structure_mutator_types>;

}

#endif // OCTAVE_IR_IR_VISITOR_FWD_HPP

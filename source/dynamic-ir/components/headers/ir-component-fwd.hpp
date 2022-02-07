/** ir-structure-fwd.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_FWD_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_FWD_HPP

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

  template <typename Subcomponent>
  struct ir_subcomponent_type_t;

  template <>
  struct ir_subcomponent_type_t<ir_block>
  {
    explicit
    ir_subcomponent_type_t (void) = default;
  };

  template <>
  struct ir_subcomponent_type_t<ir_component_fork>
  {
    explicit
    ir_subcomponent_type_t (void) = default;
  };

  template <>
  struct ir_subcomponent_type_t<ir_component_loop>
  {
    explicit
    ir_subcomponent_type_t (void) = default;
  };

  template <>
  struct ir_subcomponent_type_t<ir_component_sequence>
  {
    explicit
    ir_subcomponent_type_t (void) = default;
  };

  template <typename Subcomponent>
  inline constexpr
  ir_subcomponent_type_t<Subcomponent>
  ir_subcomponent_type
  { };

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_COMPONENT_FWD_HPP

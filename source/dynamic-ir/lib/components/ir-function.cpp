/** ir-function.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include "components/ir-function.hpp"

#include "components/ir-block.hpp"
#include "components/ir-component-sequence.hpp"

#include "visitors/ir-all-component-visitors.hpp"
#include "visitors/ir-all-structure-visitors.hpp"

#include "gch/octave-ir-utilities/ir-error.hpp"

#include <gch/nonnull_ptr.hpp>

namespace gch
{
  ir_function::
  ir_function (void)
    : m_body (allocate_subcomponent<ir_component_sequence> (ir_subcomponent_type<ir_block>))
  { }

  ir_function::
  ~ir_function (void) = default;

  ir_subcomponent&
  ir_function::
  get_body (void) noexcept
  {
    return *m_body;
  }

  const ir_subcomponent&
  ir_function::
  get_body (void) const noexcept
  {
    return as_mutable (*this).get_body ();
  }

  bool
  ir_function::
  is_body (const ir_subcomponent& sub) const noexcept
  {
    return &sub == &get_body ();
  }

  ir_variable&
  ir_function::
  get_variable (const variable_key_type& identifier)
  {
    auto [it, inserted] = m_variable_map.try_emplace (identifier, *this, identifier);
    return std::get<ir_variable> (*it);
  }

  ir_variable&
  ir_function::
  get_variable (variable_key_type&& identifier)
  {
    auto [it, inserted] = m_variable_map.try_emplace (identifier, *this, std::move (identifier));
    return std::get<ir_variable> (*it);
  }

  ir_variable&
  ir_function::
  get_variable (const variable_identifier_char_type *identifier)
  {
    auto [it, inserted] = m_variable_map.try_emplace (identifier, *this, identifier);
    return std::get<ir_variable> (*it);
  }

}

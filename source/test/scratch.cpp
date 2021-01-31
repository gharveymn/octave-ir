/** scratch.cpp
 * Short description here.
 *
 * Copyright © 2020 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-component-handle.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>

#include <cassert>
#include <memory>
#include <stack>
#include <vector>

namespace gch
{

  class ir_component { };

  struct ir_def_timeline;
  struct ir_block;

  struct incoming_block
  {
    gch::nonnull_ptr<ir_block> block;
    gch::optional_ref<ir_def_timeline> tl;
  };

  struct incoming_component
  {
    gch::nonnull_ptr<ir_component> comp;
    std::vector<incoming_block> leaves;
  };

  struct phi_manager
  {
    std::vector<incoming_component> incoming;
  };

  struct management_stack
  {
    std::stack<phi_manager> stack;
  };

}

int main (void)
{
  using namespace gch;
  management_stack s;
  ir_component_handle x = make_ir_component<ir_component> ();
  ir_component_handle y = make_ir_component<ir_component> ();

  assert (x != y);

  optional_ref<ir_component> o = *x;
  assert (o == x);
  assert (o != y);

  nonnull_ptr<ir_component> n = *x;
  assert (n == x);
  assert (n != y);

  ir_component *p = x.get ().get_pointer ();
  assert (p == x);
  assert (p != y);

  const ir_component& r = *x;
  assert (r == x);
  assert (r != y);

  assert (nullptr != x);
  assert (nullptr != y);

  return 0;
}

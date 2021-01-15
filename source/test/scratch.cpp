/** scratch.cpp
 * Short description here.
 *
 * Copyright © 2020 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
#include <stack>
#include <vector>
#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>

struct ir_component;
struct ir_def_timeline;
struct ir_basic_block;

struct incoming_block
{
  gch::nonnull_ptr<ir_basic_block> block;
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

int main (void)
{
  management_stack s;
}

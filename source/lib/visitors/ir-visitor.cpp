/** ir-visitor.cpp.c
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "visitors/ir-visitor.hpp"

namespace gch
{

  abstract_acceptor<void>::
  ~abstract_acceptor (void) = default;

  abstract_visitable<void>::
  ~abstract_visitable (void) = default;

}

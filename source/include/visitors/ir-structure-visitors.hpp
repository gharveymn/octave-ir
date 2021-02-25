/** ir-structure-visitors.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_STRUCTURE_VISITORS_HPP
#define OCTAVE_IR_IR_STRUCTURE_VISITORS_HPP

#include "visitors/ir-visitor-fwd.hpp"

#include "visitors/ir-component-visitors.hpp"

#include "visitors/inspectors/ir-entry-collector.hpp"
#include "visitors/inspectors/ir-leaf-inspector.hpp"
#include "visitors/inspectors/ir-predecessor-collector.hpp"
#include "visitors/inspectors/ir-successor-collector.hpp"

#include "visitors/mutators/ir-structure-flattener.hpp"

#endif // OCTAVE_IR_IR_STRUCTURE_VISITORS_HPP

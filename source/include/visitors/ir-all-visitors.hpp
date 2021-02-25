/** ir-all-visitors.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_ALL_VISITORS_HPP
#define OCTAVE_IR_IR_ALL_VISITORS_HPP

#include "visitors/ir-visitor-fwd.hpp"

#include "visitors/inspectors/ir-entry-collector.hpp"
#include "visitors/inspectors/ir-leaf-collector.hpp"
#include "visitors/inspectors/ir-leaf-inspector.hpp"
#include "visitors/inspectors/ir-predecessor-collector.hpp"
#include "visitors/inspectors/ir-subcomponent-inspector.hpp"
#include "visitors/inspectors/ir-successor-collector.hpp"

#include "visitors/mutators/ir-def-propagator.hpp"
#include "visitors/mutators/ir-def-resolution-builder.hpp"
#include "visitors/mutators/ir-forward-mutator.hpp"
#include "visitors/mutators/ir-structure-flattener.hpp"
#include "visitors/mutators/ir-subcomponent-mutator.hpp"

#endif // OCTAVE_IR_IR_ALL_VISITORS_HPP

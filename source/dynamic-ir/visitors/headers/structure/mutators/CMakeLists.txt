target_sources (
  octave-ir.dynamic-ir.visitors
  PRIVATE
    ir-ascending-def-propagator.hpp
    ir-ascending-forward-mutator.hpp
    ir-structure-flattener.hpp
    ir-structure-mutators.hpp
    ir-structure-mutators-fwd.hpp
)

ADD_SUBDIRECTORY (utility)

target_sources (
  octave-ir.dynamic-ir.visitors
  PRIVATE
    ir-ascending-def-propagator.cpp
    ir-ascending-forward-mutator.cpp
    ir-structure-flattener.cpp
)

add_subdirectory (utility)

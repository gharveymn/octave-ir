target_sources (
  octave-ir-dynamic-ir
  PRIVATE
  ir-ascending-def-propagator.cpp
  ir-ascending-def-resolution-builder.cpp
  ir-ascending-forward-mutator.cpp
)

ADD_SUBDIRECTORY (utility)

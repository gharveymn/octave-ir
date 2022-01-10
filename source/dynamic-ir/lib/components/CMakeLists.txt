target_sources (
  octave-ir-dynamic-ir
  PRIVATE
  ir-block.cpp
  ir-component-fork.cpp
  ir-component-loop.cpp
  ir-component-sequence.cpp
  ir-component.cpp
  ir-function.cpp
  ir-structure.cpp
)

ADD_SUBDIRECTORY (linkage)

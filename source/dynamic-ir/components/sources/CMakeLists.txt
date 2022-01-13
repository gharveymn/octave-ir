target_sources (
  octave-ir.dynamic-ir.components
  PRIVATE
    ir-block.cpp
    ir-component-fork.cpp
    ir-component-loop.cpp
    ir-component-sequence.cpp
    ir-component.cpp
    ir-function.cpp
    ir-structure.cpp
)

add_subdirectory (linkage)

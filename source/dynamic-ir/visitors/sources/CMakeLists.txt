target_sources (
  octave-ir.dynamic-ir.visitors
  PRIVATE
    ir-abstract-inspector.cpp
    ir-abstract-mutator.cpp
    ir-visitor.cpp
)

add_subdirectory (component)
add_subdirectory (structure)
add_subdirectory (subcomponent)
add_subdirectory (substructure)

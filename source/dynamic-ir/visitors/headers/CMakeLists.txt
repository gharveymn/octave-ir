target_sources (
  octave-ir.dynamic-ir.visitors
  PRIVATE
    ir-abstract-inspector.hpp
    ir-abstract-mutator.hpp
    ir-all-component-visitors.hpp
    ir-all-structure-visitors.hpp
    ir-all-subcomponent-visitors.hpp
    ir-all-substructure-visitors.hpp
    ir-all-visitors.hpp
    ir-all-visitors-fwd.hpp
    ir-visitor.hpp
    ir-visitor-fwd.hpp
)

add_subdirectory (component)
add_subdirectory (structure)
add_subdirectory (subcomponent)
add_subdirectory (substructure)

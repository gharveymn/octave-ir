target_sources (
  octave-ir.dynamic-ir.visitors
  PRIVATE
    ir-substructure-visitors.hpp
    ir-substructure-visitors-fwd.hpp
)

add_subdirectory (inspectors)
add_subdirectory (mutators)

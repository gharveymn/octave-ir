target_sources (
  octave-ir.dynamic-ir.visitors
  PRIVATE
    ir-structure-visitors.hpp
    ir-structure-visitors-fwd.hpp
)

add_subdirectory (inspectors)
add_subdirectory (mutators)

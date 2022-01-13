target_sources (
  octave-ir.dynamic-ir.visitors
  PRIVATE
    ir-subcomponent-visitors.hpp
    ir-subcomponent-visitors-fwd.hpp
)

add_subdirectory (inspectors)
add_subdirectory (mutators)

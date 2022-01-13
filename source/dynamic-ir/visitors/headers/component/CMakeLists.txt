target_sources (
  octave-ir.dynamic-ir.visitors
  PRIVATE
    ir-component-visitors.hpp
    ir-component-visitors-fwd.hpp
)

add_subdirectory (inspectors)
add_subdirectory (mutators)

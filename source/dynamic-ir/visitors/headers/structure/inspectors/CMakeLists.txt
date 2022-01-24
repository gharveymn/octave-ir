target_sources (
  octave-ir.dynamic-ir.visitors
  PRIVATE
    ir-abstract-structure-inspector.hpp
    ir-ascending-def-resolution-builder.hpp
    ir-entry-collector.hpp
    ir-leaf-inspector.hpp
    ir-predecessor-collector.hpp
    ir-structure-inspectors.hpp
    ir-structure-inspectors-fwd.hpp
    ir-successor-collector.hpp
)

add_subdirectory (utility)

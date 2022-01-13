target_sources (
  octave-ir.dynamic-ir.visitors
  PRIVATE
    ir-abstract-structure-inspector.cpp
    ir-entry-collector.cpp
    ir-leaf-inspector.cpp
    ir-predecessor-collector.cpp
    ir-successor-collector.cpp
)

add_subdirectory (utility)

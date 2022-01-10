target_sources (
  octave-ir-dynamic-ir
  PRIVATE
  ir-entry-collector.hpp
  ir-leaf-inspector.hpp
  ir-predecessor-collector.hpp
  ir-structure-inspectors.hpp
  ir-structure-inspectors-fwd.hpp
  ir-successor-collector.hpp
)

ADD_SUBDIRECTORY (utility)

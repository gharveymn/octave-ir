target_sources (
  octave-ir-dynamic-ir
  PRIVATE
  ir-block.hpp
  ir-block-common.hpp
  ir-component.hpp
  ir-component-fork.hpp
  ir-component-fwd.hpp
  ir-component-loop.hpp
  ir-component-sequence.hpp
  ir-function.hpp
  ir-structure.hpp
)

ADD_SUBDIRECTORY (linkage)
ADD_SUBDIRECTORY (utility)

target_sources (
  octave-ir.dynamic-ir.components
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

add_subdirectory (linkage)
add_subdirectory (utility)

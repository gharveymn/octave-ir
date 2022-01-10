set_property (GLOBAL APPEND PROPERTY OCTAVE_IR_LIBS octave-ir-utilities)

add_library (octave-ir-utilities OBJECT)

octave_ir_target_headers (
  octave-ir-utilities
  PRIVATE
    ir-allocator-util.hpp
    ir-common.hpp
    ir-error.hpp
    ir-function-traits.hpp
    ir-functional.hpp
    ir-index-sequence-map.hpp
    ir-iterator.hpp
    ir-link-set.hpp
    ir-optional-util.hpp
    ir-type-pack.hpp
    ir-type-traits.hpp
    ir-utility.hpp
)

octave_ir_target_sources (
  octave-ir-utilities
  PRIVATE
    ir-error.cpp
    ir-link-set.cpp
    ir-utility.cpp
)

target_link_libraries (
  octave-ir-utilities
  PUBLIC
    gch::octave-ir-common
    gch::nonnull_ptr
    gch::optional_ref
    gch::small_vector
)
target_include_directories (
  octave-ir-utilities
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

add_library (gch::octave-ir-utilities ALIAS octave-ir-utilities)

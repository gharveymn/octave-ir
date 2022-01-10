set_property (GLOBAL APPEND PROPERTY OCTAVE_IR_LIBS octave-ir-compiler)

add_library (octave-ir-compiler INTERFACE)

target_sources (
  octave-ir-compiler
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/gch/octave-ir-compiler.hpp>
)

target_include_directories (
  octave-ir-compiler
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries (
  octave-ir-compiler
  INTERFACE
    gch::octave-ir-static-ir
)

add_library (gch::octave-ir-compiler ALIAS octave-ir-compiler)

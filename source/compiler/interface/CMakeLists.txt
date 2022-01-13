add_library (octave-ir.compiler-interface INTERFACE)

target_sources (
  octave-ir.compiler-interface
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/gch/octave-ir-compiler-interface.hpp>
)

target_include_directories (
  octave-ir.compiler-interface
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries (
  octave-ir.compiler-interface
  INTERFACE
    gch::octave-ir.static-ir
)

add_library (gch::octave-ir.compiler-interface ALIAS octave-ir.compiler-interface)

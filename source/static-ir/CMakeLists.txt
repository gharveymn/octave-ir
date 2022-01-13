add_library (octave-ir.static-ir SHARED)

add_subdirectory (headers)
add_subdirectory (sources)

target_link_libraries (
  octave-ir.static-ir
  PUBLIC
    gch::octave-ir.common
    gch::nonnull_ptr
    gch::optional_ref
    gch::small_vector
)

target_link_libraries (
  octave-ir.static-ir
  PUBLIC
    $<BUILD_INTERFACE:gch::octave-ir.utilities>
)

target_include_directories (
  octave-ir.static-ir
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/headers>
)

add_library (gch::octave-ir.static-ir ALIAS octave-ir.static-ir)

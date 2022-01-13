add_library (octave-ir.utilities OBJECT)

target_link_libraries (
  octave-ir.utilities
  PUBLIC
    $<BUILD_INTERFACE:gch::octave-ir.common>
    $<BUILD_INTERFACE:gch::nonnull_ptr>
    $<BUILD_INTERFACE:gch::optional_ref>
    $<BUILD_INTERFACE:gch::small_vector>
)

target_include_directories (
  octave-ir.utilities
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/headers>
)

add_subdirectory (headers)
add_subdirectory (sources)

add_library (gch::octave-ir.utilities ALIAS octave-ir.utilities)

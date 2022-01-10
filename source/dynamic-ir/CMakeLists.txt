set_property (GLOBAL APPEND PROPERTY OCTAVE_IR_LIBS octave-ir-dynamic-ir)

add_library (octave-ir-dynamic-ir SHARED)

target_link_libraries (
  octave-ir-dynamic-ir
  PUBLIC
    gch::nonnull_ptr
    gch::octave-ir-static-ir
    gch::octave-ir-utilities
    gch::optional_ref
    gch::partition
    gch::select-iterator
    gch::small_vector
    gch::tracker
    plf::plf_list
)

target_link_libraries (
  octave-ir-static-ir
)

target_include_directories (
  octave-ir-dynamic-ir
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

add_subdirectory (include)
add_subdirectory (lib)

add_library (gch::octave-ir-dynamic-ir ALIAS octave-ir-dynamic-ir)

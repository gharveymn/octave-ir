macro (add_dynamic_ir_object_library target_name)
  add_library (${target_name} OBJECT)

  target_include_directories (
    ${target_name}
    PRIVATE
      $<TARGET_PROPERTY:octave-ir.dynamic-ir,INTERFACE_INCLUDE_DIRECTORIES>
  )

  target_link_libraries (
    ${target_name}
    PRIVATE
      gch::nonnull_ptr
      gch::octave-ir.static-ir
      gch::octave-ir.utilities
      gch::optional_ref
      gch::partition
      gch::select-iterator
      gch::small_vector
      gch::tracker
      plf::plf_list
  )

#  export (
#    TARGETS
#      ${target_name}
#    NAMESPACE
#      gch::
#    APPEND FILE
#      ${GCH_OCTAVE_IR_BUILD_EXPORT_FILE}
#  )

  add_library (gch::${target_name} ALIAS ${target_name})

  target_include_directories (
    octave-ir.dynamic-ir
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/headers>
  )

  target_link_libraries (octave-ir.dynamic-ir PRIVATE ${target_name})
endmacro ()

add_library (octave-ir.dynamic-ir SHARED)

target_link_libraries (
  octave-ir.dynamic-ir
  PUBLIC
    gch::nonnull_ptr
    gch::octave-ir.static-ir
    gch::octave-ir.utilities
    gch::optional_ref
    gch::partition
    gch::select-iterator
    gch::small_vector
    gch::tracker
    plf::plf_list
)

add_subdirectory (components)
add_subdirectory (processors)
add_subdirectory (values)
add_subdirectory (visitors)

add_library (gch::octave-ir.dynamic-ir ALIAS octave-ir.dynamic-ir)

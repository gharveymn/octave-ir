set_property (GLOBAL APPEND PROPERTY OCTAVE_IR_LIBS octave-ir-static-ir)

add_library (octave-ir-static-ir SHARED)

octave_ir_target_headers (
  octave-ir-static-ir
  PRIVATE
    ir-constant.hpp
    ir-metadata.hpp
    ir-static-block.hpp
    ir-static-function.hpp
    ir-static-def.hpp
    ir-static-instruction.hpp
    ir-static-operand.hpp
    ir-static-use.hpp
    ir-static-variable.hpp
    ir-type.hpp
    ir-type-util.hpp
)

octave_ir_target_sources (
  octave-ir-static-ir
  PRIVATE
    ir-constant.cpp
    ir-metadata.cpp
    ir-static-block.cpp
    ir-static-function.cpp
    ir-static-def.cpp
    ir-static-instruction.cpp
    ir-static-operand.cpp
    ir-static-use.cpp
    ir-static-variable.cpp
    ir-type.cpp
    ir-type-util.cpp
)

target_link_libraries (
  octave-ir-static-ir
  PUBLIC
    gch::octave-ir-common
    gch::nonnull_ptr
    gch::optional_ref
    gch::small_vector
)

target_link_libraries (
  octave-ir-static-ir
  PUBLIC
    $<BUILD_INTERFACE:gch::octave-ir-utilities>
)

target_include_directories (
  octave-ir-static-ir
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

add_library (gch::octave-ir-static-ir ALIAS octave-ir-static-ir)

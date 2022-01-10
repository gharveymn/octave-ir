option (OCTAVE_IR_LLVM_USE_DEFAULT "Set to ON to link to the default LLVM installation" ON)

set (OCTAVE_IR_LLVM_PATH "" CACHE STRING "A user specified path to an LLVM installation")
set (OCTAVE_IR_LLVM_EXTRA_PATHS "" CACHE STRING "A list of paths to additional LLVM installations")

# Find Octave.
find_package (Octave REQUIRED)

macro (create_llvm_compiler_package target_name)
  add_library (${target_name} SHARED)

  octave_ir_target_sources (
    ${target_name}
    PRIVATE
      function-translator.cpp
      instruction-translator.cpp
      llvm-interface.cpp
      llvm-value-map.cpp
  )

  target_link_libraries (
    ${target_name}
    PRIVATE
      gch::octave-ir-compiler
      gch::octave-ir-static-ir
      LLVMCore
      LLVMSupport
      LLVMOrcJIT
      LLVMJITLink
      LLVMTarget
      LLVMX86AsmParser
      LLVMX86Codegen
      LLVMX86Desc
      LLVMX86Info
      Octave::libinterp
  )

  target_compile_options (
    ${target_name}
    PRIVATE
      $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-undef>
  )

  target_compile_definitions (
    ${target_name}
    PRIVATE
      $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS>
  )

  target_include_directories (${target_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
endmacro ()

if (OCTAVE_IR_LLVM_USE_DEFAULT)
  if (OCTAVE_IR_LLVM_PATH)
    find_package (LLVM REQUIRED CONFIG PATHS "${OCTAVE_IR_LLVM_PATH}" NO_DEFAULT_PATH)
  else ()
    find_package (LLVM REQUIRED CONFIG)
  endif ()
  message (STATUS "Using default backend LLVM-${LLVM_PACKAGE_VERSION} (${LLVM_DIR})")

  create_llvm_compiler_package (octave-ir-compiler-llvm)
endif ()

foreach (path ${OCTAVE_IR_LLVM_PATHS})
  find_package (LLVM REQUIRED CONFIG PATHS "${path}" NO_DEFAULT_PATH)
  message (STATUS "Found LLVM-${LLVM_PACKAGE_VERSION} at ${path}")

  create_llvm_compiler_package (octave-ir-compiler-llvm.LLVM-${LLVM_PACKAGE_VERSION})
endforeach ()

#set_property (GLOBAL APPEND PROPERTY OCTAVE_IR_LIBS octave-ir-compiler-llvm)

#  create_package (AUTHOR gharveymn
#                  NAMESPACE gch
#                  NAME ${target_name}
#                  TYPE SHARED
#
#                  HEADERS instruction-maps/binary/arith/arith-mappers.hpp
#                  instruction-maps/binary/arith/arith-maps.hpp
#                  instruction-maps/binary/cmp/cmp-mappers.hpp
#                  instruction-maps/binary/cmp/cmp-maps.hpp
#                  instruction-maps/binary/binary-maps.hpp
#                  instruction-maps/llvm-instruction-maps.hpp
#                  function-translator.hpp
#                  instruction-translator.hpp
#                  llvm-common.hpp
#                  llvm-constant.hpp
#                  llvm-interface.hpp
#                  llvm-type.hpp
#                  llvm-value-map.hpp
#                  llvm-version.hpp
#
#                  SOURCES function-translator.cpp
#                  instruction-translator.cpp
#                  llvm-interface.cpp
#                  llvm-value-map.cpp
#
#                  DEPENDENCIES octave-ir-compiler
#                  octave-ir-static-ir)

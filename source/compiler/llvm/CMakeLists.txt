set (
  GCH_OCTAVE_IR_LLVM_PATH
  ""
  CACHE PATH
  "Specify the path to an LLVM installation."
)

if (GCH_OCTAVE_IR_LLVM_PATH)
  find_package (LLVM 13 EXACT REQUIRED CONFIG PATHS "${GCH_OCTAVE_IR_LLVM_PATH}" NO_DEFAULT_PATH)
else ()
  find_package (LLVM 13 EXACT REQUIRED CONFIG)
endif ()

add_library (octave-ir.compiler-llvm SHARED)

target_link_libraries (
  octave-ir.compiler-llvm
  PRIVATE
    gch::octave-ir.static-ir
    LLVMCore
    LLVMSupport
    LLVMExecutionEngine
    LLVMOrcJIT
    LLVMInstCombine
    LLVMJITLink
    LLVMScalarOpts
    LLVMTarget
    LLVMX86AsmParser
    LLVMX86CodeGen
    LLVMX86Desc
    LLVMX86Info
  PUBLIC
    gch::octave-ir.compiler-interface
)

target_compile_options (
  octave-ir.compiler-llvm
  PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-undef>
)

target_compile_definitions (
  octave-ir.compiler-llvm
  PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS>
)

target_include_directories (
  octave-ir.compiler-llvm
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/headers
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/interface>
)

if (LLVM_INCLUDE_DIRS)
  target_include_directories (
    octave-ir.compiler-llvm
    SYSTEM PRIVATE
      ${LLVM_INCLUDE_DIRS}
  )
endif ()

target_sources (
  octave-ir.compiler-llvm
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/interface/gch/octave-ir-compiler-llvm.hpp>
)

add_subdirectory (headers)
add_subdirectory (sources)

add_library (gch::octave-ir.compiler-llvm ALIAS octave-ir.compiler-llvm)

set (GCH_OCTAVE_IR_LLVM_PATH "" CACHE STRING "A user specified path to an LLVM installation")

# Find Octave.
find_package (Octave REQUIRED)

if (OCTAVE_IR_LLVM_PATH)
  find_package (LLVM REQUIRED CONFIG PATHS "${OCTAVE_IR_LLVM_PATH}" NO_DEFAULT_PATH)
else ()
  find_package (LLVM REQUIRED CONFIG)
endif ()

add_library (octave-ir.compiler-llvm SHARED)

target_link_libraries (
  octave-ir.compiler-llvm
  PRIVATE
    gch::octave-ir.compiler-interface
    gch::octave-ir.static-ir
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
  octave-ir.compiler-llvm
  PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-undef>
)

target_compile_definitions (
  octave-ir.compiler-llvm
  PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS>
)

target_include_directories (octave-ir.compiler-llvm PRIVATE ${CMAKE_CURRENT_LIST_DIR}/headers)

add_subdirectory (headers)
add_subdirectory (sources)

add_library (gch::octave-ir.compiler-llvm ALIAS octave-ir.compiler-llvm)

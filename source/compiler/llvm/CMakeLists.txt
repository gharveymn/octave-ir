set (GCH_OCTAVE_IR_LLVM_PATH "" CACHE STRING "A user specified path to an LLVM installation")

# Find Octave.
find_package (Octave REQUIRED)

if (OCTAVE_IR_LLVM_PATH)
  find_package (LLVM REQUIRED CONFIG PATHS "${OCTAVE_IR_LLVM_PATH}" NO_DEFAULT_PATH)
else ()
  find_package (LLVM REQUIRED CONFIG)
endif ()

add_library (octave-ir-compiler-llvm SHARED)

octave_ir_target_headers (
  octave-ir-compiler-llvm
  PRIVATE
    instruction-maps/binary/arith/arith-mappers.hpp
    instruction-maps/binary/arith/arith-maps.hpp
    instruction-maps/binary/cmp/cmp-mappers.hpp
    instruction-maps/binary/cmp/cmp-maps.hpp
    instruction-maps/binary/binary-maps.hpp
    instruction-maps/llvm-instruction-maps.hpp
    function-translator.hpp
    instruction-translator.hpp
    llvm-common.hpp
    llvm-constant.hpp
    llvm-interface.hpp
    llvm-type.hpp
    llvm-value-map.hpp
    llvm-version.hpp
)

octave_ir_target_sources (
  octave-ir-compiler-llvm
  PRIVATE
    function-translator.cpp
    instruction-translator.cpp
    llvm-interface.cpp
    llvm-value-map.cpp
)

target_link_libraries (
  octave-ir-compiler-llvm
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
  octave-ir-compiler-llvm
  PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-undef>
)

target_compile_definitions (
  octave-ir-compiler-llvm
  PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS>
)

target_include_directories (octave-ir-compiler-llvm PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

add_library (gch::octave-ir-compiler-llvm ALIAS octave-ir-compiler-llvm)

#set_property (GLOBAL APPEND PROPERTY OCTAVE_IR_LIBS octave-ir-compiler-llvm)

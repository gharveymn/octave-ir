cmake_minimum_required (VERSION 3.19)

cmake_policy (SET CMP0091 NEW)
cmake_policy (SET CMP0111 NEW)

project (
  octave-ir
  VERSION
    0.0.1
  LANGUAGES
    CXX
)

string (COMPARE EQUAL "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" _EXTRAS_DEFAULT)

option (
  GCH_OCTAVE_IR_ENABLE_TESTS
  "Set to ON to build tests for gch::octave-ir."
  ${_EXTRAS_DEFAULT}
)

include (CMakeDependentOption)
cmake_dependent_option (
  GCH_USE_LIBCXX_WITH_CLANG
  "Set to ON to use libc++ when compiling with Clang."
  OFF
  "CMAKE_CXX_COMPILER_ID MATCHES Clang"
  OFF
)

option (GCH_OCTAVE_IR_BUILD_DYNAMIC_IR "Set to ON to build gch::octave-ir.dynamic-ir" ON)
option (GCH_OCTAVE_IR_BUILD_COMPILER "Set to ON to build gch::octave-ir.compiler-interface" ON)

cmake_dependent_option (
  GCH_OCTAVE_IR_BUILD_LLVM_COMPILER
  "Set to ON to build gch::octave-ir-llvm-compiler."
  ON
  GCH_OCTAVE_IR_BUILD_COMPILER
  OFF
)

cmake_dependent_option (
  GCH_OCTAVE_IR_BUILD_STATIC_IR
  "Set to ON to build gch::octave-ir.static-ir."
  ON
  "NOT GCH_OCTAVE_IR_BUILD_DYNAMIC_IR AND NOT GCH_OCTAVE_IR_BUILD_COMPILER"
  ON
)

cmake_dependent_option (
  GCH_OCTAVE_IR_ENABLE_ASAN
  "Set to ON to build with address sanitizer."
  OFF
  "CMAKE_SYSTEM_NAME MATCHES Linux"
  OFF
)

set (
  GCH_OCTAVE_IR_CXX_STANDARD
  "17"
  CACHE STRING
  "Set the C++ standard to use when compiling octave-ir."
)

set_property (
  CACHE
  GCH_OCTAVE_IR_CXX_STANDARD
  PROPERTY
  STRINGS
    17
    20
    23
)

find_package (Git REQUIRED)
execute_process (
  COMMAND
    ${GIT_EXECUTABLE} submodule --quiet update --init
  WORKING_DIRECTORY
    ${PROJECT_SOURCE_DIR}
)

add_subdirectory (external/nonnull_ptr)
add_subdirectory (external/optional_ref)
add_subdirectory (external/partition)
add_subdirectory (external/plf_list)
add_subdirectory (external/select-iterator)
add_subdirectory (external/small_vector)
add_subdirectory (external/tracker)
add_subdirectory (external/variant-iterator)

if (GCH_OCTAVE_IR_ENABLE_TESTS)
  enable_testing ()
endif ()

add_subdirectory (source)

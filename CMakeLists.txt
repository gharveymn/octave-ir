CMAKE_MINIMUM_REQUIRED (VERSION 3.13)
PROJECT (octave-ir
         VERSION 0.0.1
         LANGUAGES CXX)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# For some reason CMP0092 which removes /W3 from CMAKE_CXX_FLAGS isn't active on my machine
# even with CMake 3.20, so I guess I'll just do it manually here...
IF (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  STRING (REGEX REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF ()

# Options
IF (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  OPTION (OCTAVE_IR_BUILD_TESTS "Set to ON to build tests" ON)
ELSE ()
  OPTION (OCTAVE_IR_BUILD_TESTS "Set to ON to build tests" OFF)
ENDIF ()

OPTION (GCH_USE_LIBCXX_WITH_CLANG     "Set to ON to use libc++ when compiling with Clang" OFF)

OPTION (OCTAVE_IR_BUILD_DYNAMIC_IR    "Set to ON to build gch::octave-ir-dynamic-ir"      ON)
OPTION (OCTAVE_IR_BUILD_STATIC_IR     "Set to ON to build gch::octave-ir-static-ir"       ON)
OPTION (OCTAVE_IR_BUILD_COMPILER      "Set to ON to build gch::octave-ir-compiler"        ON)
OPTION (OCTAVE_IR_BUILD_LLVM_COMPILER "Set to ON to build gch::octave-ir-llvm-compiler"   ON)

IF (OCTAVE_IR_BUILD_DYNAMIC_IR
 OR OCTAVE_IR_BUILD_COMPILER)

  SET (OCTAVE_IR_BUILD_STATIC_IR ON)
ENDIF ()

IF (OCTAVE_IR_BUILD_LLVM_COMPILER)
  SET (OCTAVE_IR_BUILD_COMPILER ON)
ENDIF ()

ADD_SUBDIRECTORY (source)

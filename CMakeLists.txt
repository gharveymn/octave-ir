CMAKE_MINIMUM_REQUIRED (VERSION 3.13)
PROJECT (octave-ir
         VERSION 0.0.1
         LANGUAGES CXX)

# Options
IF (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  OPTION (GCH_OCTAVE_IR_BUILD_TESTS "Set to ON to build tests" ON)
ELSE ()
  OPTION (GCH_OCTAVE_IR_BUILD_TESTS "Set to ON to build tests" OFF)
ENDIF ()


OPTION (GCH_USE_LIBCXX_WITH_CLANG      "Set to ON to use libc++ when compiling with Clang" OFF)
OPTION (GCH_OCTAVE_IR_BUILD_DYNAMIC_IR "Set to ON to build gch::octave-dynamic-ir"         ON)
OPTION (GCH_OCTAVE_IR_BUILD_STATIC_IR  "Set to ON to build gch::octave-static-ir"          ON)

ADD_SUBDIRECTORY (source)

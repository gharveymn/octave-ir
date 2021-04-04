OPTION (OCTAVE_IR_LLVM_USE_DEFAULT "Set to ON to link to the default LLVM installation" ON)

SET (OCTAVE_IR_LLVM_PATH        "" CACHE STRING "A user specified path to an LLVM installation")
SET (OCTAVE_IR_LLVM_EXTRA_PATHS "" CACHE STRING "A list of paths to additional LLVM installations")

IF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  LIST (APPEND OCTAVE_IR_COMPILE_OPTS -Wno-undef)
ELSEIF (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  LIST (APPEND OCTAVE_IR_COMPILE_DEFS -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
ENDIF ()

MACRO (CREATE_LLVM_COMPILER_PACKAGE target_name)
  CREATE_PACKAGE (AUTHOR       gharveymn
                  NAMESPACE    gch
                  NAME         ${target_name}
                  TYPE         SHARED
                  HEADERS      octave-ir-compiler-llvm.hpp
                  SOURCES      octave-ir-compiler-llvm.cpp
                  DEPENDENCIES octave-ir-compiler
                               octave-ir-static-ir)

  TARGET_INCLUDE_DIRECTORIES (${target_name} SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})

  LLVM_MAP_COMPONENTS_TO_LIBNAMES (LLVM_LIBS support
                                             core
                                             orcjit
                                             jitlink
                                             target
                                             x86asmparser
                                             x86codegen
                                             x86desc
                                             x86info)

  TARGET_COMPILE_DEFINITIONS (${target_name} PRIVATE ${OCTAVE_IR_COMPILE_DEFS} ${LLVM_DEFINITIONS})
  TARGET_COMPILE_OPTIONS     (${target_name} PRIVATE ${OCTAVE_IR_COMPILE_OPTS})
  TARGET_LINK_OPTIONS        (${target_name} PRIVATE ${OCTAVE_IR_LINK_OPTS})
  TARGET_LINK_LIBRARIES      (${target_name} PRIVATE ${OCTAVE_IR_LINK_LIBS} ${LLVM_LIBS})

  SET_TARGET_PROPERTIES (${target_name}
                         PROPERTIES CXX_STANDARD               17
                                    CXX_STANDARD_REQUIRED      NO
                                    CXX_EXTENSIONS             NO
                                    WINDOWS_EXPORT_ALL_SYMBOLS YES)
ENDMACRO ()

IF (OCTAVE_IR_LLVM_USE_DEFAULT)
  IF (OCTAVE_IR_LLVM_PATH)
    FIND_PACKAGE (LLVM REQUIRED CONFIG PATHS "${OCTAVE_IR_LLVM_PATH}" NO_DEFAULT_PATH)
  ELSE ()
    FIND_PACKAGE (LLVM REQUIRED CONFIG)
  ENDIF()
  MESSAGE (STATUS "Using default backend LLVM-${LLVM_PACKAGE_VERSION} (${LLVM_DIR})")

  CREATE_LLVM_COMPILER_PACKAGE (octave-ir-compiler-llvm)
ENDIF ()

FOREACH (path ${OCTAVE_IR_LLVM_PATHS})
  FIND_PACKAGE (LLVM REQUIRED CONFIG PATHS "${path}" NO_DEFAULT_PATH)
  MESSAGE (STATUS "Found LLVM-${LLVM_PACKAGE_VERSION} at ${path}")

  CREATE_LLVM_COMPILER_PACKAGE (octave-ir-compiler-llvm.LLVM-${LLVM_PACKAGE_VERSION})
ENDFOREACH ()

#SET_PROPERTY (GLOBAL APPEND PROPERTY OCTAVE_IR_LIBS octave-ir-compiler-llvm)


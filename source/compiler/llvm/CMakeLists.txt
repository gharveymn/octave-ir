CREATE_PACKAGE (AUTHOR       gharveymn
                NAMESPACE    gch
                NAME         octave-ir-llvm-compiler
                TYPE         SHARED
                HEADERS      octave-ir-llvm-compiler.hpp
                SOURCES      octave-ir-llvm-compiler.cpp
                DEPENDENCIES octave-ir-compiler)

FIND_PACKAGE (LLVM REQUIRED CONFIG)

MESSAGE (STATUS "LLVM headers located at ${LLVM_INCLUDE_DIRS}")

TARGET_INCLUDE_DIRECTORIES (octave-ir-llvm-compiler SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES (octave-ir-llvm-compiler PRIVATE LLVM)

TARGET_COMPILE_OPTIONS (octave-ir-llvm-compiler PRIVATE ${OCTAVE_IR_COMPILE_OPTS})
TARGET_LINK_OPTIONS    (octave-ir-llvm-compiler PRIVATE ${OCTAVE_IR_LINK_OPTS})
TARGET_LINK_LIBRARIES  (octave-ir-llvm-compiler PRIVATE ${OCTAVE_IR_LINK_LIBS})

SET_TARGET_PROPERTIES (octave-ir-llvm-compiler
                       PROPERTIES CXX_STANDARD          17
                                  CXX_STANDARD_REQUIRED NO
                                  CXX_EXTENSIONS        NO)

SET_PROPERTY (GLOBAL APPEND PROPERTY OCTAVE_IR_LIBS octave-ir-llvm-compiler)


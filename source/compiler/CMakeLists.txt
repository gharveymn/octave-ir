ADD_SUBDIRECTORY (interface)

IF (OCTAVE_IR_BUILD_LLVM_COMPILER)
  MESSAGE (STATUS "Generating build files for gch::octave-ir-llvm-compiler")
  ADD_SUBDIRECTORY (llvm)
ENDIF ()

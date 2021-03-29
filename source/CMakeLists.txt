ADD_SUBDIRECTORY (utilities)
IF (GCH_OCTAVE_IR_BUILD_DYNAMIC_IR)
  MESSAGE (STATUS "Building gch::octave-dynamic-ir")
  ADD_SUBDIRECTORY (static-ir)
  ADD_SUBDIRECTORY (dynamic-ir)

ELSEIF (GCH_OCTAVE_IR_BUILD_STATIC_IR)

  MESSAGE (STATUS "Building gch::octave-static-ir")
  ADD_SUBDIRECTORY (static-ir)

ENDIF ()

IF (GCH_OCTAVE_IR_BUILD_TESTS)
  MESSAGE (STATUS "Building tests for gch::octave-ir.")
  ADD_SUBDIRECTORY (test)
ENDIF ()

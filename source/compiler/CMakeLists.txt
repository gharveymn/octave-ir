add_subdirectory (interface)

if (GCH_OCTAVE_IR_BUILD_LLVM_COMPILER)
  add_subdirectory (llvm)
endif ()

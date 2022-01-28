if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  string (REGEX REPLACE "/[wW]([0-4deovX]|all) ?" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif ()

add_library (octave-ir.test.extern-funcs OBJECT)
target_sources (octave-ir.test.extern-funcs PRIVATE extern-funcs.cpp)

macro (add_ctest_executables)
  foreach (file ${ARGN})
    get_filename_component (_TARGET_NAME "${file}" NAME_WE)
    string (PREPEND _TARGET_NAME "octave-ir.")
    foreach (version 17 20)
      add_test_executable (${_TARGET_NAME}.c++${version} ${file})

      target_link_libraries (
        ${_TARGET_NAME}.c++${version}
        PRIVATE
          octave-ir.utilities
          octave-ir.static-ir
          octave-ir.dynamic-ir
          octave-ir.compiler-llvm
          octave-ir.test.extern-funcs
      )

      set_target_properties (
        ${_TARGET_NAME}.c++${version}
        PROPERTIES
        CXX_STANDARD
          ${version}
        CXX_STANDARD_REQUIRED
          NO
        CXX_EXTENSIONS
          NO
      )

      add_test (
        NAME
          ${_TARGET_NAME}.c++${version}
        COMMAND
          ${_TARGET_NAME}.c++${version}
      )

      add_dependencies(octave-ir.ctest ${_TARGET_NAME}.c++${version})
    endforeach ()
  endforeach ()
endmacro ()

add_ctest_executables (
  test-add.cpp
  test-call.cpp
  test-if.cpp
  test-land.cpp
  test-lnot.cpp
  test-loop.cpp
  test-lor.cpp
  test-nested-loop.cpp
  test-uninit.cpp
)

# add_subdirectory (scratch)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  string (REGEX REPLACE "/EH[ascr]+-?" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif ()

add_library (octave-ir.test.extern-funcs OBJECT)
target_sources (octave-ir.test.extern-funcs PRIVATE extern-funcs.cpp)

add_custom_target (octave-ir.ctest)

macro (add_ctest_executables)
  foreach (file ${ARGN})
    get_filename_component (_TARGET_NAME "${file}" NAME_WE)
    string (PREPEND _TARGET_NAME "octave-ir.")

    add_test_executable (${_TARGET_NAME} ${file})

    target_link_libraries (
      ${_TARGET_NAME}
      PRIVATE
        octave-ir.utilities
        octave-ir.static-ir
        octave-ir.dynamic-ir
        octave-ir.compiler-llvm
        octave-ir.test.extern-funcs
    )

    add_test (
      NAME
        ${_TARGET_NAME}
      COMMAND
        ${_TARGET_NAME}
    )

    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      target_compile_options (
        ${_TARGET_NAME}
        PRIVATE
          /EHsc-
      )
    endif ()

    add_dependencies(octave-ir.ctest ${_TARGET_NAME})
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
  test-sub.cpp
  test-uninit.cpp
)

# add_subdirectory (scratch)

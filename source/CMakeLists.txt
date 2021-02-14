IF(NOT CREATE_PACKAGE_VERSION VERSION_GREATER_EQUAL 0.0.4)
  IF(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/create-package/cmake/create-package.cmake")
    MESSAGE(STATUS "Initializing create-package-0.0.4 in git submodule.")
    EXECUTE_PROCESS(COMMAND git submodule --quiet update --init -- "${PROJECT_SOURCE_DIR}/external/create-package"
                    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
  ENDIF()
  INCLUDE("${PROJECT_SOURCE_DIR}/external/create-package/cmake/create-package.cmake")
ENDIF()

# Sources
SET(HEADERS ir-block.hpp
            ir-common.hpp
            ir-common-util.hpp
            ir-component.hpp
            ir-component-fork.hpp
            ir-component-handle.hpp
            ir-component-loop.hpp
            ir-component-sequence.hpp
            ir-constant.hpp
            ir-def-resolution.hpp
            ir-function.hpp
            ir-instruction.hpp
            ir-instruction-fwd.hpp
            ir-link-set.hpp
            ir-optional-util.hpp
            ir-stack-builder.hpp
            ir-structure.hpp
            ir-structure-fwd.hpp
            ir-traverser.hpp
            ir-type.hpp
            ir-type-base.hpp
            ir-type-ir.hpp
            ir-type-std.hpp
            ir-variable.hpp)

SET(SOURCES ir-block.cpp
            ir-common-util.cpp
            ir-component.cpp
            ir-component-fork.cpp
            ir-component-loop.cpp
            ir-component-sequence.cpp
            ir-constant.cpp
            ir-def-resolution.cpp
            ir-function.cpp
            ir-instruction.cpp
            ir-link-set.cpp
            ir-structure.cpp
            ir-traverser.cpp
            ir-stack-builder.cpp
            ir-type.cpp
            ir-type-base.cpp
            ir-type-ir.cpp
            ir-type-std.cpp
            ir-variable.cpp)

CREATE_PACKAGE(AUTHOR       gharveymn
               NAME         octave-ir.c++17
               HEADERS      ${HEADERS}
               SOURCES      ${SOURCES}
               NO_HEADERS_PREFIX
               DEPENDENCIES optional_ref
                            plf_list
                            tracker
                            nonnull_ptr
                            select-iterator
                            partition
                            small_vector
               )

SET_TARGET_PROPERTIES(octave-ir.c++17 PROPERTIES
                      CXX_STANDARD          17
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS        NO)

CREATE_PACKAGE(AUTHOR       gharveymn
               NAME         octave-ir.c++20
               HEADERS      ${HEADERS}
               SOURCES      ${SOURCES}
               NO_HEADERS_PREFIX
               DEPENDENCIES optional_ref
                            plf_list
                            tracker
                            nonnull_ptr
                            select-iterator
                            partition
                            small_vector
               )

SET_TARGET_PROPERTIES(octave-ir.c++20 PROPERTIES
                      CXX_STANDARD          20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS        NO)

# Only include tests if we are at the top level
IF(OCTAVE_IR_BUILD_TESTS)
  ADD_SUBDIRECTORY(test)
ENDIF()

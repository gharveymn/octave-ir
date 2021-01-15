IF(NOT CREATE_PACKAGE_VERSION VERSION_GREATER_EQUAL 0.0.4)
  IF(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/create-package/cmake/create-package.cmake")
    MESSAGE(STATUS "Initializing create-package-0.0.4 in git submodule.")
    EXECUTE_PROCESS(COMMAND git submodule --quiet update --init -- "${PROJECT_SOURCE_DIR}/external/create-package"
                    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
  ENDIF()
  INCLUDE("${PROJECT_SOURCE_DIR}/external/create-package/cmake/create-package.cmake")
ENDIF()

# Sources
SET(HEADERS
    "ir-block.hpp"
    "ir-common.hpp"
    "ir-common-util.hpp"
    "ir-component.hpp"
    "ir-constant.hpp"
    "ir-function.hpp"
    "ir-instruction.hpp"
    "ir-instruction-fwd.hpp"
    "ir-structure.hpp"
    "ir-type.hpp"
    "ir-type-base.hpp"
    "ir-type-ir.hpp"
    "ir-type-std.hpp"
    "ir-variable.hpp")

SET(SOURCES
    "ir-block.cpp"
    "ir-common-util.cpp"
    "ir-component.cpp"
    "ir-constant.cpp"
    "ir-function.cpp"
    "ir-instruction.cpp"
    "ir-structure.cpp"
    "ir-type.cpp"
    "ir-type-base.cpp"
    "ir-type-ir.cpp"
    "ir-type-std.cpp"
    "ir-variable.cpp")

CREATE_PACKAGE(AUTHOR       gharveymn
               NAME         octave-ir
               HEADERS      ${HEADERS}
               SOURCES      ${SOURCES}
               NO_HEADERS_PREFIX
               DEPENDENCIES optional_ref
                            plf_list
                            tracker
                            variant-iterator
                            nonnull_ptr
                            select-iterator
                            partition
                            small_vector)

SET_TARGET_PROPERTIES(octave-ir PROPERTIES
                      CXX_STANDARD          17
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS        NO)

# Only include tests if we are at the top level
IF(OCTAVE_IR_BUILD_TESTS)
  ADD_SUBDIRECTORY(test)
ENDIF()

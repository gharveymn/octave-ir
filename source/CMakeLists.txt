IF (NOT CREATE_PACKAGE_VERSION VERSION_GREATER_EQUAL 0.0.4)
  IF (NOT EXISTS "${PROJECT_SOURCE_DIR}/external/create-package/cmake/create-package.cmake")
    MESSAGE (STATUS "Initializing create-package-0.0.4 in git submodule.")
    EXECUTE_PROCESS (COMMAND git submodule --quiet update --init -- "${PROJECT_SOURCE_DIR}/external/create-package"
                     WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
  ENDIF ()
  INCLUDE ("${PROJECT_SOURCE_DIR}/external/create-package/cmake/create-package.cmake")
ENDIF ()

OPTION (GCH_USE_LIBCXX_WITH_CLANG "Set to ON to use libc++ when compiling with Clang" OFF)

# Sources
SET (HEADERS
     components/linkage/ir-incoming-node.hpp
     components/linkage/ir-def-timeline.hpp

     components/ir-block.hpp
     components/ir-block-common.hpp
     components/ir-component.hpp
     components/ir-component-fork.hpp
     components/ir-component-fwd.hpp
     components/ir-component-handle.hpp
     components/ir-component-loop.hpp
     components/ir-component-sequence.hpp
     components/ir-function.hpp
     components/ir-structure.hpp

     processors/ir-def-resolution.hpp

     utilities/ir-allocator-util.hpp
     utilities/ir-common.hpp
     utilities/ir-error.hpp
     utilities/ir-optional-util.hpp
     utilities/ir-link-set.hpp

     values/types/ir-instruction-metadata.hpp
     values/types/ir-type.hpp
     values/types/ir-type-base.hpp
     values/types/ir-type-ir.hpp
     values/types/ir-type-std.hpp

     values/ir-constant.hpp
     values/ir-instruction.hpp
     values/ir-instruction-fwd.hpp
     values/ir-variable.hpp

     visitors/inspectors/ir-entry-collector.hpp
     visitors/inspectors/ir-leaf-collector.hpp
     visitors/inspectors/ir-leaf-inspector.hpp
     visitors/inspectors/ir-predecessor-collector.hpp
     visitors/inspectors/ir-subcomponent-inspector.hpp
     visitors/inspectors/ir-successor-collector.hpp

     visitors/mutators/ir-def-propagator.hpp
     visitors/mutators/ir-def-resolution-builder.hpp
     visitors/mutators/ir-forward-mutator.hpp
     visitors/mutators/ir-structure-flattener.hpp
     visitors/mutators/ir-subcomponent-mutator.hpp

     visitors/ir-traverser.hpp
     visitors/ir-visitor.hpp
     visitors/ir-visitor-fwd.hpp)

SET (SOURCES
     components/linkage/ir-incoming-node.cpp
     components/linkage/ir-def-timeline.cpp

     components/ir-block.cpp
     components/ir-component.cpp
     components/ir-component-fork.cpp
     components/ir-component-loop.cpp
     components/ir-component-sequence.cpp
     components/ir-function.cpp
     components/ir-structure.cpp

     processors/ir-def-resolution.cpp

     utilities/ir-common-util.cpp
     utilities/ir-error.cpp
     utilities/ir-link-set.cpp

     values/types/ir-instruction-metadata.cpp
     values/types/ir-type.cpp
     values/types/ir-type-base.cpp
     values/types/ir-type-ir.cpp
     values/types/ir-type-std.cpp

     values/ir-constant.cpp
     values/ir-instruction.cpp
     values/ir-variable.cpp

     visitors/inspectors/ir-entry-collector.cpp
     visitors/inspectors/ir-leaf-collector.cpp
     visitors/inspectors/ir-leaf-inspector.cpp
     visitors/inspectors/ir-predecessor-collector.cpp
     visitors/inspectors/ir-subcomponent-inspector.cpp
     visitors/inspectors/ir-successor-collector.cpp

     visitors/mutators/ir-def-propagator.cpp
     visitors/mutators/ir-def-resolution-builder.cpp
     visitors/mutators/ir-forward-mutator.cpp
     visitors/mutators/ir-structure-flattener.cpp
     visitors/mutators/ir-subcomponent-mutator.cpp

     visitors/ir-traverser.cpp)

CREATE_PACKAGE (AUTHOR             gharveymn
                NAME               octave-ir.c++17
                HEADERS            ${HEADERS}
                SOURCES            ${SOURCES}
                NO_HEADERS_PREFIX
                DEPENDENCIES       optional_ref
                                   plf_list
                                   tracker
                                   nonnull_ptr
                                   select-iterator
                                   partition
                                   small_vector)

SET_TARGET_PROPERTIES (octave-ir.c++17
                       PROPERTIES CXX_STANDARD          17
                                  CXX_STANDARD_REQUIRED NO
                                  CXX_EXTENSIONS        NO)

CREATE_PACKAGE (AUTHOR             gharveymn
                NAME               octave-ir.c++20
                HEADERS            ${HEADERS}
                SOURCES            ${SOURCES}
                NO_HEADERS_PREFIX
                DEPENDENCIES       optional_ref
                                   plf_list
                                   tracker
                                   nonnull_ptr
                                   select-iterator
                                   partition
                                   small_vector)

SET_TARGET_PROPERTIES (octave-ir.c++20
                       PROPERTIES CXX_STANDARD          20
                                  CXX_STANDARD_REQUIRED NO
                                  CXX_EXTENSIONS        NO)

# Only include tests if we are at the top level
IF (OCTAVE_IR_BUILD_TESTS)
  ADD_SUBDIRECTORY (test)
ENDIF ()

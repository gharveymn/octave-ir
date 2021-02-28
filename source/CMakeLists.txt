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

     # component visitors
     visitors/component/inspectors/ir-block-counter.hpp
     visitors/component/inspectors/ir-component-inspectors.hpp
     visitors/component/inspectors/ir-component-inspectors-fwd.hpp
     visitors/component/inspectors/ir-leaf-collector.hpp

     visitors/component/mutators/ir-component-mutators.hpp
     visitors/component/mutators/ir-component-mutators-fwd.hpp
     visitors/component/mutators/ir-descending-def-propagator.hpp
     visitors/component/mutators/ir-descending-def-resolution-builder.hpp
     visitors/component/mutators/ir-descending-forward-mutator.hpp

     visitors/component/ir-component-visitors.hpp
     visitors/component/ir-component-visitors-fwd.hpp

     # structure visitors
     visitors/structure/inspectors/utility/ir-subcomponent-inspector.hpp
     visitors/structure/inspectors/ir-entry-collector.hpp
     visitors/structure/inspectors/ir-leaf-inspector.hpp
     visitors/structure/inspectors/ir-predecessor-collector.hpp
     visitors/structure/inspectors/ir-structure-inspectors.hpp
     visitors/structure/inspectors/ir-structure-inspectors-fwd.hpp
     visitors/structure/inspectors/ir-successor-collector.hpp

     visitors/structure/mutators/utility/ir-subcomponent-mutator.hpp
     visitors/structure/mutators/ir-ascending-def-propagator.hpp
     visitors/structure/mutators/ir-ascending-def-resolution-builder.hpp
     visitors/structure/mutators/ir-ascending-forward-mutator.hpp
     visitors/structure/mutators/ir-structure-flattener.hpp
     visitors/structure/mutators/ir-structure-mutators.hpp
     visitors/structure/mutators/ir-structure-mutators-fwd.hpp

     visitors/structure/ir-structure-visitors.hpp
     visitors/structure/ir-structure-visitors-fwd.hpp

     # subcomponent visitors
     visitors/subcomponent/inspectors/ir-subcomponent-inspectors.hpp
     visitors/subcomponent/inspectors/ir-subcomponent-inspectors-fwd.hpp

     visitors/subcomponent/mutators/ir-subcomponent-mutators.hpp
     visitors/subcomponent/mutators/ir-subcomponent-mutators-fwd.hpp

     visitors/subcomponent/ir-subcomponent-visitors.hpp
     visitors/subcomponent/ir-subcomponent-visitors-fwd.hpp

     # substructure visitors
     visitors/substructure/inspectors/ir-substructure-inspectors.hpp
     visitors/substructure/inspectors/ir-substructure-inspectors-fwd.hpp

     visitors/substructure/mutators/ir-substructure-mutators.hpp
     visitors/substructure/mutators/ir-substructure-mutators-fwd.hpp

     visitors/substructure/ir-substructure-visitors.hpp
     visitors/substructure/ir-substructure-visitors-fwd.hpp

     visitors/ir-all-visitors.hpp
     visitors/ir-all-visitors-fwd.hpp
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

     # component visitors
     visitors/component/inspectors/ir-block-counter.cpp
     visitors/component/inspectors/ir-leaf-collector.cpp

     visitors/component/mutators/ir-descending-def-propagator.cpp
     visitors/component/mutators/ir-descending-def-resolution-builder.cpp
     visitors/component/mutators/ir-descending-forward-mutator.cpp

     # structure visitors
     visitors/structure/inspectors/utility/ir-subcomponent-inspector.cpp
     visitors/structure/inspectors/ir-entry-collector.cpp
     visitors/structure/inspectors/ir-leaf-inspector.cpp
     visitors/structure/inspectors/ir-predecessor-collector.cpp
     visitors/structure/inspectors/ir-successor-collector.cpp

     visitors/structure/mutators/utility/ir-subcomponent-mutator.cpp
     visitors/structure/mutators/ir-ascending-def-propagator.cpp
     visitors/structure/mutators/ir-ascending-def-resolution-builder.cpp
     visitors/structure/mutators/ir-ascending-forward-mutator.cpp
     visitors/structure/mutators/ir-structure-flattener.cpp

     # subcomponent visitors

     # substructure visitors

     )

SET(TARGET_VERSIONS 17 20)

SET(COMPILE_OPTS )
SET(LINK_OPTS )

IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

  LIST (APPEND
        COMPILE_OPTS -ftime-trace
                     --coverage
                     -Weverything
                     -Wno-padded
                     -Wno-c++98-compat
                     -Wno-c++98-compat-pedantic
                     -Wno-zero-as-null-pointer-constant)

    LIST (APPEND
          LINK_OPTS --coverage)

  # Clang options
  IF (GCH_USE_LIBCXX_WITH_CLANG)

    LIST (APPEND
          COMPILE_OPTS -stdlib=libc++)

    LIST (APPEND
          LINK_OPTS -Wl,--allow-multiple-definition)

    LIST (APPEND
          LINK_LIBS c++
                    c++abi)

    MESSAGE (STATUS "Linking with libc++")

  ENDIF ()

ELSEIF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

  # GCC options

  LIST (APPEND
        COMPILE_OPTS -pedantic
                     -Wall
                     -Wextra
                     -Wcast-align
                     -Wcast-qual
                     -Wctor-dtor-privacy
                     -Wdisabled-optimization
                     -Wformat=2
                     -Winit-self
                     -Wlogical-op
                     -Wmissing-declarations
                     -Wmissing-include-dirs
                     -Wnoexcept
                     -Wold-style-cast
                     -Woverloaded-virtual
                     -Wredundant-decls
                     -Wshadow
                     -Wsign-conversion
                     -Wsign-promo
                     -Wstrict-null-sentinel
                     -Wstrict-overflow=5
                     -Wswitch-default
                     -Wundef
                     -Wno-unused)

ELSEIF (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

  # MSVC options

  LIST (APPEND
        COMPILE_OPTS /W4 /wd4250)

ENDIF ()

IF (UNIX
    AND CMAKE_BUILD_TYPE STREQUAL Debug)

  LIST (APPEND
        COMPILE_OPTS -fsanitize=address)

  LIST (APPEND
        LINK_OPTS    -fsanitize=address)

ENDIF ()

FOREACH (version ${TARGET_VERSIONS})
  SET(target_name octave-ir.c++${version})

  CREATE_PACKAGE (AUTHOR             gharveymn
                  NAME               ${target_name}
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

  TARGET_COMPILE_OPTIONS(${target_name} PRIVATE ${COMPILE_OPTS})
  TARGET_LINK_OPTIONS   (${target_name} PRIVATE ${LINK_OPTS})

  SET_TARGET_PROPERTIES (${target_name}
                         PROPERTIES CXX_STANDARD          ${version}
                                    CXX_STANDARD_REQUIRED NO
                                    CXX_EXTENSIONS        NO)
ENDFOREACH ()

ADD_LIBRARY (test-visitor-link)
TARGET_SOURCES(test-visitor-link
               PUBLIC "$<BUILD_INTERFACE:${OCTAVE_IR_C__20_ABSOLUTE_HEADERS}>"
               PRIVATE "lib/components/ir-component-sequence.cpp;lib/visitors/component/inspectors/ir-leaf-collector.cpp")

TARGET_INCLUDE_DIRECTORIES(test-visitor-link
                           PUBLIC "$<BUILD_INTERFACE:${OCTAVE_IR_C__20_ABSOLUTE_HEADERS_PATH}>")

TARGET_INCLUDE_DIRECTORIES(test-visitor-link SYSTEM INTERFACE
                           "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${OCTAVE_IR_C__20_HEADERS_INSTALL_PATH}>")

TARGET_LINK_LIBRARIES (test-visitor-link
                       PRIVATE
                       optional_ref
                       plf_list
                       tracker
                       nonnull_ptr
                       select-iterator
                       partition
                       small_vector)

SET_TARGET_PROPERTIES (test-visitor-link
                       PROPERTIES CXX_STANDARD          17
                                  CXX_STANDARD_REQUIRED NO
                                  CXX_EXTENSIONS        NO)

# Only include tests if we are at the top level
IF (OCTAVE_IR_BUILD_TESTS)
  ADD_SUBDIRECTORY (test)
ENDIF ()

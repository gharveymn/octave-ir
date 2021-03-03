IF (NOT CREATE_PACKAGE_VERSION VERSION_GREATER_EQUAL 0.0.5)
  IF (NOT EXISTS "${PROJECT_SOURCE_DIR}/external/create-package/cmake/create-package.cmake")
    MESSAGE (STATUS "Initializing create-package-0.0.5 in git submodule.")
    EXECUTE_PROCESS (COMMAND git submodule --quiet update --init -- "${PROJECT_SOURCE_DIR}/external/create-package"
                     WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
  ENDIF ()
  INCLUDE ("${PROJECT_SOURCE_DIR}/external/create-package/cmake/create-package.cmake")
ENDIF ()

OPTION (GCH_USE_LIBCXX_WITH_CLANG "Set to ON to use libc++ when compiling with Clang" OFF)

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

SET (OCTAVE_IR_TARGETS)
FOREACH (version ${TARGET_VERSIONS})
  SET(target_name octave-ir.c++${version})

  CREATE_PACKAGE (AUTHOR             gharveymn
                  NAME               ${target_name}
                  TYPE               STATIC
                  NO_HEADERS_PREFIX
                  NO_PRINT
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

  LIST (APPEND OCTAVE_IR_TARGETS ${target_name})
ENDFOREACH ()

ADD_SUBDIRECTORY (include)
ADD_SUBDIRECTORY (lib)

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

IF (NOT CREATE_PACKAGE_VERSION VERSION_GREATER_EQUAL 0.0.5)
  IF (NOT EXISTS "${PROJECT_SOURCE_DIR}/external/create-package/cmake/create-package.cmake")
    MESSAGE (STATUS "Initializing create-package-0.0.5 in git submodule.")
    EXECUTE_PROCESS (COMMAND git submodule --quiet update --init -- "${PROJECT_SOURCE_DIR}/external/create-package"
                     WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
  ENDIF ()
  INCLUDE ("${PROJECT_SOURCE_DIR}/external/create-package/cmake/create-package.cmake")
ENDIF ()

# Project-wide compile options
SET (OCTAVE_IR_COMPILE_OPTS)
SET (OCTAVE_IR_LINK_OPTS)
SET (OCTAVE_IR_LINK_LIBS)

IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

  LIST (APPEND
        OCTAVE_IR_COMPILE_OPTS -ftime-trace
                               --coverage
                               -Weverything
                               -Wno-padded
                               -Wno-c++98-compat
                               -Wno-c++98-compat-pedantic
                               -Wno-zero-as-null-pointer-constant)

    LIST (APPEND
          OCTAVE_IR_LINK_OPTS --coverage)

  # Clang options
  IF (GCH_USE_LIBCXX_WITH_CLANG)

    LIST (APPEND
          OCTAVE_IR_COMPILE_OPTS -stdlib=libc++)

    LIST (APPEND
          OCTAVE_IR_LINK_OPTS -Wl,--allow-multiple-definition)

    LIST (APPEND
          OCTAVE_IR_LINK_LIBS c++
                              c++abi)

    MESSAGE (STATUS "Linking with libc++")

  ENDIF ()

ELSEIF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

  # GCC options

  LIST (APPEND
        OCTAVE_IR_COMPILE_OPTS -pedantic
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
        OCTAVE_IR_COMPILE_OPTS /W4 /wd4250)

ENDIF ()

IF (UNIX
    AND CMAKE_BUILD_TYPE STREQUAL "Debug")

  LIST (APPEND
        OCTAVE_IR_COMPILE_OPTS -fsanitize=address)

  LIST (APPEND
        OCTAVE_IR_LINK_OPTS    -fsanitize=address)

ENDIF ()

# Always build octave-ir-utilities
ADD_SUBDIRECTORY (utilities)

IF (GCH_OCTAVE_IR_BUILD_STATIC_IR)
  MESSAGE (STATUS "Generating build files for gch::octave-static-ir")
  ADD_SUBDIRECTORY (static-ir)
ENDIF ()

IF (GCH_OCTAVE_IR_BUILD_DYNAMIC_IR)
  MESSAGE (STATUS "Generating build files for gch::octave-dynamic-ir")
  ADD_SUBDIRECTORY (dynamic-ir)
ENDIF ()

IF (GCH_OCTAVE_IR_BUILD_IR_COMPILER)
  MESSAGE (STATUS "Generating build files for gch::octave-ir-compiler")
  ADD_SUBDIRECTORY (compiler)
ENDIF ()

IF (GCH_OCTAVE_IR_BUILD_TESTS)
  MESSAGE (STATUS "Building tests for gch::octave-ir.")
  ADD_SUBDIRECTORY (test)
ENDIF ()

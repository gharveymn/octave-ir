INITIALIZE_DEPENDENCY(doctest)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(TEST_NAME ${PROJECT_NAME})

SET(SOURCE_FILES
    main.test.cpp
    utilities/ir-link-set.test.cpp
    )

SET(TARGET_VERSIONS 17 20)

SET(COMPILE_OPTS)
SET(LINK_OPTS)
SET(LINK_LIBS doctest::doctest
              nonnull_ptr
              optional_ref
              partition
              plf_list
              small_vector
              tracker)

SET(INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/source/include)

IF (CMAKE_CXX_COMPILER_ID
    STREQUAL
    Clang)

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

ELSEIF (CMAKE_CXX_COMPILER_ID
        STREQUAL
        GNU)

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

ELSEIF (CMAKE_CXX_COMPILER_ID
        STREQUAL
        MSVC)

  # MSVC options

  LIST (APPEND
        COMPILE_OPTS /W4)

ENDIF ()

IF (UNIX
    AND CMAKE_BUILD_TYPE
        STREQUAL
        Debug)

  LIST (APPEND
        COMPILE_OPTS -fsanitize=address)

  LIST (APPEND
        LINK_OPTS    -fsanitize=address)

ENDIF ()

FOREACH(version ${TARGET_VERSIONS})
  SET(target_name ${TEST_NAME}.doctest.c++${version})
  ADD_EXECUTABLE(${target_name} ${SOURCE_FILES})

  TARGET_INCLUDE_DIRECTORIES(${target_name} PRIVATE ${INCLUDE_DIRS})

  TARGET_COMPILE_OPTIONS(${target_name} PRIVATE ${COMPILE_OPTS})
  TARGET_LINK_OPTIONS   (${target_name} PRIVATE ${LINK_OPTS})
  TARGET_LINK_LIBRARIES (${target_name} PRIVATE ${LINK_LIBS})

  SET_TARGET_PROPERTIES(${target_name} PROPERTIES
                        CXX_STANDARD          ${version}
                        CXX_STANDARD_REQUIRED NO
                        CXX_EXTENSIONS        NO)
ENDFOREACH()


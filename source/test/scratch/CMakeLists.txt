SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(TEST_NAME scratch)
SET(SOURCE_FILES scratch.cpp
    ../../lib/utilities/ir-link-set.cpp)

SET(TARGET_VERSIONS 17 20)

SET(LINK_LIBS optional_ref
              plf_list
              tracker
              nonnull_ptr
              small_vector)

SET(INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/source/include)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  LIST(APPEND COMPILE_OPTS -stdlib=libc++ -ftime-trace --coverage -Wall -Wextra)
  LIST(APPEND LINK_OPTS -Wl,--allow-multiple-definition --coverage)
  LIST(APPEND LINK_LIBS c++ c++abi)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  LIST(APPEND COMPILE_OPTS -Wall -Wextra)
  LIST(APPEND LINK_OPTS )
  LIST(APPEND LINK_LIBS )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  LIST(APPEND COMPILE_OPTS /W4)
  LIST(APPEND LINK_OPTS )
  LIST(APPEND LINK_LIBS )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  LIST(APPEND COMPILE_OPTS )
  LIST(APPEND LINK_OPTS )
  LIST(APPEND LINK_LIBS )
endif()

if (UNIX)
  LIST(APPEND COMPILE_OPTS -fsanitize=address)
  LIST(APPEND LINK_OPTS -fsanitize=address)
endif()

FOREACH(version ${TARGET_VERSIONS})
  SET(target_name ${TEST_NAME}.test.c++${version})
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

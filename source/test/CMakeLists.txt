SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
ADD_EXECUTABLE(test_octave-ir main.cpp)
TARGET_LINK_LIBRARIES(test_octave-ir octave-ir)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  #    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # using Intel C++
endif()

if (UNIX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
endif()

SET_TARGET_PROPERTIES(test_octave-ir PROPERTIES
                      CXX_STANDARD          17
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS        NO
                      COMPILE_OPTIONS       "${COMPILE_OPTS}"
                      LINK_OPTIONS          "${LINK_OPTS}")

ADD_EXECUTABLE(scratch scratch.cpp)
TARGET_LINK_LIBRARIES(scratch PRIVATE optional_ref plf_list tracker nonnull_ptr)
TARGET_INCLUDE_DIRECTORIES(scratch PRIVATE ../include)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(COMPILE_OPTS -fno-rtti --coverage -Wall -Wextra)
  SET(LINK_OPTS --coverage)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  SET(COMPILE_OPTS -fno-rtti -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  SET(COMPILE_OPTS /GR-)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")

endif()

if (UNIX)
  LIST(APPEND COMPILE_OPTS -fsanitize=address)
  LIST(APPEND LINK_OPTS -fsanitize=address)
endif()

SET_TARGET_PROPERTIES(scratch PROPERTIES
                      CXX_STANDARD          17
                      CXX_STANDARD_REQUIRED NO
                      CXX_EXTENSIONS        NO
                      COMPILE_OPTIONS       "${COMPILE_OPTS}"
                      LINK_OPTIONS          "${LINK_OPTS}")

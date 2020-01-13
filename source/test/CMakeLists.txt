SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_EXTENSIONS OFF)

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

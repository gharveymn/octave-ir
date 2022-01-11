if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  string (REGEX REPLACE "/[wW]([0-4deovX]|all) ?" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif ()

macro (add_test_executable target_name)
  add_executable (${target_name} ${ARGN})

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options (
      ${target_name}
      PRIVATE
        -ftime-trace
        --coverage
        -Weverything
        -Wno-padded
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-zero-as-null-pointer-constant
        -Wno-used-but-marked-unused
    )

    target_link_options (
      ${target_name}
      PRIVATE
        --coverage
    )

    if (GCH_USE_LIBCXX_WITH_CLANG)
      target_compile_options (${target_name} PRIVATE -stdlib=libc++)
      target_link_options (${target_name} PRIVATE LINKER:--allow-multiple-definition)
      target_link_libraries (${target_name} PRIVATE c++ c++abi)
    endif ()
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options (
      ${target_name}
      PRIVATE
        -pedantic
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
        -Wno-unused
    )
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options (
      ${target_name}
      PRIVATE
        /W4
    )
  endif ()

  if (UNIX)
    target_compile_options (${target_name} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
    target_link_options (${target_name} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
  endif ()
endmacro ()

foreach (version 17 20)
  add_test_executable (octave-ir.test.c++${version} main.cpp)

  target_link_libraries (
    octave-ir.test.c++${version}
    PRIVATE
      octave-ir-utilities
      octave-ir-static-ir
      octave-ir-dynamic-ir
      octave-ir-compiler
  )

  set_target_properties (
    octave-ir.test.c++${version}
    PROPERTIES
    CXX_STANDARD
      ${version}
    CXX_STANDARD_REQUIRED
      NO
    CXX_EXTENSIONS
      NO
  )

  add_test (
    NAME
      octave-ir.test.c++${version}
    COMMAND
      octave-ir.test.c++${version}
  )
endforeach ()

add_subdirectory (doctest)
# add_subdirectory (scratch)

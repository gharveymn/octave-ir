set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY     ${CMAKE_BINARY_DIR}/build-output/lib)
set (CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build-output/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY     ${CMAKE_BINARY_DIR}/build-output/lib)
set (CMAKE_PDB_OUTPUT_DIRECTORY         ${CMAKE_BINARY_DIR}/build-output/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY     ${CMAKE_BINARY_DIR}/build-output/bin)

add_library (octave-ir.common INTERFACE)

target_compile_features (octave-ir.common INTERFACE cxx_std_17)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options (
    octave-ir.common
    INTERFACE
      -ftime-trace
      --coverage
      -Weverything
      -Wno-padded
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-zero-as-null-pointer-constant
      -Wno-used-but-marked-unused
      -Wno-exit-time-destructors
  )

  target_link_options (
    octave-ir.common
    INTERFACE
      --coverage
  )

  if (GCH_USE_LIBCXX_WITH_CLANG)
    target_compile_options (octave-ir.common INTERFACE -stdlib=libc++)
    target_link_options (octave-ir.common INTERFACE LINKER:--allow-multiple-definition)
    target_link_libraries (octave-ir.common INTERFACE c++ c++abi)
  endif ()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options (
    octave-ir.common
    INTERFACE
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
      -Wno-noexcept
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
    octave-ir.common
    INTERFACE
      /W4
      /wd4250
  )
endif ()

if (UNIX)
  target_compile_options (octave-ir.common INTERFACE $<$<CONFIG:Debug>:-fsanitize=address>)
  target_link_options (octave-ir.common INTERFACE $<$<CONFIG:Debug>:-fsanitize=address>)
endif ()

include (CheckLinkerFlag)
check_linker_flag (CXX -fuse-ld=lld HAVE_lld_LINK_OPTION)
if (HAVE_lld_LINK_OPTION AND NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  target_link_options (octave-ir.common INTERFACE -fuse-ld=lld)
endif ()

add_library (gch::octave-ir.common ALIAS octave-ir.common)

# Always build octave-ir.utilities
add_subdirectory (utilities)

if (GCH_OCTAVE_IR_BUILD_STATIC_IR)
  add_subdirectory (static-ir)
endif ()

if (GCH_OCTAVE_IR_BUILD_DYNAMIC_IR)
  add_subdirectory (dynamic-ir)
endif ()

if (GCH_OCTAVE_IR_BUILD_COMPILER)
  add_subdirectory (compiler)
endif ()

if (GCH_OCTAVE_IR_BUILD_TESTS)
  add_subdirectory (test)
endif ()

set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY     ${CMAKE_BINARY_DIR}/build-output/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY     ${CMAKE_BINARY_DIR}/build-output/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY     ${CMAKE_BINARY_DIR}/build-output/bin)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  string (REGEX REPLACE "/[wW]([0-4deovX]|all) ?" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif ()

add_library (octave-ir.common INTERFACE)

target_compile_features (octave-ir.common INTERFACE cxx_std_${GCH_OCTAVE_IR_CXX_STANDARD})

if (NOT DEFINED CMAKE_CXX_STANDARD_REQUIRED)
  set (CMAKE_CXX_STANDARD_REQUIRED OFF)
endif ()

if (NOT DEFINED CMAKE_CXX_EXTENSIONS)
  set (CMAKE_CXX_EXTENSIONS OFF)
endif ()

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
      -Wno-documentation
      -Wno-weak-vtables
      -Wno-weak-template-vtables
      -Wno-extra-semi
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
      /permissive-
  )
endif ()

if (GCH_OCTAVE_IR_ENABLE_ASAN)
  target_compile_options (octave-ir.common INTERFACE -fsanitize=address)
  target_link_options (octave-ir.common INTERFACE -fsanitize=address)
endif ()

set_target_properties (
  octave-ir.common
  PROPERTIES
  INTERFACE_POSITION_INDEPENDENT_CODE
    ON
)

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  include (CheckLinkerFlag)
  check_linker_flag (CXX -fuse-ld=lld HAVE_lld_LINK_OPTION)
  if (HAVE_lld_LINK_OPTION)
    target_link_options (octave-ir.common INTERFACE -fuse-ld=lld)
  endif ()
endif ()

add_library (gch::octave-ir.common ALIAS octave-ir.common)

macro (add_test_executable target_name)
  add_executable (${target_name} ${ARGN})

  target_compile_features (${target_name} PRIVATE cxx_std_${GCH_OCTAVE_IR_CXX_STANDARD})

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
        /permissive-
    )
  endif ()

  if (GCH_OCTAVE_IR_ENABLE_ASAN)
    target_compile_options (${target_name} PRIVATE -fsanitize=address)
    target_link_options (${target_name} PRIVATE -fsanitize=address)
  endif ()

  if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    include (CheckLinkerFlag)
    check_linker_flag (CXX -fuse-ld=lld HAVE_lld_LINK_OPTION)
    if (HAVE_lld_LINK_OPTION)
      target_link_options (${target_name} PRIVATE -fuse-ld=lld)
    endif ()
  endif ()
endmacro ()

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

if (GCH_OCTAVE_IR_ENABLE_TESTS)
  add_subdirectory (test)
endif ()

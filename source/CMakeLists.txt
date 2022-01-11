macro (octave_ir_target_headers target_name)
  cmake_parse_arguments (oith "" "" "PRIVATE;INTERFACE;PUBLIC" "${ARGN}")

  string (REPLACE "." "/" HEADER_PATH "${target_name}")

  foreach (scope PRIVATE INTERFACE PUBLIC)
    list (TRANSFORM oith_${scope} PREPEND include/gch/${HEADER_PATH}/)
    target_sources (${target_name} ${scope} ${oith_${scope}})
    set_source_files_properties (
      ${oith_${scope}}
      PROPERTIES
      HEADER_FILE_ONLY
        ON
    )
  endforeach ()
endmacro ()

macro (octave_ir_target_sources target_name)
  cmake_parse_arguments (oits "" "" "PRIVATE;INTERFACE;PUBLIC" "${ARGN}")

  foreach (scope PRIVATE INTERFACE PUBLIC)
    list (TRANSFORM oits_${scope} PREPEND lib/)
    target_sources (${target_name} ${scope} ${oits_${scope}})
  endforeach ()
endmacro ()

add_library (octave-ir-common INTERFACE)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options (
    octave-ir-common
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
    octave-ir-common
    INTERFACE
      --coverage
  )

  if (GCH_USE_LIBCXX_WITH_CLANG)
    target_compile_options (octave-ir-common INTERFACE -stdlib=libc++)
    target_link_options (octave-ir-common INTERFACE LINKER:--allow-multiple-definition)
    target_link_libraries (octave-ir-common INTERFACE c++ c++abi)
  endif ()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options (
    octave-ir-common
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
    octave-ir-common
    INTERFACE
      /W4
      /wd4250
  )
endif ()

if (UNIX)
  target_compile_options (octave-ir-common INTERFACE $<$<CONFIG:Debug>:-fsanitize=address>)
  target_link_options (octave-ir-common INTERFACE $<$<CONFIG:Debug>:-fsanitize=address>)
endif ()

add_library (gch::octave-ir-common ALIAS octave-ir-common)

# Always build octave-ir-utilities
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

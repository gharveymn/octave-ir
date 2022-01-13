find_package (doctest)

if (doctest_FOUND)
  foreach (version 17 20)
    add_test_executable (
      octave-ir.doctest.c++${version}
      main.test.cpp
      utilities/ir-link-set.test.cpp
    )

    target_link_libraries (
      octave-ir.doctest.c++${version}
      PRIVATE
      doctest::doctest
      gch::nonnull_ptr
      gch::octave-ir.utilities
      gch::optional_ref
      gch::partition
      gch::small_vector
      gch::tracker
      plf::plf_list
    )

    set_target_properties (
      octave-ir.doctest.c++${version}
      PROPERTIES
      CXX_STANDARD
        ${version}
      CXX_STANDARD_REQUIRED
        NO
      CXX_EXTENSIONS
        NO
    )
  endforeach ()
else ()
  message (WARNING "Could not find doctest.")
endif ()

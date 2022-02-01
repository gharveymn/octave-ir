add_library (octave-ir.utilities.ir-link-set.test_common OBJECT)
target_sources (octave-ir.utilities.ir-link-set.test_common PRIVATE test_common.cpp)
target_link_libraries (octave-ir.utilities.ir-link-set.test_common PRIVATE octave-ir.utilities)

macro (add_ctest_executables)
  foreach (file ${ARGN})
    get_filename_component (_TARGET_NAME "${file}" NAME_WE)
    string (PREPEND _TARGET_NAME "octave-ir.utilities.ir-link-set.")

    add_test_executable (${_TARGET_NAME} ${file})

    target_link_libraries (
      ${_TARGET_NAME}
      PRIVATE
        octave-ir.utilities.ir-link-set.test_common
        octave-ir.utilities
    )

    add_test (
      NAME
        ${_TARGET_NAME}
      COMMAND
        ${_TARGET_NAME}
    )

    add_dependencies(octave-ir.utilities.ctest ${_TARGET_NAME})
  endforeach ()
endmacro ()

add_ctest_executables (
  test-contains.cpp
  test-copy-assignment-operator.cpp
  test-copy-constructor.cpp
  test-element-insert.cpp
  test-element-insert-repeats.cpp
  test-emplace.cpp
  test-equal_range.cpp
  test-erase.cpp
  test-erase-key.cpp
  test-erase_if.cpp
  test-find.cpp
  test-ilist-constructor.cpp
  test-ilist-constructor-repeats.cpp
  test-ilist-constructor-repeats-multiple.cpp
  test-ilist-insert.cpp
  test-ilist-insert-repeats.cpp
  test-ilist-insert-repeats-multiple.cpp
  test-lower_bound.cpp
  test-move-assignment-operator.cpp
  test-move-constructor.cpp
  test-multiple-merges.cpp
  test-range-constructor.cpp
  test-range-insert.cpp
  test-sub-lv-lv.cpp
  test-sub-lv-rv.cpp
  test-sub-rv-lv.cpp
  test-sub-rv-rv.cpp
  test-subeq.cpp
  test-swap.cpp
  test-upper_bound.cpp
)

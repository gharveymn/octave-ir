CREATE_PACKAGE (AUTHOR       gharveymn
                NAMESPACE    gch
                NAME         octave-ir-utilities
                TYPE         STATIC

                HEADERS      ir-allocator-util.hpp
                             ir-common.hpp
                             ir-error.hpp
                             ir-functional.hpp
                             ir-index-sequence-map.hpp
                             ir-iterator.hpp
                             ir-link-set.hpp
                             ir-optional-util.hpp
                             ir-type-traits.hpp
                             ir-utility.hpp

                SOURCES      ir-error.cpp
                             ir-link-set.cpp
                             ir-utility.cpp

                DEPENDENCIES nonnull_ptr
                             optional_ref
                             small_vector)

TARGET_COMPILE_OPTIONS (octave-ir-utilities PRIVATE ${OCTAVE_IR_COMPILE_OPTS})
TARGET_LINK_OPTIONS    (octave-ir-utilities PRIVATE ${OCTAVE_IR_LINK_OPTS})
TARGET_LINK_LIBRARIES  (octave-ir-utilities PRIVATE ${OCTAVE_IR_LINK_LIBS})

SET_TARGET_PROPERTIES (octave-ir-utilities
                       PROPERTIES CXX_STANDARD          17
                                  CXX_STANDARD_REQUIRED NO
                                  CXX_EXTENSIONS        NO)

SET_PROPERTY (GLOBAL APPEND PROPERTY OCTAVE_IR_LIBS octave-ir-utilities)

CREATE_PACKAGE (AUTHOR             gharveymn
                NAMESPACE          gch
                NAME               octave-dynamic-ir
                TYPE               STATIC
                NO_HEADERS_PREFIX
                NO_PRINT
                DEPENDENCIES nonnull_ptr
                             octave-ir-utilities
                             octave-static-ir
                             optional_ref
                             partition
                             plf_list
                             select-iterator
                             small_vector
                             tracker)

TARGET_COMPILE_OPTIONS (octave-dynamic-ir PRIVATE ${OCTAVE_IR_COMPILE_OPTS})
TARGET_LINK_OPTIONS    (octave-dynamic-ir PRIVATE ${OCTAVE_IR_LINK_OPTS})
TARGET_LINK_LIBRARIES  (octave-dynamic-ir PRIVATE ${OCTAVE_IR_LINK_LIBS})

SET_TARGET_PROPERTIES (octave-dynamic-ir
                       PROPERTIES CXX_STANDARD          17
                                  CXX_STANDARD_REQUIRED NO
                                  CXX_EXTENSIONS        NO)

ADD_SUBDIRECTORY (include)
ADD_SUBDIRECTORY (lib)

SET_PROPERTY (GLOBAL APPEND PROPERTY OCTAVE_IR_LIBS octave-dynamic-ir)

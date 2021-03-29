CREATE_PACKAGE (AUTHOR       gharveymn
                NAMESPACE    gch
                NAME         octave-static-ir
                TYPE         STATIC

                HEADERS      ir-constant.hpp
                             ir-metadata.hpp
                             ir-static-block.hpp
                             ir-static-module.hpp
                             ir-static-def.hpp
                             ir-static-instruction.hpp
                             ir-static-operand.hpp
                             ir-static-use.hpp
                             ir-static-variable.hpp
                             ir-type.hpp
                             ir-type-base.hpp
                             ir-type-std.hpp

                SOURCES      ir-constant.cpp
                             ir-metadata.cpp
                             ir-static-block.cpp
                             ir-static-module.cpp
                             ir-static-def.cpp
                             ir-static-instruction.cpp
                             ir-static-operand.cpp
                             ir-static-use.cpp
                             ir-static-variable.cpp
                             ir-type.cpp
                             ir-type-base.cpp
                             ir-type-std.cpp

                DEPENDENCIES nonnull_ptr
                             octave-ir-utilities
                             optional_ref
                             small_vector)

TARGET_COMPILE_OPTIONS (octave-static-ir PRIVATE ${OCTAVE_IR_COMPILE_OPTS})
TARGET_LINK_OPTIONS    (octave-static-ir PRIVATE ${OCTAVE_IR_LINK_OPTS})
TARGET_LINK_LIBRARIES  (octave-static-ir PRIVATE ${OCTAVE_IR_LINK_LIBS})

SET_TARGET_PROPERTIES (octave-static-ir
                       PROPERTIES CXX_STANDARD          17
                                  CXX_STANDARD_REQUIRED NO
                                  CXX_EXTENSIONS        NO)

SET_PROPERTY (GLOBAL APPEND PROPERTY OCTAVE_IR_LIBS octave-static-ir)

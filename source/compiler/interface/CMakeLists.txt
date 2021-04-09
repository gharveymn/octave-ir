CREATE_PACKAGE (AUTHOR       gharveymn
                NAMESPACE    gch
                NAME         octave-ir-compiler
                HEADERS      octave-ir-compiler.hpp
                DEPENDENCIES octave-ir-static-ir)

SET_PROPERTY (GLOBAL APPEND PROPERTY OCTAVE_IR_LIBS octave-ir-compiler)

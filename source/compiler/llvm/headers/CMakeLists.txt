target_sources (
  octave-ir.compiler-llvm
  PRIVATE
    mappers/arithmetic-mappers.hpp
    mappers/bitwise-mappers.hpp
    mappers/relation-mappers.hpp
    function-translator.hpp
    instruction-translator.hpp
    llvm-common.hpp
    llvm-constant.hpp
    llvm-interface.hpp
    llvm-type.hpp
    llvm-value-map.hpp
    llvm-version.hpp
)

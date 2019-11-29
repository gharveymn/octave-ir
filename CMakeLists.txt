CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
PROJECT(octave_ir)

SET(CMAKE_CXX_STANDARD_REQUIRED 11)
SET(CMAKE_CXX_EXTENSIONS OFF)

#SET(CMAKE_CXX_FLAGS "-ftime-report")

SET(OCTAVE_BUILD "/home/gene/workspace/octave/dev/octave/build")
SET(OCTAVE_SOURCE "/home/gene/workspace/octave/dev/octave/source")


#INCLUDE_DIRECTORIES(/home/gene/workspace/octave/dev/octave/bin/x86_64-pc-linux-gnu/debug/include/octave-6.0.0/octave)
#INCLUDE_DIRECTORIES("C:\\Octave\\Octave-5.1.0.0\\mingw64\\include\\octave-5.1.0\\octave")

INCLUDE_DIRECTORIES(external/plf_list)

ADD_LIBRARY(octave_ir SHARED
		ir-common.h
		ir-common-util.cc
		ir-common-util.h
		ir-component.cc
		ir-component.h
		ir-instruction.cc
		ir-instruction.h
		ir-function.cc
		ir-function.h
		ir-operand.cc
		ir-operand.h
		ir-structure.cc
		ir-structure.h
		ir-type.cc
		ir-type.h
		ir-type-base.cc
		ir-type-base.h
		ir-type-std.cc
		ir-type-std.h
#		ir-value.cc
#		ir-value.h
		ir-variable.cc
		ir-variable.h
#		ir-visitor.cc
#		ir-visitor.h
		octave-config.h
          ir-type-extra.cc
		ir-type-extra.h
		ir-block.cc
		ir-block.h
		ir-constant.cc
		ir-constant.h
#		ir-build.cc
#		ir-build.h
		tracker.h ir-instruction-fwd.h)

IF (MSVC)
#	target_compile_options(octave_ir PRIVATE -Wall)
ELSE()
#	target_compile_options(octave_ir PRIVATE -Wall -Wextra -pedantic)
ENDIF()

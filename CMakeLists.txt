CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
PROJECT(octave_ir)

SET(CMAKE_CXX_STANDARD 11)

#SET(CMAKE_CXX_FLAGS "-ftime-report")

ADD_LIBRARY(octave_ir SHARED
		ir-common.h
		ir-common-util.cc
		ir-common-util.h
		ir-component.cc
		ir-component.h
		ir-instruction.cc
		ir-instruction.h
		ir-module.cc
		ir-module.h
		ir-operand.cc
		ir-operand.h
		ir-structure.cc
		ir-structure.h
		ir-type.cc
		ir-type.h
		ir-type-base.cc
		ir-type-base.h
		ir-value.cc
		ir-value.h
		ir-variable.cc
		ir-variable.h
#		ir-visitor.cc
#		ir-visitor.h
		octave-config.h
          ir-type-extra.cc
		ir-type-extra.h
		ir-block.cc
		ir-block.h ir-constant.cc ir-constant.h)

IF (MSVC)
#	target_compile_options(octave_ir PRIVATE -Wall)
ELSE()
#	target_compile_options(octave_ir PRIVATE -Wall -Wextra -pedantic)
ENDIF()
target_sources (
  octave-ir-dynamic-ir
  PRIVATE
  ir-structure-visitors.hpp
  ir-structure-visitors-fwd.hpp
)

ADD_SUBDIRECTORY (inspectors)
ADD_SUBDIRECTORY (mutators)

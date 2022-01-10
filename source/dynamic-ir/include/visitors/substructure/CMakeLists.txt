target_sources (
  octave-ir-dynamic-ir
  PRIVATE
  ir-substructure-visitors.hpp
  ir-substructure-visitors-fwd.hpp
)

ADD_SUBDIRECTORY (inspectors)
ADD_SUBDIRECTORY (mutators)

target_sources (
  octave-ir-dynamic-ir
  PRIVATE
  ir-subcomponent-visitors.hpp
  ir-subcomponent-visitors-fwd.hpp
)

ADD_SUBDIRECTORY (inspectors)
ADD_SUBDIRECTORY (mutators)

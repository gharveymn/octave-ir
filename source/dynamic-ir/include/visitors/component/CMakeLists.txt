target_sources (
  octave-ir-dynamic-ir
  PRIVATE
  ir-component-visitors.hpp
  ir-component-visitors-fwd.hpp
)

ADD_SUBDIRECTORY (inspectors)
ADD_SUBDIRECTORY (mutators)

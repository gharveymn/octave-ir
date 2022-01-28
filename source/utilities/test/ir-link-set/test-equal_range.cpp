#include "test_common.hpp"

using namespace gch;

int
main (void)
{
  ir_link_set l {
    blks[6],
    blks[2],
    blks[7],
    blks[5]
  };

  auto v = l.equal_range (blks[6]);
  CHECK (! v.empty ());
  CHECK (v.begin () == std::next (l.begin (), 2));
  CHECK (v.end () == std::next (l.begin (), 3));

  auto w = l.equal_range (blks[1]);
  CHECK (w.empty ());

  return 0;
}

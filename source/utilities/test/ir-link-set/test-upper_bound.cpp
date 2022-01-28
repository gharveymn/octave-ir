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

  CHECK (l.upper_bound (blks[6]) == std::next (l.begin (), 3));
  CHECK (l.upper_bound (blks[1]) == l.begin ());
  CHECK (l.upper_bound (blks[8]) == l.end ());

  return 0;
}

#include "test_common.hpp"

using namespace gch;

int
main (void)
{
  ir_link_set l {
    blks[6],
    blks[2],
    blks[4],
    blks[9]
  };

  CHECK (l.contains (blks[2]));
  CHECK (! l.contains (blks[3]));

  return 0;
}

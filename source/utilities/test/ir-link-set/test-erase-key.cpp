#include "test_common.hpp"

using namespace gch;

int
main (void)
{
  ir_link_set l {
    blks[6],
    blks[2],
    blks[4],
    blks[9],
    blks[3],
    blks[1],
    blks[5],
    blks[7]
  };

  const ir_link_set r {
    blks[2],
    blks[3],
    blks[4],
    blks[5],
    blks[6],
    blks[9],
  };

  CHECK (l.erase (blks[1]));
  CHECK (l.erase (blks[7]));
  CHECK (! l.erase (blks[8]));

  CHECK (l == r);

  return 0;
}

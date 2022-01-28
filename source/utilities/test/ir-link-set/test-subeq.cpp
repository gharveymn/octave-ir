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

  const ir_link_set r {
    blks[6],
    blks[7],
  };

  const ir_link_set res {
    blks[2],
    blks[5],
  };

  l -= r;

  CHECK (res == l);

  return 0;
}

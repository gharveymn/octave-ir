#include "test_common.hpp"

using namespace gch;

int
main (void)
{
  const ir_link_set r {
    blks[7],
    blks[6],
  };

  const ir_link_set res {
    blks[2],
    blks[5],
  };

  CHECK (res == (ir_link_set { blks[6], blks[2], blks[7], blks[5] } - r));

  return 0;
}

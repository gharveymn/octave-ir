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

  CHECK (2 == erase_if (l, [&](nonnull_ptr<test_struct> b)
  {
    return b < &blks[6];
  }));
  CHECK (l == r);

  return 0;
}

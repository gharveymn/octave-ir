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

  ir_link_set r {
    blks[3],
    blks[1],
    blks[5],
    blks[7]
  };

  const ir_link_set l_copy (l);
  const ir_link_set r_copy (r);

  l.swap (r);
  CHECK (r == l_copy);
  CHECK (l == r_copy);

  using std::swap;
  swap (l, r);
  CHECK (l == l_copy);
  CHECK (r == r_copy);

  return 0;
}

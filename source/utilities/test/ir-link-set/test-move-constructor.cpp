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

  ir_link_set l_copy (l);
  ir_link_set r (std::move (l));

  CHECK (r == l_copy);

  return 0;
}

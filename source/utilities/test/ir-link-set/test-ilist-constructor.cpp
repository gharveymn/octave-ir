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

  CHECK (std::is_sorted (l.begin (), l.end ()));
  CHECK (l.size () == 4);

  return 0;
}

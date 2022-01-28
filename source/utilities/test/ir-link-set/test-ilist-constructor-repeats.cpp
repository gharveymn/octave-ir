#include "test_common.hpp"

using namespace gch;

int
main (void)
{
  ir_link_set l {
    blks[6],
    blks[6],
    blks[6],
    blks[6]
  };

  CHECK (std::is_sorted (l.begin (), l.end ()));
  CHECK (l.size () == 1);

  return 0;
}

#include "test_common.hpp"

using namespace gch;

int
main (void)
{
  ir_link_set l {
    blks[6],
    blks[6],
    blks[3],
    blks[3]
  };

  CHECK (std::is_sorted (l.begin (), l.end ()));
  CHECK (l.size () == 2);

  return 0;
}

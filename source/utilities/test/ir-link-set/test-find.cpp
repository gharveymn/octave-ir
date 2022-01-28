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

  CHECK (l.find (blks[6]) == std::next (l.begin (), 2));
  CHECK (l.find (blks[1]) == l.end ());

  return 0;
}

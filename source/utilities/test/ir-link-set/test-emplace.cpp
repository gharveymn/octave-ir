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

  auto p = l.emplace (blks[3]);
  CHECK (p.position == std::next (l.begin ()));
  CHECK (p.inserted);
  CHECK (l.emplace (blks[1]).inserted);
  l.emplace (blks[5]);
  l.emplace (blks[7]);

  CHECK (! l.emplace (blks[1]).inserted);
  CHECK (l.emplace (blks[1]).position == l.begin ());

  CHECK (std::is_sorted (l.begin (), l.end ()));
  CHECK (l.size () == 8);

  return 0;
}

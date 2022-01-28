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

  std::array<nonnull_ptr<test_struct>, 4> a {
    nonnull_ptr { blks[3] },
    nonnull_ptr { blks[1] },
    nonnull_ptr { blks[5] },
    nonnull_ptr { blks[7] }
  };

  l.insert (a.begin (), a.end ());
  CHECK (std::is_sorted (l.begin (), l.end ()));
  CHECK (l.size () == 8);

  return 0;
}

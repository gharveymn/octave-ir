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

  l.insert (nonnull_ptr { blks[3] });
  l.insert (nonnull_ptr { blks[1] });
  l.insert (nonnull_ptr { blks[5] });
  l.insert (nonnull_ptr { blks[7] });

  CHECK (std::is_sorted (l.begin (), l.end ()));
  CHECK (l.size () == 8);

  return 0;
}

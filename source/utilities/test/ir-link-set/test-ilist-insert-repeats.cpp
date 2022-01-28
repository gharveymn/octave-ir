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

  l.insert ({
              nonnull_ptr { blks[6] },
              nonnull_ptr { blks[6] },
              nonnull_ptr { blks[4] },
              nonnull_ptr { blks[2] }
            });

  CHECK (std::is_sorted (l.begin (), l.end ()));
  CHECK (l.size () == 4);

  return 0;
}

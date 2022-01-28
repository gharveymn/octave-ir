#include "test_common.hpp"

using namespace gch;

int
main (void)
{
  std::array<nonnull_ptr<test_struct>, 4> a {
    nonnull_ptr { blks[6] },
    nonnull_ptr { blks[2] },
    nonnull_ptr { blks[4] },
    nonnull_ptr { blks[9] }
  };

  ir_link_set<test_struct> l (a.begin (), a.end ());

  CHECK (std::is_sorted (l.begin (), l.end ()));
  CHECK (l.size () == 4);

  return 0;
}

/** ir-link-set.test.cpp.c
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "doctest/doctest.h"

#include "utilities/ir-link-set.hpp"

#include <gch/nonnull_ptr.hpp>

#include <algorithm>
#include <chrono>
#include <random>
#include <iostream>

using namespace gch; // yeah, yeah, we're in a source file, it's fine

struct test_struct
{ };

static std::array<test_struct, 10> blks { };

TEST_CASE ("range constructor")
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
}

TEST_CASE ("ilist constructor")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[4],
    blks[9]
  };

  CHECK (std::is_sorted (l.begin (), l.end ()));
  CHECK (l.size () == 4);
}

TEST_CASE ("ilist constructor (repeats)")
{

  ir_link_set l {
    blks[6],
    blks[6],
    blks[6],
    blks[6]
  };

  CHECK (std::is_sorted (l.begin (), l.end ()));
  CHECK (l.size () == 1);
}

TEST_CASE ("ilist constructor (multiple repeats)")
{

  ir_link_set l {
    blks[6],
    blks[6],
    blks[3],
    blks[3]
  };

  CHECK (std::is_sorted (l.begin (), l.end ()));
  CHECK (l.size () == 2);
}

TEST_CASE ("copy constructor")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[4],
    blks[9]
  };

  ir_link_set r (l);

  CHECK (r == l);
}

TEST_CASE ("move constructor")
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
}

TEST_CASE ("copy assignment operator")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[4],
    blks[9]
  };

  ir_link_set<test_struct> r;
  r = l;

  CHECK (r == l);
}

TEST_CASE ("move assignment operator")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[4],
    blks[9]
  };

  ir_link_set l_copy (l);
  ir_link_set<test_struct> r;
  r = std::move (l);

  CHECK (r == l_copy);
}

TEST_CASE ("element insert")
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
}

TEST_CASE ("element insert (repeats)")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[4],
    blks[9]
  };

  l.insert (nonnull_ptr { blks[6] });
  l.insert (nonnull_ptr { blks[6] });
  l.insert (nonnull_ptr { blks[2] });
  l.insert (nonnull_ptr { blks[4] });

  CHECK (std::is_sorted (l.begin (), l.end ()));
  CHECK (l.size () == 4);
}

TEST_CASE ("range insert")
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
}

TEST_CASE ("ilist insert")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[4],
    blks[9]
  };

  l.insert ({
    nonnull_ptr { blks[3] },
    nonnull_ptr { blks[1] },
    nonnull_ptr { blks[5] },
    nonnull_ptr { blks[7] }
  });

  CHECK (std::is_sorted (l.begin (), l.end ()));
  CHECK (l.size () == 8);
}

TEST_CASE ("ilist insert (repeats)")
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
}

TEST_CASE ("ilist insert (multiple repeats)")
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
}

TEST_CASE ("emplace")
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
}

TEST_CASE ("erase")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[4],
    blks[9],
    blks[3],
    blks[1],
    blks[5],
    blks[7]
  };

  const ir_link_set r {
    blks[2],
    blks[3],
    blks[4],
    blks[5],
    blks[6],
    blks[9],
  };

  l.erase (l.begin ());
  l.erase (std::next (l.begin (), 5));

  CHECK (l == r);
}

TEST_CASE ("erase key")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[4],
    blks[9],
    blks[3],
    blks[1],
    blks[5],
    blks[7]
  };

  const ir_link_set r {
    blks[2],
    blks[3],
    blks[4],
    blks[5],
    blks[6],
    blks[9],
  };

  CHECK (l.erase (blks[1]));
  CHECK (l.erase (blks[7]));
  CHECK (! l.erase (blks[8]));

  CHECK (l == r);
}

TEST_CASE ("swap")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[4],
    blks[9]
  };

  ir_link_set r {
    blks[3],
    blks[1],
    blks[5],
    blks[7]
  };

  const ir_link_set l_copy (l);
  const ir_link_set r_copy (r);

  l.swap (r);
  CHECK (r == l_copy);
  CHECK (l == r_copy);

  using std::swap;
  swap (l, r);
  CHECK (l == l_copy);
  CHECK (r == r_copy);
}

TEST_CASE ("find")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[4],
    blks[9]
  };

  CHECK (l.find (blks[6]) == std::next (l.begin (), 2));
  CHECK (l.find (blks[1]) == l.end ());
}

TEST_CASE ("contains")
{
  ir_link_set l {
    blks[6],
    blks[2],
    blks[4],
    blks[9]
  };

  CHECK (l.contains (blks[2]));
  CHECK (! l.contains (blks[3]));
}

TEST_CASE ("equal_range")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[7],
    blks[5]
  };

  auto v = l.equal_range (blks[6]);
  CHECK (! v.empty ());
  CHECK (v.begin () == std::next (l.begin (), 2));
  CHECK (v.end () == std::next (l.begin (), 3));

  auto w = l.equal_range (blks[1]);
  CHECK (w.empty ());
}

TEST_CASE ("lower_bound")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[7],
    blks[5]
  };

  CHECK (l.lower_bound (blks[6]) == std::next (l.begin (), 2));
  CHECK (l.lower_bound (blks[1]) == l.begin ());
  CHECK (l.lower_bound (blks[8]) == l.end ());
}

TEST_CASE ("upper_bound")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[7],
    blks[5]
  };

  CHECK (l.upper_bound (blks[6]) == std::next (l.begin (), 3));
  CHECK (l.upper_bound (blks[1]) == l.begin ());
  CHECK (l.upper_bound (blks[8]) == l.end ());
}

TEST_CASE ("erase_if")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[7],
    blks[5]
  };

  const ir_link_set r {
    blks[6],
    blks[7],
  };

  CHECK (2 == erase_if (l, [&](nonnull_ptr<test_struct> b)
                            {
                              return b < &blks[6];
                            }));
  CHECK (l == r);
}

TEST_CASE ("operator-=")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[7],
    blks[5]
  };

  const ir_link_set r {
    blks[6],
    blks[7],
  };

  const ir_link_set res {
    blks[2],
    blks[5],
  };

  l -= r;

  CHECK (res == l);
}

TEST_CASE ("operator- (&&, const &)")
{

  const ir_link_set r {
    blks[7],
    blks[6],
  };

  const ir_link_set res {
    blks[2],
    blks[5],
  };

  CHECK (res == (ir_link_set { blks[6], blks[2], blks[7], blks[5] } - r));
}

TEST_CASE ("operator- (const &, &&)")
{

  const ir_link_set r {
    blks[7],
    blks[6],
  };

  const ir_link_set res {
    blks[2],
    blks[5],
  };

  CHECK (res == (ir_link_set { blks[6], blks[2], blks[7], blks[5] } - r));
}

TEST_CASE ("operator- (&&, &&)")
{

  const ir_link_set res {
    blks[2],
    blks[5],
  };

  CHECK (res == (  ir_link_set { blks[6], blks[2], blks[7], blks[5] }
                  - ir_link_set { blks[7], blks[6] }));
}

TEST_CASE ("operator- (const &, const &)")
{

  ir_link_set l {
    blks[6],
    blks[2],
    blks[7],
    blks[5]
  };

  const ir_link_set r {
    blks[6],
    blks[7],
  };

  const ir_link_set res {
    blks[2],
    blks[5],
  };

  CHECK (res == l - r);
}

TEST_CASE ("multiple merges")
{
  using namespace std::chrono;
  using clock = high_resolution_clock;
  using time = clock::time_point;

#ifdef NDEBUG
  constexpr std::size_t num_values  = 100000000;
  constexpr std::size_t num_samples = 10000000;
#else
  constexpr std::size_t num_values  = 100000;
  constexpr std::size_t num_samples = 10000;
#endif

  std::vector<test_struct> s (num_values);
  std::vector<nonnull_ptr<test_struct>> ptrs;
  ptrs.reserve (num_values);

  std::transform (s.begin (), s.end (), std::back_inserter (ptrs),
                  [](test_struct& b) { return nonnull_ptr { b }; });

  std::vector<nonnull_ptr<test_struct>> v;
  v.reserve (num_values);

  std::sample (ptrs.begin (), ptrs.end (), std::back_inserter (v), num_samples,
               std::mt19937 { std::random_device { } () });

  const ir_link_set<test_struct> l (v.begin (), v.end ());

  std::sample (ptrs.begin (), ptrs.end (), v.begin (), num_samples,
               std::mt19937 { std::random_device { } () });

  const ir_link_set<test_struct> r (v.begin (), v.end ());
  {
    constexpr auto num_merges = 5;
    std::array<ir_link_set<test_struct>, num_merges> p;
    p.fill (l);
    std::array<ir_link_set<test_struct>, num_merges> q;
    q.fill (r);

    time t1 = clock::now ();
    auto it = q.begin ();
    std::for_each (p.begin (), p.end (), [&it](auto&& x) { x.merge (*it++); });
    time t2 = clock::now ();
    std::cout << num_merges
              << " merges done in  "
              << duration_cast<duration<double>> (t2 - t1).count ()
              << " ms."
              << std::endl;
  }
}

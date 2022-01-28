#include "test_common.hpp"

#include <chrono>
#include <random>
#include <iostream>

using namespace gch;

int
main (void)
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

  return 0;
}

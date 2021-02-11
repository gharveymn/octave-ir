/** scratch.cpp
 * Short description here.
 *
 * Copyright © 2020 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ir-component-handle.hpp"
#include "ir-common-util.hpp"
#include "ir-optional-util.hpp"
#include "ir-link-set.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>

#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <numeric>
#include <stack>
#include <optional>
#include <vector>
#include <random>

namespace gch
{

  class ir_component { };

  class ir_def_timeline;
  class ir_block { int x; };

  struct incoming_block
  {
    gch::nonnull_ptr<ir_block> block;
    gch::optional_ref<ir_def_timeline> tl;
  };

  struct incoming_component
  {
    gch::nonnull_ptr<ir_component> comp;
    std::vector<incoming_block> leaves;
  };

  struct phi_manager
  {
    std::vector<incoming_component> incoming;
  };

  struct management_stack
  {
    std::stack<phi_manager> stack;
  };

  const std::vector<int>&
  test_ext_helper (const std::vector<int>& v = { 1, 2, 3, 4 })
  {
    return v;
  }

  std::vector<int>
  test_ext (void)
  {
    return test_ext_helper ();
  }

  struct test_struct
  {
    std::string
    g (void)
    {
      return "hi";
    }

    std::string
    g (void) const
    {
      return "hello";
    }

    std::string
    f (void) const
    {
      return "f";
    }
  };

  std::optional<test_struct>
  get_opt (void)
  {
    return { { } };
  }

  std::optional<const test_struct>
  get_copt (void)
  {
    return { { } };
  }

  void
  test_bind (void)
  {
    // operator>>= (get_opt (), &test_struct::g);
    // operator>>= (get_copt (), &test_struct::g);
    std::cout << (get_opt ()  >>= &test_struct::g) << std::endl;
    std::cout << (get_copt () >>= &test_struct::g) << std::endl;

    const test_struct x;
    std::invoke (&test_struct::f, x);
  }

  std::optional<int>
  add (std::optional<int> mx, std::optional<int> my)
  {
    return mx >>= [my](int x) { return my >>= [x](int y) { return std::optional { x + y }; } ; };
  }

  struct my_class
  {
    my_class (std::unique_ptr<my_class>&& p, int x)
      : m_p (std::move (p)),
        m_x (x)
    { }

    std::unique_ptr<my_class> m_p;
    int m_x;
  };

  struct test_accum
  {
    test_accum (void)
      : m_s ("hi")
    { }

    test_accum (const test_accum& other)
      : m_s (other.m_s)
    {
      ++num_copies;
    }

    test_accum (test_accum&& other) noexcept
      : m_s (std::move (other.m_s))
    {
      ++num_moves;
    }

    test_accum&
    operator= (const test_accum& other)
    {
      if (&other != this)
        m_s = other.m_s;
      ++num_copy_assigns;
      return *this;
    }

    test_accum&
    operator= (test_accum&& other)
    {
      if (&other != this)
        m_s = std::move (other.m_s);
      ++num_move_assigns;
      return *this;
    }

    void
    append (const std::string& s)
    {
      m_s.append (s);
    }

    static
    void
    reset_nums (void)
    {
      num_copies       = 0;
      num_moves        = 0;
      num_copy_assigns = 0;
      num_move_assigns = 0;
    }

    static inline std::size_t num_copies;
    static inline std::size_t num_moves;
    static inline std::size_t num_copy_assigns;
    static inline std::size_t num_move_assigns;

  private:
    std::string m_s;
  };

  test_accum
  test_a_impl (void)
  {
    return { };
  }

  test_accum
  test_a (void)
  {
    test_accum x = test_a_impl ();
    x.append ("dsf");
    return x;
  }

  [[nodiscard]]
  static
  ir_link_set<ir_block>
  copy_leaves (int comp)
  {
    std::vector<ir_block> blks (10 + comp);
    return {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };
  }

  template <typename ...Args>
  [[nodiscard]]
  static
  ir_link_set<ir_block>
  copy_leaves (int comp, Args&&... args)
  {
    return (copy_leaves (comp) | ... | copy_leaves (std::forward<Args> (args)));
  }

}

static
void
test_links (void)
{
  using namespace gch;

  std::array<ir_block, 10> blks { };

  auto convert
    = [&](nonnull_ptr<ir_block> p)
      {
        return std::to_string (p.get () - blks.data ());
      };

  auto print
    = [&](const ir_link_set<ir_block>& l)
      {
        std::string s = std::accumulate (std::next (l.begin ()),
                                         l.end (),
                                         convert (*l.begin ()),
                                         [&](std::string s, nonnull_ptr<ir_block> p)
                                         {
                                           return std::move (s) + ", " + convert (p);
                                         });
        std::cout << "[ " << s << " ]" << std::endl;
      };

  {
    // range constructor

    std::array<nonnull_ptr<ir_block>, 4> a {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    ir_link_set<ir_block> l (a.begin (), a.end ());

    assert (std::is_sorted (l.begin (), l.end ()));
    assert (l.size () == 4);
  }
  {
    // ilist constructor

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    assert (std::is_sorted (l.begin (), l.end ()));
    assert (l.size () == 4);
  }
  {
    // ilist constructor (repeats)

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[6] }
    };

    assert (std::is_sorted (l.begin (), l.end ()));
    assert (l.size () == 1);
  }
  {
    // ilist constructor (repeats)

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[3] },
      nonnull_ptr { blks[3] }
    };

    assert (std::is_sorted (l.begin (), l.end ()));
    assert (l.size () == 2);
  }
  {
    // copy constructor

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    ir_link_set r (l);

    assert (r == l);
  }
  {
    // move constructor

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    ir_link_set l_copy (l);
    ir_link_set r (std::move (l));

    assert (r == l_copy);
  }
  {
    // copy assignment operator

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    ir_link_set<ir_block> r;
    r = l;

    assert (r == l);
  }
  {
    // move assignment operator

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    ir_link_set l_copy (l);
    ir_link_set<ir_block> r;
    r = std::move (l);

    assert (r == l_copy);
  }
  {
    // element insert

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };
    print (l);

    l.insert (nonnull_ptr { blks[3] });
    print (l);

    l.insert (nonnull_ptr { blks[1] });
    print (l);

    l.insert (nonnull_ptr { blks[5] });
    print (l);

    l.insert (nonnull_ptr { blks[7] });
    print (l);

    assert (std::is_sorted (l.begin (), l.end ()));
    assert (l.size () == 8);
  }
  {
    // element insert (repeats)

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    l.insert (nonnull_ptr { blks[6] });
    l.insert (nonnull_ptr { blks[6] });
    l.insert (nonnull_ptr { blks[2] });
    l.insert (nonnull_ptr { blks[4] });

    assert (std::is_sorted (l.begin (), l.end ()));
    assert (l.size () == 4);
  }
  {
    // range insert

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    std::array<nonnull_ptr<ir_block>, 4> a {
      nonnull_ptr { blks[3] },
      nonnull_ptr { blks[1] },
      nonnull_ptr { blks[5] },
      nonnull_ptr { blks[7] }
    };

    l.insert (a.begin (), a.end ());
    assert (std::is_sorted (l.begin (), l.end ()));
    assert (l.size () == 8);
  }
  {
    // ilist insert

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    l.insert ({
      nonnull_ptr { blks[3] },
      nonnull_ptr { blks[1] },
      nonnull_ptr { blks[5] },
      nonnull_ptr { blks[7] }
    });

    assert (std::is_sorted (l.begin (), l.end ()));
    assert (l.size () == 8);
  }
  {
    // ilist insert (repeats)

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    l.insert ({
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[2] }
    });

    assert (std::is_sorted (l.begin (), l.end ()));
    assert (l.size () == 4);
  }
  {
    // ilist insert (repeats)

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    l.insert ({
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[2] }
    });

    assert (std::is_sorted (l.begin (), l.end ()));
    assert (l.size () == 4);
  }
  {
    // emplace

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    auto p = l.emplace (blks[3]);
    assert (p.position == std::next (l.begin ()));
    assert (p.inserted);
    assert (l.emplace (blks[1]).inserted);
    l.emplace (blks[5]);
    l.emplace (blks[7]);

    assert (! l.emplace (blks[1]).inserted);
    assert (l.emplace (blks[1]).position == l.begin ());

    assert (std::is_sorted (l.begin (), l.end ()));
    assert (l.size () == 8);
  }
  {
    // erase

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] },
      nonnull_ptr { blks[3] },
      nonnull_ptr { blks[1] },
      nonnull_ptr { blks[5] },
      nonnull_ptr { blks[7] }
    };

    const ir_link_set r {
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[3] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[5] },
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[9] },
    };

    l.erase (l.begin ());
    l.erase (std::next (l.begin (), 5));

    assert (l == r);
  }
  {
    // erase key

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] },
      nonnull_ptr { blks[3] },
      nonnull_ptr { blks[1] },
      nonnull_ptr { blks[5] },
      nonnull_ptr { blks[7] }
    };

    const ir_link_set r {
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[3] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[5] },
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[9] },
    };

    assert (l.erase (blks[1]));
    assert (l.erase (blks[7]));
    assert (! l.erase (blks[8]));

    assert (l == r);
  }
  {
    // swap

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    ir_link_set r {
      nonnull_ptr { blks[3] },
      nonnull_ptr { blks[1] },
      nonnull_ptr { blks[5] },
      nonnull_ptr { blks[7] }
    };

    const ir_link_set l_copy (l);
    const ir_link_set r_copy (r);

    l.swap (r);
    assert (r == l_copy);
    assert (l == r_copy);

    using std::swap;
    swap (l, r);
    assert (l == l_copy);
    assert (r == r_copy);
  }
  {
    // find

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    assert (l.find (blks[6]) == std::next (l.begin (), 2));
    assert (l.find (blks[1]) == l.end ());
  }
  {
    // contains
    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[4] },
      nonnull_ptr { blks[9] }
    };

    assert (l.contains (blks[2]));
    assert (! l.contains (blks[3]));
  }
  {
    // equal_range

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[7] },
      nonnull_ptr { blks[5] }
    };

    auto v = l.equal_range (blks[6]);
    assert (! v.empty ());
    assert (v.begin () == std::next (l.begin (), 2));
    assert (v.end () == std::next (l.begin (), 3));

    auto w = l.equal_range (blks[1]);
    assert (w.empty ());
  }
  {
    // lower_bound

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[7] },
      nonnull_ptr { blks[5] }
    };

    assert (l.lower_bound (blks[6]) == std::next (l.begin (), 2));
    assert (l.lower_bound (blks[1]) == l.begin ());
    assert (l.lower_bound (blks[8]) == l.end ());
  }
  {
    // upper_bound

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[7] },
      nonnull_ptr { blks[5] }
    };

    assert (l.upper_bound (blks[6]) == std::next (l.begin (), 3));
    assert (l.upper_bound (blks[1]) == l.begin ());
    assert (l.upper_bound (blks[8]) == l.end ());
  }
  {
    // erase_if

    ir_link_set l {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[2] },
      nonnull_ptr { blks[7] },
      nonnull_ptr { blks[5] }
    };

    const ir_link_set r {
      nonnull_ptr { blks[6] },
      nonnull_ptr { blks[7] },
    };

    assert (2 == erase_if (l, [&](nonnull_ptr<ir_block> b)
                              {
                                return b < nonnull_ptr { blks[6] };
                              }));
    assert (l == r);
  }
  {
    // ir_link_set<ir_block>::reset_nums ();
    ir_link_set l (copy_leaves (1, 2, 3, 4, 5));
    // std::cout << "num copies:       " << ir_link_set<ir_block>::num_copies << std::endl;
    // std::cout << "num moves:        " << ir_link_set<ir_block>::num_moves << std::endl;
    // std::cout << "num copy assigns: " << ir_link_set<ir_block>::num_copy_assigns << std::endl;
    // std::cout << "num move assigns: " << ir_link_set<ir_block>::num_move_assigns << std::endl;
  }
  {
    using namespace std::chrono;
    using clock = high_resolution_clock;
    using time = clock::time_point;

    // constexpr std::size_t num_values  = 10000000;
    // constexpr std::size_t num_samples = 1000000;

    constexpr std::size_t num_values  = 100000;
    constexpr std::size_t num_samples = 10000;

    std::vector<ir_block> s (num_values);
    std::vector<nonnull_ptr<ir_block>> ptrs;
    ptrs.reserve (num_values);

    std::transform (s.begin (), s.end (), std::back_inserter (ptrs),
                    [](ir_block& b) { return nonnull_ptr { b }; });

    std::vector<nonnull_ptr<ir_block>> v;
    v.reserve (num_values);

    std::sample (ptrs.begin (), ptrs.end (), std::back_inserter (v), num_samples,
                 std::mt19937 { std::random_device { } () });

    const ir_link_set<ir_block> l (v.begin (), v.end ());

    std::sample (ptrs.begin (), ptrs.end (), v.begin (), num_samples,
                 std::mt19937 { std::random_device { } () });

    const ir_link_set<ir_block> r (v.begin (), v.end ());
    {
      ir_link_set p (l);
      ir_link_set q (r);

      time t1 = clock::now ();
      p.merge (q);
      time t2 = clock::now ();
      std::cout << "merge done in  "
                << duration_cast<duration<double>> (t2 - t1).count ()
                << " ms."
                << std::endl;
    }
  }
}

int
main (void)
{
  using namespace gch;

  test_links ();

  management_stack s;
  ir_component_storage xs = std::make_unique<ir_component> ();
  ir_component_storage ys = std::make_unique<ir_component> ();

  ir_component_ptr x { &xs };
  ir_component_ptr y { &ys };

  assert (x != y);

  optional_ref<ir_component> o { *x };
  static_cast<void> (o);
  assert (o == x);
  assert (o != y);

  nonnull_ptr<ir_component> n { *x };
  static_cast<void> (n);
  assert (n == x);
  assert (n != y);

  ir_component *p = x.get_component_pointer ();
  static_cast<void> (p);
  assert (p == x);
  assert (p != y);

  const ir_component& r = *x;
  static_cast<void> (r);
  assert (&r == x);
  assert (&r != y);

  int yy = 1;
  optional_ref op { *nonnull_ptr<int> { yy } };

  // int * xx = optional_ref<int>::to_address (nonnull_ptr<int> { yy });

  std::byte b;
  b >>= 2;

  std::vector<int> v = test_ext ();
  test_bind ();

  std::optional<int> mx { 1 };
  std::optional<int> my { 2 };

  assert (add (mx, my) == 3);
  assert (add (mx, std::nullopt) == std::nullopt);
  assert (add (std::nullopt, my) == std::nullopt);
  assert (add (std::nullopt, std::nullopt) == std::nullopt);

  auto u = std::make_unique<my_class> (std::make_unique<my_class> (nullptr, 3), 2 );
  auto *up = &u;
  *up = std::move ((*up)->m_p);

  test_accum::reset_nums ();
  std::vector<std::string> va (15, "hi");

  auto ta = std::accumulate (va.begin (), va.end (), test_accum { },
                             [](auto&& ta, const std::string& s) -> decltype (auto)
                             {
                               ta.append (s);
                               return std::move (ta);
                             });

  std::cout << "num copies:       " << test_accum::num_copies << std::endl;
  std::cout << "num moves:        " << test_accum::num_moves << std::endl;
  std::cout << "num copy assigns: " << test_accum::num_copy_assigns << std::endl;
  std::cout << "num move assigns: " << test_accum::num_move_assigns << std::endl;

  test_accum::reset_nums ();
  test_a ();
  std::cout << "num copies:       " << test_accum::num_copies << std::endl;
  std::cout << "num moves:        " << test_accum::num_moves << std::endl;
  std::cout << "num copy assigns: " << test_accum::num_copy_assigns << std::endl;
  std::cout << "num move assigns: " << test_accum::num_move_assigns << std::endl;

  return 0;
}

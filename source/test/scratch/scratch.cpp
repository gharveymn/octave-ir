/** scratch.cpp
 * Short description here.
 *
 * Copyright © 2020 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "components/utility/ir-component-handle.hpp"
#include "ir-utility.hpp"
#include "ir-optional-util.hpp"
#include "ir-link-set.hpp"
#include "ir-visitor.hpp"

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

  class ir_subcomponent { };

  class ir_def_timeline;
  class ir_block { };

  struct incoming_block
  {
    gch::nonnull_ptr<ir_block> block;
    gch::optional_ref<ir_def_timeline> tl;
  };

  struct incoming_component
  {
    gch::nonnull_ptr<ir_subcomponent> comp;
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

  static
  const std::vector<int>&
  test_ext_helper (const std::vector<int>& v = { 1, 2, 3, 4 })
  {
    return v;
  }

  static
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

  static
  std::optional<test_struct>
  get_opt (void)
  {
    return { { } };
  }

  static
  std::optional<const test_struct>
  get_copt (void)
  {
    return { { } };
  }

  static
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

  static
  std::optional<int>
  add (std::optional<int> mx, std::optional<int> my)
  {
    return mx >>= [my](int x) { return my >>= [x](int y) { return std::optional { x + y }; }; };
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

  static
  test_accum
  test_a_impl (void)
  {
    return { };
  }

  static
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
  copy_leaves (unsigned comp)
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
  copy_leaves (Args&&... args)
  {
    return (copy_leaves (std::forward<Args> (args)) | ...);
  }

  static
  void
  test_links (void)
  {
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
                                           [&](std::string str, nonnull_ptr<ir_block> p)
                                           {
                                             return std::move (str) + ", " + convert (p);
                                           });
          std::cout << "[ " << s << " ]" << std::endl;
        };
  }

// #define DO_VISITOR_TESTS
#ifdef DO_VISITOR_TESTS

  //*****************************************************************
// Pre-declare the shapes.
//*****************************************************************
  class Square;
  class Circle;
  class Triangle;

  class Shape;

  class ShapeVisitor;
  class StaticVisitor;
  class ConstShapeVisitor;

  template <>
  struct visitor_traits<ShapeVisitor>
  {
    using result_type      = void;
    using visitor_category = mutator_tag;
  };

  template <>
  struct visitor_traits<StaticVisitor>
  {
    using result_type      = void;
    using visitor_category = mutator_tag;
  };

  template <>
  struct visitor_traits<ConstShapeVisitor>
  {
    using result_type      = void;
    using visitor_category = inspector_tag;
  };

//*****************************************************************
// The shape visitor base class.
// Pure virtual 'Visit' functions will be defined for the Square,
// Circle, and Triangle types.
//*****************************************************************
  class ShapeVisitor
    : public abstract_mutator<Square, Circle, Triangle>
  {
  public:
    ShapeVisitor            (void)                    = default;
    ShapeVisitor            (const ShapeVisitor&)     = default;
    ShapeVisitor            (ShapeVisitor&&) noexcept = default;
    ShapeVisitor& operator= (const ShapeVisitor&)     = default;
    ShapeVisitor& operator= (ShapeVisitor&&) noexcept = default;
    ~ShapeVisitor           (void) override;
  };

  ShapeVisitor::
  ~ShapeVisitor (void) = default;

  class ConstShapeVisitor
    : public abstract_inspector<Square, Circle, Triangle>
  {
  public:
    ConstShapeVisitor            (void)                         = default;
    ConstShapeVisitor            (const ConstShapeVisitor&)     = default;
    ConstShapeVisitor            (ConstShapeVisitor&&) noexcept = default;
    ConstShapeVisitor& operator= (const ConstShapeVisitor&)     = default;
    ConstShapeVisitor& operator= (ConstShapeVisitor&&) noexcept = default;
    ~ConstShapeVisitor           (void) override;
  };

  ConstShapeVisitor::
  ~ConstShapeVisitor (void) = default;

  class Square;
  class Circle;
  class Triangle;

  class StaticVisitor;

  using shape_mutators   = visitor_types<ShapeVisitor, StaticVisitor>;
  using shape_inspectors = visitor_types<ConstShapeVisitor>;
  using shape_visitors   = visitor_types<shape_mutators, shape_inspectors>;

  //*****************************************************************
// The shape base class.
//*****************************************************************
  class Shape
    : public abstract_visitable<shape_visitors>
  {
  public:
    Shape            (void)             = default;
    Shape            (const Shape&)     = default;
    Shape            (Shape&&) noexcept = default;
    Shape& operator= (const Shape&)     = default;
    Shape& operator= (Shape&&) noexcept = default;
    virtual ~Shape   (void)             = 0;
  };

  Shape::
  ~Shape (void) = default;

//*****************************************************************
// The square class
//*****************************************************************
  class Square
    : public Shape,
      public visitable<Square, shape_visitors>
  {
  public:
    Square            (void)              = default;
    Square            (const Square&)     = default;
    Square            (Square&&) noexcept = default;
    Square& operator= (const Square&)     = default;
    Square& operator= (Square&&) noexcept = default;
    ~Square           (void) override;
  };

  Square::
  ~Square (void) = default;

//*****************************************************************
// The circle class
//*****************************************************************
  class Circle
    : public Shape,
      public visitable<Circle, shape_visitors>
  {
  public:
    Circle            (void)              = default;
    Circle            (const Circle&)     = default;
    Circle            (Circle&&) noexcept = default;
    Circle& operator= (const Circle&)     = default;
    Circle& operator= (Circle&&) noexcept = default;
    ~Circle           (void) override;
  };

  Circle::
  ~Circle (void) = default;

//*****************************************************************
// The triangle class
//*****************************************************************
  class Triangle
    : public Shape,
      public visitable<Triangle, shape_visitors>
  {
  public:
    Triangle            (void)                = default;
    Triangle            (const Triangle&)     = default;
    Triangle            (Triangle&&) noexcept = default;
    Triangle& operator= (const Triangle&)     = default;
    Triangle& operator= (Triangle&&) noexcept = default;
    ~Triangle           (void) override;
  };

  Triangle::
  ~Triangle (void) = default;

  template <>
  auto
  acceptor<Square, ShapeVisitor>::
  accept (visitor_reference v)
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<Circle, ShapeVisitor>::
  accept (visitor_reference v)
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<Triangle, ShapeVisitor>::
  accept (visitor_reference v)
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<Square, ConstShapeVisitor>::
  accept (visitor_reference v) const
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<Circle, ConstShapeVisitor>::
  accept (visitor_reference v) const
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<Triangle, ConstShapeVisitor>::
  accept (visitor_reference v) const
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  //*****************************************************************
// The 'draw' visitor.
//*****************************************************************
  class DrawVisitor
    : public ShapeVisitor
  {
  public:
    DrawVisitor            (void)                   = default;
    DrawVisitor            (const DrawVisitor&)     = default;
    DrawVisitor            (DrawVisitor&&) noexcept = default;
    DrawVisitor& operator= (const DrawVisitor&)     = default;
    DrawVisitor& operator= (DrawVisitor&&) noexcept = default;
    ~DrawVisitor           (void) override;

    void
    visit (Square&) override
    {
      std::cout << "Draw the square\n";
    }

    void
    visit (Circle&) override
    {
      std::cout << "Draw the circle\n";
    }

    void
    visit (Triangle&) override
    {
      std::cout << "Draw the triangle\n";
    }
  };

  DrawVisitor::
  ~DrawVisitor (void) = default;

//*****************************************************************
// The 'serialise' visitor.
//*****************************************************************
  class SerialiseVisitor
    : public ConstShapeVisitor
  {
  public:
    SerialiseVisitor            (void)                        = default;
    SerialiseVisitor            (const SerialiseVisitor&)     = default;
    SerialiseVisitor            (SerialiseVisitor&&) noexcept = default;
    SerialiseVisitor& operator= (const SerialiseVisitor&)     = default;
    SerialiseVisitor& operator= (SerialiseVisitor&&) noexcept = default;
    ~SerialiseVisitor           (void) override;

    void
    visit (const Square&) override
    {
      std::cout << "Serialise the square\n";
    }

    void
    visit (const Circle&) override
    {
      std::cout << "Serialise the circle\n";
    }

    void
    visit (const Triangle&) override
    {
      std::cout << "Serialise the triangle\n";
    }
  };

  SerialiseVisitor::
  ~SerialiseVisitor (void) = default;

  class StaticVisitor
  {
  public:
    template <typename, typename, typename>
    friend class acceptor;

    void
    operator() (Shape& s)
    {
      s.accept (*this);
    }

  private:
    static
    void
    visit (Square&)
    {
      std::cout << "static square\n";
    }

    static
    void
    visit (Circle&)
    {
      std::cout << "static circle\n";
    }

    static
    void
    visit (Triangle&)
    {
      std::cout << "static triangle\n";
    }
  };

  template <>
  auto
  acceptor<Square, StaticVisitor>::
  accept (visitor_reference v)
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<Circle, StaticVisitor>::
  accept (visitor_reference v)
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  template <>
  auto
  acceptor<Triangle, StaticVisitor>::
  accept (visitor_reference v)
  -> result_type
  {
    return v.visit (static_cast<concrete_reference> (*this));
  }

  static
  void
  test_visitor (void)
  {
    //*****************************************************************
    // The actual visitors.
    //*****************************************************************
    DrawVisitor      draw_visitor;
    SerialiseVisitor serialise_visitor;
    StaticVisitor    static_visitor_v;

    //*****************************************************************
    // The list of shapes.
    //*****************************************************************
    std::vector<Shape *> shape_list;

    auto ApplyMutator
      = [&](ShapeVisitor &visitor)
        {
          std::for_each (shape_list.begin (), shape_list.end (),
                         [&](Shape *u) { u->accept (visitor); });
        };

    auto ApplyInspector
      = [&](ConstShapeVisitor &visitor)
        {
          std::for_each (shape_list.begin (), shape_list.end (),
                         [&](Shape *u) { u->accept (visitor); });
        };

    auto ApplyStatic
      = [&](StaticVisitor &visitor)
        {
          std::for_each (shape_list.begin (), shape_list.end (),
                         [&](Shape *u) { visitor (*u); });
        };

    // Create some shapes.
    Square   square;
    Circle   circle;
    Triangle triangle;

    // Add them to the vector
    shape_list.push_back(&square);
    shape_list.push_back(&circle);
    shape_list.push_back(&triangle);

    // Apply the visitors.
    ApplyMutator (draw_visitor);
    ApplyInspector (serialise_visitor);
    ApplyStatic (static_visitor_v);
  }

#endif

  struct blah
  {
    int x;
    long y;
    std::string z;

    bool
    operator() (ir_block&)
    {
      ++x;
      ++y;
      z += "hi";
      return true;
    }
  };

  static
  void
  f (void)
  {
    ir_block bl;
    blah b { };
    std::function<bool (ir_block&)> func { std::move (b) };
    assert (typeid (b) == func.target_type ());
    blah *p1 = func.target<blah> ();
    func (bl);
    std::function<bool (ir_block&)> func2 = std::move (func);
    blah *p2 = func2.target<blah> ();
    (void)p1;
    (void)p2;
    func2 (bl);
  }

  struct some_class;

  struct some_other_class
  {
    // std::stack<some_class> m_stack;
  };

  struct some_class
  {
    some_class            (void)                  = default;
    some_class            (const some_class&)     = default;
    some_class            (some_class&&) noexcept = default;
    some_class& operator= (const some_class&)     = default;
    some_class& operator= (some_class&&) noexcept = default;
    ~some_class           (void)                  = default;

    std::vector<some_class> v;
    small_vector<some_class, 0> m_self;
  };

  struct blahk;

  // template class std::vector<blahk>;
  // template class gch::small_vector<int>;
  // template class gch::small_vector<blahk, 0>;

  struct incom
  {
    incom            (void)             = default;
    incom            (const incom&)     = default;
    incom            (incom&&) noexcept = default;
    incom& operator= (const incom&)     = default;
    incom& operator= (incom&&) noexcept = default;
    ~incom           (void)             = default;

    std::vector<blahk> v;
    small_vector<blahk, 0> sv;
  };

  struct blahk
  { };

  struct blahc
  {
    blahc            (void)             = default;
    blahc            (const blahc&)     = delete;
    blahc            (blahc&&) noexcept = delete;
    blahc& operator= (const blahc&)     = delete;
    blahc& operator= (blahc&&) noexcept = delete;
    ~blahc           (void)             = delete;
  };

#ifdef GCH_LIB_CONCEPTS

  static_assert (! concepts::MoveInsertable<blahc, small_vector<blahc>, std::allocator<blahc>>);
  static_assert (! concepts::Erasable<blahc, small_vector<blahc>, std::allocator<blahc>>);


  template <typename T>
  void
  t1 (T *)
  {
    // static_assert (! std::is_array_v<T>);
  }

  static
  void
  fff (void)
  {
    using arr = int[3];
    arr a { };
    arr *p = &a;
    t1 (p);

    int *b;
    blahk k;
    blahk *kp = &k;

    static_assert (std::is_same_v<decltype (kp->~blahk ()), void>);
    // t1 (b);
  }

  template <typename T, typename Enable = void>
  struct dest1
    : std::false_type
  { };

  template <typename T>
  struct dest1<T, std::void_t<decltype (std::declval<T *> ()->~T ())>>
    : std::true_type
  { };

  template <typename T, typename Enable = void>
  struct dest
    : std::false_type
  { };

  template <typename T, typename Enable = void>
  struct deref
    : std::false_type
  { };

  template <typename T>
  struct deref<T, std::void_t<decltype (*std::declval<T> ())>>
    : dest<std::remove_reference_t<decltype (*std::declval<T> ())>>
  { };

  template <typename T>
  struct dest<T, std::enable_if_t<std::is_destructible_v<std::remove_all_extents_t<T>>>>
    : std::true_type
  { };

  static_assert (! dest<blahc>::value);
  static_assert (! dest<blahc[3]>::value);
  static_assert (dest<blahk>::value);
  static_assert (dest<blahk[3]>::value);

  struct tt { };

  static_assert (! concepts::Erasable<blahc, small_vector<blahc>, std::allocator<blahc>>);
  static_assert (! concepts::Erasable<blahc, small_vector<blahc>, std::allocator<blahc>>);
  static_assert (concepts::Erasable<tt, small_vector<tt>, std::allocator<tt>>);
  static_assert (concepts::Erasable<tt[3], small_vector<tt[3]>, std::allocator<tt[3]>>);
  static_assert (! dest<blahc[3]>::value);
  static_assert (dest<blahk>::value);
  static_assert (dest<blahk[3]>::value);

#endif

  static
  void
  g (void)
  {
    some_class x { };
    (void)x;
    // x.m_self.begin ();
    x.v.emplace_back ();
    x.m_self.emplace_back ();

    some_class xx { };
    xx = std::move (x);

    some_other_class y { };
    (void)y;
    // y.m_stack.emplace ();

    incom z { };
    z.sv.push_back ({ });

    // void* sv = new small_vector<blahc> ();
    // sv.push_back ({ });
  }

  struct abstract_base
  {
    virtual ~abstract_base (void) = 0;
  };

  abstract_base::
  ~abstract_base (void) = default;

  struct other_base
  {
    virtual ~other_base (void);
  };

  other_base::
  ~other_base (void) = default;

  struct concrete1
    : abstract_base
  {
    concrete1            (void)                 = default;
    concrete1            (const concrete1&)     = default;
    concrete1            (concrete1&&) noexcept = default;
    concrete1& operator= (const concrete1&)     = default;
    concrete1& operator= (concrete1&&) noexcept = default;
    ~concrete1           (void) override;
  };

  concrete1::
  ~concrete1 (void) = default;

  struct concrete2
    : abstract_base,
      other_base
  {
    concrete2            (void)                 = default;
    concrete2            (const concrete2&)     = default;
    concrete2            (concrete2&&) noexcept = default;
    concrete2& operator= (const concrete2&)     = default;
    concrete2& operator= (concrete2&&) noexcept = default;
    ~concrete2           (void) override;
  };

  concrete2::
  ~concrete2 (void) = default;

  struct concrete3
    : abstract_base,
      other_base
  {
    concrete3            (void)                 = default;
    concrete3            (const concrete3&)     = default;
    concrete3            (concrete3&&) noexcept = default;
    concrete3& operator= (const concrete3&)     = default;
    concrete3& operator= (concrete3&&) noexcept = default;
    ~concrete3           (void) override;
  };

  concrete3::
  ~concrete3 (void) = default;

  static
  void
  test_dy (void)
  {
    concrete1 x;
    concrete2 y;
    concrete3 z;
    abstract_base *p = &x;
    assert (  is_a<concrete1> (p));
    assert (! is_a<concrete2> (p));
    assert (! is_a<other_base> (p));

    p = &y;
    assert (! is_a<concrete1> (p));
    assert (  is_a<concrete2> (p));
    assert (  is_a<other_base> (p));

    other_base *o = &y;
    assert (! is_a<concrete1> (o));
    assert (  is_a<concrete2> (o));
    assert (! is_a<concrete3> (o));
    assert (  is_a<abstract_base> (o));

    o = &z;
    assert (! is_a<concrete1> (o));
    assert (! is_a<concrete2> (o));
    assert (  is_a<concrete3> (o));
    assert (  is_a<abstract_base> (o));
  }

  struct test_move_struct
  {
    test_move_struct            (void)                        = default;
    test_move_struct            (const test_move_struct&)     { }
    test_move_struct            (test_move_struct&&) noexcept { }
    test_move_struct& operator= (const test_move_struct&)     { return *this; }
    test_move_struct& operator= (test_move_struct&&) noexcept { return *this; }
    ~test_move_struct           (void)                        = default;
  };

  static
  test_move_struct
  ggg (void)
  {
    return { };
  }

  static
  void
  test_move (void)
  {
    test_move_struct x { };
    x = ggg ();
    x = ggg ();
  }

  class nontrivial
  {
  public:
    nontrivial (void)
      : x (3),
        y (4)
    { }

  private:
    int x;
    long y;
  };

  struct test_bound_mem_fn_struct
  {
    int
    f (nontrivial&, nontrivial&&) const
    {
      return 2;
    }

    int
    f (nontrivial&, int)
    {
      return 2;
    }

    int
    g (void) &&
    {
      return 2;
    }

    int
    h (void)
    {
      return 3;
    }

    int data;
  };

  static
  void
  test_bound_mem_fn (void)
  {
    test_bound_mem_fn_struct x;
    auto l = bound_mem_fn (x, &test_bound_mem_fn_struct::f);
    nontrivial s;
    x.f (s, std::move (s));

    auto func = &test_bound_mem_fn_struct::h;

    l (s, 1);

    auto gg = bound_mem_fn (std::move (x), &test_bound_mem_fn_struct::g);
    int r = gg ();

    auto ll = bound_mem_fn (x, &test_bound_mem_fn_struct::data);

    // static_assert (std::is_invocable_v<decltype (&test_bound_mem_fn_struct::f), test_bound_mem_fn_struct, nontrivial&, nontrivial&&>);
    // static_assert (std::is_invocable_v<decltype (l), nontrivial&, nontrivial&&>);
  }

  // template <typename T, typename Functor>
  // static constexpr
  // auto
  // invoke_blah (T&& x, Functor&& f)
  //   noexcept (noexcept (std::forward<Functor> (f) (std::forward<T> (x))))
  //   -> std::enable_if_t<
  //        std::is_same_v<void, decltype (std::forward<Functor> (f) (std::forward<T> (x)))>, void>
  // {
  //   std::forward<Functor> (f) (std::forward<T> (x));
  // }
  //
  // static
  // void
  // do_invoke_blah (void)
  // {
  //   std::string s;
  //   auto f = [](void) -> void { };
  //   invoke_blah (s, f);
  // }

}

int
main (void)
{
  using namespace gch;

  std::vector<ir_component_mover> ret { };

  test_move ();
  test_dy ();
  f ();
  g ();

#ifdef DO_VISITOR_TESTS
  test_visitor ();
#endif

  test_links ();

  management_stack s;
  ir_component_storage xs = std::make_unique<ir_subcomponent> ();
  ir_component_storage ys = std::make_unique<ir_subcomponent> ();

  ret.emplace_back (xs);

  ir_component_ptr x { &xs };
  ir_component_ptr y { &ys };

  assert (x != y);

  optional_ref o { *x };
  static_cast<void> (o);
  assert (o.get_pointer () == x);
  assert (o.get_pointer () != y);

  nonnull_ptr n { *x };
  static_cast<void> (n);
  assert (n == x);
  assert (n != y);

  ir_subcomponent *p = x.get_component_pointer ();
  static_cast<void> (p);
  assert (p == x);
  assert (p != y);

  const ir_subcomponent& r = *x;
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
                             [](auto&& taa, const std::string& str) -> decltype (auto)
                             {
                               taa.append (str);
                               return std::move (taa);
                             });

  std::cout << "num copies:       " << test_accum::num_copies << std::endl;
  std::cout << "num moves:        " << test_accum::num_moves << std::endl;
  std::cout << "num copy assigns: " << test_accum::num_copy_assigns << std::endl;
  std::cout << "num move assigns: " << test_accum::num_move_assigns << std::endl << std::endl;

  test_accum::reset_nums ();
  test_a ();
  std::cout << "num copies:       " << test_accum::num_copies << std::endl;
  std::cout << "num moves:        " << test_accum::num_moves << std::endl;
  std::cout << "num copy assigns: " << test_accum::num_copy_assigns << std::endl;
  std::cout << "num move assigns: " << test_accum::num_move_assigns << std::endl;

  return 0;
}

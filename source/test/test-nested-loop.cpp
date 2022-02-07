/** test-nested-loop.cpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "test-templates.hpp"
#include <numeric>

using namespace gch;

// void
// UseCharPointer (char const volatile*) { }

// template <class Tp>
// inline void
// DoNotOptimize (Tp const& value)
// {
//   UseCharPointer (&reinterpret_cast<char const volatile&>(value));
//   _ReadWriteBarrier ();
// }

static
ir_static_function
test (void)
{
  ir_function my_func ("x", "myloopfunc");
  ir_variable& var_x = my_func.get_variable ("x");
  var_x.set_type<int> ();

  ir_variable& var_i = my_func.create_variable<int> ("i");

  ir_variable& var_j = my_func.create_variable<int> ("j");

  my_func.set_anonymous_variable_type<bool> ();

  auto& seq = dynamic_cast<ir_component_sequence&> (my_func.get_body ());

  ir_block& entry_block = get_entry_block (seq);
  auto& loop = seq.emplace_back<ir_component_loop> (my_func.get_variable ());
  auto& start_block = static_cast<ir_block&> (loop.get_start ());
  ir_block& condition_block = loop.get_condition ();
  auto& body_seq = static_cast<ir_component_sequence&> (loop.get_body ());
  auto& body_block = static_cast<ir_block&> (body_seq.front ());
  auto& update_block = static_cast<ir_block&> (loop.get_update ());

  auto& loop2 = body_seq.emplace_back<ir_component_loop> (my_func.get_variable ());
  auto& start_block2 = static_cast<ir_block&> (loop2.get_start ());
  ir_block& condition_block2 = loop2.get_condition ();
  auto& body_seq2 = static_cast<ir_component_sequence&> (loop2.get_body ());
  auto& body_block2 = static_cast<ir_block&> (body_seq2.front ());
  auto& update_block2 = static_cast<ir_block&> (loop2.get_update ());

  auto& after_block = seq.emplace_back<ir_block> ();

  entry_block.append_with_def<ir_opcode::assign> (var_x, 1);

  start_block.append_with_def<ir_opcode::assign> (var_i, 0);
  update_block.append_with_def<ir_opcode::add> (var_i, var_i, 1);

  start_block2.append_with_def<ir_opcode::assign> (var_j, 0);
  update_block2.append_with_def<ir_opcode::add> (var_j, var_j, 1);

  body_block2.append_with_def<ir_opcode::add> (var_x, var_x, 2);

  condition_block2.append_with_def<ir_opcode::lt> (condition_block2.get_condition_variable (), var_j, 3);

  condition_block.append_with_def<ir_opcode::lt> (condition_block.get_condition_variable (), var_i, 5);

  after_block.append<ir_opcode::ret> (var_x);

  return generate_static_function (my_func);
}

struct command
{
  static std::size_t curr;

  command (std::function<void(void)> func)
    : m_func (std::move (func)),
      m_idx  (curr++)
  { }

  static
  void
  reset (void)
  {
    curr = 0;
  }

  std::function<void(void)> m_func;
  std::size_t               m_idx;
};

std::size_t command::curr;

static
bool
operator< (const command& lhs, const command& rhs)
{
  return lhs.m_idx < rhs.m_idx;
}

static
void
test2 (void)
{
  ir_variable *var_x;
  ir_variable *var_i;
  ir_variable *var_j;

  ir_block *entry_block;
  ir_block *start_block;
  ir_block *condition_block;
  ir_block *body_block;
  ir_block *update_block;
  ir_block *start_block2;
  ir_block *condition_block2;
  ir_block *body_block2;
  ir_block *update_block2;
  ir_block *after_block;

  command::reset ();

  std::vector<command> commands;
  /* 0 */commands.emplace_back ([&](void) {
    entry_block->append_with_def<ir_opcode::assign> (*var_x, 1); });
  /* 1 */commands.emplace_back ([&](void) {
    entry_block->append_with_def<ir_opcode::assign> (*var_x, 1); });
  /* 2 */commands.emplace_back ([&](void) {
    start_block->append_with_def<ir_opcode::assign> (*var_i, 0); });
  /* 3 */commands.emplace_back ([&](void) {
    update_block->append_with_def<ir_opcode::add> (*var_i, *var_i, 1); });
  /* 4 */commands.emplace_back ([&](void) {
    start_block2->append_with_def<ir_opcode::assign> (*var_j, 0); });
  /* 5 */commands.emplace_back ([&](void) {
    update_block2->append_with_def<ir_opcode::add> (*var_j, *var_j, 1); });
  /* 6 */commands.emplace_back ([&](void) {
    body_block2->append_with_def<ir_opcode::add> (*var_x, *var_x, 2); });
  /* 7 */commands.emplace_back ([&](void) {
    condition_block2->append_with_def<ir_opcode::lt> (condition_block2->get_condition_variable (), *var_j, 3); });
  /* 8 */commands.emplace_back ([&](void) {
    condition_block->append_with_def<ir_opcode::lt> (condition_block->get_condition_variable (), *var_i, 5); });
  /* 9 */commands.emplace_back ([&](void) { after_block->append<ir_opcode::ret> (*var_x); });

  std::string cmp;

  double total = std::tgamma (commands.size () + 1.0);
  double curr = 0.0;

  do
  {
    // std::accumulate (std::next (commands.begin ()), commands.end (),
    //                  std::ref (std::cout << commands[0].m_idx),
    //                  [](std::ostream& out, const command& c) {
    //                    return std::ref (out << ", " << c.m_idx);
    //                  });
    // std::cout <<  "... " << std::flush;

    ir_function my_func ("x", "myloopfunc");
    var_x = &my_func.get_variable ("x");
    var_x->set_type<int> ();

    var_i = &my_func.create_variable<int> ("i");
    var_j = &my_func.create_variable<int> ("j");
    my_func.set_anonymous_variable_type<bool> ();

    auto& seq = static_cast<ir_component_sequence&> (my_func.get_body ());
    auto& loop = seq.emplace_back<ir_component_loop> (my_func.get_variable ());
    auto& body_seq = static_cast<ir_component_sequence&> (loop.get_body ());
    auto& loop2 = body_seq.emplace_back<ir_component_loop> (my_func.get_variable ());
    auto& body_seq2 = static_cast<ir_component_sequence&> (loop2.get_body ());

    entry_block = &get_entry_block (seq);
    start_block = &static_cast<ir_block&> (loop.get_start ());
    condition_block = &loop.get_condition ();
    body_block = &static_cast<ir_block&> (body_seq.front ());
    update_block = &static_cast<ir_block&> (loop.get_update ());
    start_block2 = &static_cast<ir_block&> (loop2.get_start ());
    condition_block2 = &loop2.get_condition ();
    body_block2 = &static_cast<ir_block&> (body_seq2.front ());
    update_block2 = &static_cast<ir_block&> (loop2.get_update ());
    after_block = &seq.emplace_back<ir_block> ();

    entry_block     ->set_name ("entry");
    start_block     ->set_name ("start");
    condition_block ->set_name ("condition");
    body_block      ->set_name ("body");
    update_block    ->set_name ("update");
    start_block2    ->set_name ("start2");
    condition_block2->set_name ("condition2");
    body_block2     ->set_name ("body2");
    update_block2   ->set_name ("update2");
    after_block     ->set_name ("after");

    // commands[0].m_func ();
    // commands[1].m_func ();
    // commands[2].m_func ();
    // commands[3].m_func ();
    // commands[4].m_func ();
    // commands[6].m_func ();
    // commands[7].m_func ();
    // commands[5].m_func ();
    // commands[8].m_func ();
    // commands[9].m_func ();

    std::for_each (commands.begin (), commands.end (), [](const command& c) {
      std::invoke (c.m_func);
    });

    ir_static_function func = generate_static_function (my_func);

    std::stringstream ss;
    ss << func << std::endl;

    if (cmp.empty ())
      cmp = ss.str ();
    else
    {
      if (cmp != ss.str ())
      {
        std::cerr << "Static functions are not the same!\n"
                  << "\nFirst:\n"
                  << "############################################\n"
                  << cmp
                  << "############################################\n"
                  << "\nSecond:\n"
                  << "############################################\n"
                  << ss.str ()
                  << "############################################\n"
                  << "\n";
        std::cerr << "Sequence: ";
        std::accumulate (std::next (commands.begin ()), commands.end (),
                         std::ref (std::cerr << commands[0].m_idx),
                         [](std::ostream& out, const command& c) {
                           return std::ref (out << ", " << c.m_idx);
                         });
        std::cerr << std::endl;
        abort ();
      }
      std::stringstream tmp;
      ss.swap (tmp);
    }

    // std::cout <<  "OK!" << std::endl;
    ++curr;
    if (static_cast<int> (curr) % 100 == 0)
      std::cout << 100.0 * curr/total << "%" << std::endl;

  } while (std::next_permutation (commands.begin (), commands.end ()));
}

int
main (void)
{
  // for (int i = 0; i < 100000; ++i)
  // {
  //   ir_static_function static_func = test ();
  //   DoNotOptimize (static_func);
  // }
  // test2 ();

  // return 0;

  static constexpr int expected = 31;

  ir_function my_func ("x", "myloopfunc");
  ir_variable& var_x = my_func.get_variable ("x");
  var_x.set_type<int> ();

  ir_variable& var_i = my_func.create_variable<int> ("i");

  ir_variable& var_j = my_func.create_variable<int> ("j");

  my_func.set_anonymous_variable_type<bool> ();

  auto& seq = dynamic_cast<ir_component_sequence&> (my_func.get_body ());

  ir_block& entry_block     = get_entry_block (seq);
  auto&     loop            = seq.emplace_back<ir_component_loop> (my_func.get_variable ());
  auto&     start_block     = static_cast<ir_block&> (loop.get_start ());
  ir_block& condition_block = loop.get_condition ();
  auto&     body_seq        = static_cast<ir_component_sequence&> (loop.get_body ());
  auto&     body_block      = static_cast<ir_block&> (body_seq.front ());
  auto&     update_block    = static_cast<ir_block&> (loop.get_update ());
  auto&     after_block     = static_cast<ir_block&> (loop.get_after ());

  auto&     loop2            = body_seq.emplace_back<ir_component_loop> (my_func.get_variable ());
  auto&     start_block2     = static_cast<ir_block&> (loop2.get_start ());
  ir_block& condition_block2 = loop2.get_condition ();
  auto&     body_seq2        = static_cast<ir_component_sequence&> (loop2.get_body ());
  auto&     body_block2      = static_cast<ir_block&> (body_seq2.front ());
  auto&     update_block2    = static_cast<ir_block&> (loop2.get_update ());
  auto&     after_block2     = static_cast<ir_block&> (loop2.get_after ());

  entry_block     .set_name ("entry");

  start_block     .set_name ("start");
  condition_block .set_name ("condition");
  body_block      .set_name ("body");
  update_block    .set_name ("update");
  after_block     .set_name ("after");

  start_block2    .set_name ("start2");
  condition_block2.set_name ("condition2");
  body_block2     .set_name ("body2");
  update_block2   .set_name ("update2");
  after_block2    .set_name ("after2");

  auto print_dt = [](const ir_block& block, const ir_variable& var) {
    block.maybe_get_def_timeline (var) >>= [](auto& dt) { std::cout << dt << '\n' << std::endl; };
  };

  auto print_dts = [&] {
    print_dt (entry_block,     var_x);
    print_dt (start_block,     var_x);
    print_dt (condition_block, var_x);
    print_dt (body_block,      var_x);
    print_dt (start_block2,    var_x);
    print_dt (condition_block2,var_x);
    print_dt (body_block2,     var_x);
    print_dt (update_block2,   var_x);
    print_dt (after_block2,    var_x);
    print_dt (update_block,    var_x);
    print_dt (after_block,     var_x);
  };

  entry_block.append_with_def<ir_opcode::assign> (var_x, 1);

  start_block.append_with_def<ir_opcode::assign> (var_i, 0);
  update_block.append_with_def<ir_opcode::add> (var_i, var_i, 1);

  start_block2.append_with_def<ir_opcode::assign> (var_j, 0);
  update_block2.append_with_def<ir_opcode::add> (var_j, var_j, 1);

  body_block2.append_with_def<ir_opcode::add> (var_x, var_x, 2);

  print_dts ();

  condition_block2.append_with_def<ir_opcode::lt> (condition_block2.get_condition_variable (), var_j, 3);

  condition_block.append_with_def<ir_opcode::lt> (condition_block.get_condition_variable (), var_i, 5);

  after_block.append<ir_opcode::ret> (var_x);

  ir_static_function my_static_func = generate_static_function (my_func);

  std::cout << my_static_func << std::endl << std::endl;

  auto jit = octave_jit_compiler::create<octave_jit_compiler_llvm> ();
  jit.enable_printing ();

  try
  {
    int res = invoke_compiled_function<int> (jit.compile (my_static_func));

    std::cout << "Result:    " << res << "\n";
    std::cout << "Expected:  " << expected << std::endl;

    if (expected != res)
      return 1;
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what () << std::endl;
    return 1;
  }

  return 0;
}

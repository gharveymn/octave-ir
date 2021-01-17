/*

Copyright (C) 2019 Gene Harvey

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if ! defined (ir_instruction_h)
#define ir_instruction_h 1

#include "ir-common.hpp"
#include "ir-variable.hpp"
#include "ir-type-ir.hpp"
#include "ir-instruction-fwd.hpp"
#include <unordered_map>
#include <list>
#include <vector>

#include <memory>
#include <utility>

namespace gch
{
  class ir_operand_pre
  {
  public:
    using use_pair = std::pair<nonnull_ptr<ir_use_timeline>, ir_use_timeline::citer>;

    ir_operand_pre (void)                            = delete;
    ir_operand_pre (const ir_operand_pre&)     = default;
    ir_operand_pre (ir_operand_pre&&) noexcept = default;
    ir_operand_pre& operator= (const ir_operand_pre&)     = default;
    ir_operand_pre& operator= (ir_operand_pre&&) noexcept = default;
    ~ir_operand_pre (void)                            = default;

    constexpr ir_operand_pre (use_pair&& p)
      : m_data (std::move (p))
    { }

    constexpr ir_operand_pre (ir_constant&& c)
      : m_data (std::move (c))
    { }

    ir_operand construct (ir_instruction& instr);

  private:
    std::variant<ir_constant, use_pair> m_data;
  };

  enum class ir_opcode : unsigned
  {
    _span_     =  35,

    phi        =  0,
    assign     =  1,
    call       =  2,
    fetch      =  3,
    convert    =  4,

    branch     =  5, // abstract
    cbranch    =  6,
    ucbranch   =  7,

    relation   =  8, // abstract
    eq         =  9,
    ne         = 10,
    lt         = 11,
    le         = 12,
    gt         = 13,
    ge         = 14,

    arithmetic = 15, // abstract
    add        = 16,
    sub        = 17,
    mul        = 18,
    div        = 19,
    mod        = 20,
    rem        = 21,
    neg        = 22,

    logical    = 23, // abstract
    land       = 24,
    lor        = 25,
    lnot       = 26,

    bitwise    = 27, // abstract
    band       = 28,
    bor        = 29,
    bxor       = 30,
    bshiftl    = 31,
    bashiftr   = 32,
    blshiftr   = 33,
    bnot       = 34,
  };

  class ir_instruction_metadata
  {
  public:
    enum class arity_class : int
    {
      n_ary    = -1,
      nullary  =  0,
      unary    =  1,
      binary   =  2,
      ternary  =  3,
    };

  private:
    struct impl
    {
      const char *      m_name;
      const impl *      m_base;
      const ir_opcode   m_opcode;
      const arity_class m_arity;
      const bool        m_has_return;
      const bool        m_is_abstract;
    };

    static_assert (std::is_standard_layout_v<const char*> && std::is_trivial_v<const char*>);
    static_assert (std::is_standard_layout_v<const impl*> && std::is_trivial_v<const impl*>);
    static_assert (std::is_standard_layout_v<ir_opcode> && std::is_trivial_v<ir_opcode>);
    static_assert (std::is_standard_layout_v<arity_class> && std::is_trivial_v<arity_class>);
    static_assert (std::is_standard_layout_v<bool> && std::is_trivial_v<bool>);
    static_assert (std::is_standard_layout_v<impl>
                     && std::is_trivially_constructible_v<impl, impl&>);

    template <ir_opcode Op>
    struct instance;

    static_assert (std::is_standard_layout_v<impl> && std::is_trivial_v<impl>);

    constexpr ir_instruction_metadata (const impl& impl_ref) noexcept
      : m_ptr (impl_ref)
    { }

  public:
    ir_instruction_metadata            (void)                = delete;
    ir_instruction_metadata            (const ir_instruction_metadata&)     = default;
    ir_instruction_metadata            (ir_instruction_metadata&&) noexcept = default;
    ir_instruction_metadata& operator= (const ir_instruction_metadata&)     = default;
    ir_instruction_metadata& operator= (ir_instruction_metadata&&) noexcept = default;
    ~ir_instruction_metadata           (void)                = default;

    [[nodiscard]] constexpr ir_opcode               get_opcode  (void) const noexcept { return  m_ptr->m_opcode;      }
    [[nodiscard]] constexpr const char *            get_name    (void) const noexcept { return  m_ptr->m_name;        }
    [[nodiscard]] constexpr bool                    is_abstract (void) const noexcept { return  m_ptr->m_is_abstract; }
    [[nodiscard]] constexpr ir_instruction_metadata get_base    (void) const noexcept { return *m_ptr->m_base;        }
    [[nodiscard]] constexpr bool                    has_return  (void) const noexcept { return  m_ptr->m_has_return;  }
    [[nodiscard]] constexpr arity_class             get_arity   (void) const noexcept { return  m_ptr->m_arity;       }

    [[nodiscard]] constexpr bool has_base (void) const noexcept { return m_ptr->m_base != nullptr; }

    [[nodiscard]] constexpr bool is_n_ary   (void)  const noexcept  { return  get_arity () ==  arity_class::n_ary;   }
    [[nodiscard]] constexpr bool is_nullary (void)  const noexcept  { return  get_arity () ==  arity_class::nullary; }
    [[nodiscard]] constexpr bool is_unary   (void)  const noexcept  { return  get_arity () ==  arity_class::unary;   }
    [[nodiscard]] constexpr bool is_binary  (void)  const noexcept  { return  get_arity () ==  arity_class::binary;  }
    [[nodiscard]] constexpr bool is_ternary (void)  const noexcept  { return  get_arity () ==  arity_class::ternary; }

    [[nodiscard]] constexpr
    bool
    valid_num_args (const std::size_t n) const noexcept
    {
      return is_n_ary () || (n == static_cast<std::size_t> (get_arity ()));
    }

    [[nodiscard]] constexpr
    bool
    operator== (const ir_instruction_metadata& other) const noexcept
    {
      return m_ptr == other.m_ptr;
    }

    [[nodiscard]] constexpr
    bool
    operator!= (const ir_instruction_metadata& other) const noexcept
    {
      return ! (other == *this);
    }

    template <ir_opcode Op>
    [[nodiscard]]
    static constexpr
    ir_instruction_metadata
    get (void) noexcept { return instance<Op>::data; }

    [[nodiscard]] constexpr
    bool
    is_a (ir_instruction_metadata cmp) const noexcept
    {
      return (cmp == *this) || (has_base () && get_base ().is_a (cmp));
    }

    template <ir_opcode Op>
    [[nodiscard]] constexpr bool is_a (void) const noexcept
    {
      return is_a (get<Op> ());
    }

    template <ir_opcode Op>
    [[nodiscard]]
    static constexpr ir_opcode opcode (void) noexcept
    {
      return get<Op> ().get_opcode ();
    }

    template <ir_opcode Op>
    [[nodiscard]]
    static constexpr const char * name (void) noexcept
    {
      return get<Op> ().get_name ();
    }

    template <ir_opcode Op>
    [[nodiscard]]
    static constexpr bool is_abstract (void) noexcept
    {
      return get<Op> ().is_abstract ();
    }

    template <ir_opcode Op>
    [[nodiscard]]
    static constexpr bool has_base (void) noexcept
    {
      return get<Op> ().has_base ();
    }

    template <ir_opcode Op>
    [[nodiscard]]
    static constexpr std::enable_if_t<get<Op> ().has_base (), ir_instruction_metadata>
    base (void) noexcept
    {
      return get<Op> ().get_base ();
    }

    template <ir_opcode Op>
    [[nodiscard]]
    static constexpr bool has_return (void) noexcept
    {
      return get<Op> ().has_return ();
    }

    template <ir_opcode Op>
    [[nodiscard]]
    static constexpr arity_class arity (void) noexcept
    {
      return get<Op> ().get_arity ();
    }

    template <ir_opcode BaseOp>
    [[nodiscard]]
    static constexpr bool is_a (ir_instruction_metadata m) noexcept
    {
      return m.is_a (get<BaseOp> ());
    }

    template <ir_opcode Op, ir_opcode BaseOp>
    [[nodiscard]]
    static constexpr bool is_a (void) noexcept
    {
      return is_a<BaseOp> (get<Op> ());
    }

    // template <ir_opcode Op> static constexpr auto opcode_v      = opcode<Op> ();
    // template <ir_opcode Op> static constexpr auto name_v        = name<Op> ();
    // template <ir_opcode Op> static constexpr auto is_abstract_v = is_abstract<Op> ();
    // template <ir_opcode Op> static constexpr auto has_base_v    = has_base<Op> ();
    // template <ir_opcode Op> static constexpr auto base_v        = base<Op> ();
    // template <ir_opcode Op> static constexpr auto has_return_v  = has_return<Op> ();
    // template <ir_opcode Op> static constexpr auto arity_v       = arity<Op> ();

  private:

    template <ir_opcode Op>
    [[nodiscard]]
    static constexpr const impl* get_pointer (void) noexcept
    {
      return get<Op> ().m_ptr;
    }

    template <ir_opcode Op, ir_opcode BaseOp>
    static constexpr impl create_type (const char* name, bool has_return,
                                       arity_class n = arity<BaseOp> (),
                                       bool is_abstract = false) noexcept
    {
      return { name, get_pointer<BaseOp> (), Op, n, has_return, is_abstract };
    }

    template <ir_opcode Op, ir_opcode BaseOp>
    static constexpr impl create_type (const char* name, arity_class n,
                                       bool is_abstract = false) noexcept
    {
      return { name, get_pointer<BaseOp> (), Op, n, has_return<BaseOp> (), is_abstract };
    }

    template <ir_opcode Op, ir_opcode BaseOp>
    static constexpr impl create_type (const char* name, bool is_abstract = false) noexcept
    {
      return { name, get_pointer<BaseOp> (), Op, arity<BaseOp> (), has_return<BaseOp> (),
               is_abstract };
    }

    template <ir_opcode Op>
    static constexpr impl create_type (const char* name, bool has_return,
                                       arity_class n = arity_class::n_ary,
                                       bool is_abstract = false) noexcept
    {
      return { name, nullptr, Op, n, has_return, is_abstract };
    }

    template <ir_opcode Op>
    static constexpr impl create_type (const char* name, bool has_return,
                                       bool is_abstract = false) noexcept
    {
      return { name, nullptr, Op, arity_class::n_ary, has_return, is_abstract };
    }

    template <ir_opcode Op>
    static constexpr impl create_type (const char* name, arity_class n = arity_class::n_ary,
                                       bool is_abstract = false) noexcept
    {
      return { name, nullptr, Op, n, false, is_abstract };
    }

    nonnull_ptr<const impl> m_ptr;
  };

  template <ir_opcode Op>
  inline constexpr
  ir_instruction_metadata
  ir_instruction_metadata_v = ir_instruction_metadata::get<Op> ();

  template <ir_opcode Op>
  struct ir_instruction_type_t
  {
    explicit ir_instruction_type_t (void) = default;
  };

  template <ir_opcode Op>
  inline constexpr ir_instruction_type_t<Op> ir_instruction_type { };

  template <> struct ir_instruction_metadata::instance<ir_opcode::phi>     { static constexpr impl data { create_type<ir_opcode::phi>     ("phi",     true, arity_class::n_ary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::assign>  { static constexpr impl data { create_type<ir_opcode::assign>  ("assign",  true, arity_class::unary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::call>    { static constexpr impl data { create_type<ir_opcode::call>    ("call",    true, arity_class::n_ary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::fetch>   { static constexpr impl data { create_type<ir_opcode::fetch>   ("fetch",   true, arity_class::unary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::convert> { static constexpr impl data { create_type<ir_opcode::convert> ("convert", true, arity_class::unary) }; };

  template <> struct ir_instruction_metadata::instance<ir_opcode::branch>   { static constexpr impl data { create_type<ir_opcode::branch> ("branch", false, arity_class::n_ary) }; };     // abstract
  template <> struct ir_instruction_metadata::instance<ir_opcode::cbranch>  { static constexpr impl data { create_type<ir_opcode::cbranch, ir_opcode::branch>  ("br",  arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::ucbranch> { static constexpr impl data { create_type<ir_opcode::ucbranch, ir_opcode::branch> ("ubr", arity_class::unary) }; };

  template <> struct ir_instruction_metadata::instance<ir_opcode::relation> { static constexpr impl data { create_type<ir_opcode::relation> ("relation", true, arity_class::binary, true) }; };     // abstract
  template <> struct ir_instruction_metadata::instance<ir_opcode::eq>       { static constexpr impl data { create_type<ir_opcode::eq, ir_opcode::relation> ("==") }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::ne>       { static constexpr impl data { create_type<ir_opcode::ne, ir_opcode::relation> ("!=") }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::lt>       { static constexpr impl data { create_type<ir_opcode::lt, ir_opcode::relation> ("<") }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::le>       { static constexpr impl data { create_type<ir_opcode::le, ir_opcode::relation> ("<=") }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::gt>       { static constexpr impl data { create_type<ir_opcode::gt, ir_opcode::relation> (">") }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::ge>       { static constexpr impl data { create_type<ir_opcode::ge, ir_opcode::relation> (">=") }; };

  template <> struct ir_instruction_metadata::instance<ir_opcode::arithmetic> { static constexpr impl data { create_type<ir_opcode::arithmetic> ("arithmetic", true, true) }; };   // abstract
  template <> struct ir_instruction_metadata::instance<ir_opcode::add>        { static constexpr impl data { create_type<ir_opcode::add, ir_opcode::arithmetic> ("+",   arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::sub>        { static constexpr impl data { create_type<ir_opcode::sub, ir_opcode::arithmetic> ("-",   arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::mul>        { static constexpr impl data { create_type<ir_opcode::mul, ir_opcode::arithmetic> ("*",   arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::div>        { static constexpr impl data { create_type<ir_opcode::div, ir_opcode::arithmetic> ("/",   arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::mod>        { static constexpr impl data { create_type<ir_opcode::mod, ir_opcode::arithmetic> ("%",   arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::rem>        { static constexpr impl data { create_type<ir_opcode::rem, ir_opcode::arithmetic> ("rem", arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::neg>        { static constexpr impl data { create_type<ir_opcode::neg, ir_opcode::arithmetic> ("-",   arity_class::unary) }; };

  template <> struct ir_instruction_metadata::instance<ir_opcode::logical> { static constexpr impl data { create_type<ir_opcode::logical> ("logical", true, true) }; };      // abstract
  template <> struct ir_instruction_metadata::instance<ir_opcode::land>    { static constexpr impl data { create_type<ir_opcode::land, ir_opcode::logical> ("&&", arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::lor>     { static constexpr impl data { create_type<ir_opcode::lor,  ir_opcode::logical> ("||", arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::lnot>    { static constexpr impl data { create_type<ir_opcode::lnot, ir_opcode::logical> ("!",  arity_class::unary) }; };

  template <> struct ir_instruction_metadata::instance<ir_opcode::bitwise>  { static constexpr impl data { create_type<ir_opcode::bitwise> ("bitwise", true, true) }; };      // abstract
  template <> struct ir_instruction_metadata::instance<ir_opcode::band>     { static constexpr impl data { create_type<ir_opcode::band,     ir_opcode::bitwise> ("&",  arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::bor>      { static constexpr impl data { create_type<ir_opcode::bor,      ir_opcode::bitwise> ("|",  arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::bxor>     { static constexpr impl data { create_type<ir_opcode::bxor,     ir_opcode::bitwise> ("^",  arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::bshiftl>  { static constexpr impl data { create_type<ir_opcode::bshiftl,  ir_opcode::bitwise> ("<<", arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::bashiftr> { static constexpr impl data { create_type<ir_opcode::bashiftr, ir_opcode::bitwise> (">>", arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::blshiftr> { static constexpr impl data { create_type<ir_opcode::blshiftr, ir_opcode::bitwise> (">>", arity_class::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::bnot>     { static constexpr impl data { create_type<ir_opcode::bnot,     ir_opcode::bitwise> ("~",  arity_class::unary) }; };

  class ir_instruction
  {
  public:
    using metadata = ir_instruction_metadata;

    template <ir_opcode Op>
    static constexpr metadata metadata_v = ir_instruction_metadata_v<Op>;

    using args_container_type     = std::vector<ir_operand>;
    using iterator                = typename args_container_type::iterator;
    using const_iterator          = typename args_container_type::const_iterator;
    using reverse_iterator        = typename args_container_type::reverse_iterator;
    using const_reverse_iterator  = typename args_container_type::const_reverse_iterator;
    using reference               = typename args_container_type::reference;
    using const_reference         = typename args_container_type::const_reference;
    using size_type               = typename args_container_type::size_type;
    using difference_type         = typename args_container_type::difference_type;
    using value_type              = typename args_container_type::value_type;
    using allocator_type          = typename args_container_type::allocator_type;

    using iter  = iterator;
    using citer = const_iterator;
    using ref   = reference;
    using cref  = const_reference;

    using optional_def_type = std::optional<ir_def>;

    ir_instruction            (void)                      = delete;
    ir_instruction            (const ir_instruction&)     = delete;
//  ir_instruction            (ir_instruction&&) noexcept = impl;
    ir_instruction& operator= (const ir_instruction&)     = delete;
//  ir_instruction& operator= (ir_instruction&&) noexcept = impl;
    ~ir_instruction           (void)                      = default;

    template <ir_opcode Op>
    static constexpr void assert_not_abstract (void)
    {
      static_assert (! metadata_v<Op>.is_abstract (),
                     "Cannot instantiate abstract instruction");
    }

    template <ir_opcode Op>
    static constexpr void assert_has_return (void)
    {
      static_assert (metadata_v<Op>.has_return (),
                     "Instruction metadata specified no such return.");
    }

    template <ir_opcode Op>
    static constexpr void assert_no_return (void)
    {
      static_assert (! metadata_v<Op>.has_return (),
                     "Instruction metadata specified a return.");
    }

    template <ir_opcode Op, std::size_t N>
    static constexpr void assert_valid_num_args (void)
    {
      static_assert (metadata_v<Op>.valid_num_args (N),
                     "Instruction metadata specified a different number of arguments.");
    }

    template <ir_opcode Op>
    static constexpr void assert_no_args (void)
    {
      static_assert (metadata_v<Op>.valid_num_args (0),
                     "Instruction metadata specified at least one argument");
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction (ir_instruction_type_t<Op>, ir_variable& ret_var, Args&&... args)
      : m_metadata (metadata_v<Op>),
        m_return   (std::in_place, ret_var, *this),
        m_args     ({ create_operand (std::forward<Args> (args))... })
    {
      assert_not_abstract<Op> ();
      assert_has_return<Op> ();
      assert_valid_num_args<Op, 1 + sizeof...(Args)> ();
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction (ir_instruction_type_t<Op>, ir_operand_pre& op, Args&&... args)
      : m_metadata (metadata_v<Op>),
        m_return   (std::nullopt),
        m_args     { create_operand (op),
                     create_operand (std::forward<Args> (args))... }
    {
      assert_not_abstract<Op> ();
      assert_no_return<Op> ();
      assert_valid_num_args<Op, 1 + sizeof...(Args)> ();
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction (ir_instruction_type_t<Op>, ir_constant&& c, Args&&... args)
      : m_metadata (metadata_v<Op>),
        m_return   (std::nullopt),
        m_args     { create_operand (std::move (c)),
                     create_operand (std::forward<Args> (args))... }
    {
      assert_not_abstract<Op> ();
      assert_no_return<Op> ();
      assert_valid_num_args<Op, 1 + sizeof...(Args)> ();
    }

    template <ir_opcode Op>
    explicit
    ir_instruction (ir_instruction_type_t<Op>)
      : m_metadata (metadata_v<Op>),
        m_return   (std::nullopt),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_no_return<Op> ();
      assert_no_args<Op> ();
    }

    template <ir_opcode Op, std::size_t N>
    explicit
    ir_instruction (ir_instruction_type_t<Op>, ir_variable& ret_var,
                    std::array<ir_operand_pre, N>& args)
      : m_metadata (metadata_v<Op>),
        m_return   (std::in_place, ret_var, *this),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_has_return<Op> ();
      assert_valid_num_args<Op, N> ();

      std::transform (std::move_iterator { args.begin () },
                      std::move_iterator { args.end () },
                      std::back_inserter (m_args),
                      [this](ir_operand_pre&& op)
                      {
                        return op.construct (*this);
                      });
    }

    template <ir_opcode Op, std::size_t N>
    explicit
    ir_instruction (ir_instruction_type_t<Op>, std::array<ir_operand_pre, N>& args)
      : m_metadata (metadata_v<Op>),
        m_return   (std::nullopt),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_no_return<Op> ();
      assert_valid_num_args<Op, N> ();

      std::transform (std::move_iterator { args.begin () },
                      std::move_iterator { args.end () },
                      std::back_inserter (m_args),
                      [this](ir_operand_pre&& op)
                      {
                        return op.construct (*this);
                      });
    }

    ir_instruction (ir_instruction&& other) noexcept
      : m_metadata (other.m_metadata),
        m_return   (other.m_return ? optional_def_type (std::in_place,
                                                        std::move (*other.m_return), *this)
                                   : std::nullopt),
        m_args     (std::move (other.m_args))
    {
      std::for_each (m_args.begin (), m_args.end (),
                     [this](ir_operand& arg)
                     {
                       if (optional_ref<ir_use> u = get_if<ir_use> (arg))
                         u->set_instruction (*this);
                     });
    }

    ir_instruction& operator= (ir_instruction&& other) noexcept
    {
      m_metadata = other.m_metadata;
      set_return (std::move (other.m_return));
      set_args (std::move (other.m_args));
      return *this;
    }

    template <ir_opcode Op, typename ...Args>
    static ir_instruction create (ir_variable& ret_var, Args&&... args)
    {
      assert_not_abstract<Op> ();
      assert_has_return<Op> ();
      assert_valid_num_args<Op, 1 + sizeof...(Args)> ();
      return { metadata_v<Op>, ret_var, std::forward<Args> (args)... };
    }

    template <ir_opcode Op, typename T, typename ...Args>
    static ir_instruction create (ir_operand_pre& op, Args&&... args)
    {
      assert_not_abstract<Op> ();
      assert_no_return<Op> ();
      assert_valid_num_args<Op, 1 + sizeof...(Args)> ();
      return { metadata_v<Op>, std::nullopt, op, std::forward<Args> (args)... };
    }

    template <ir_opcode Op, typename T, typename ...Args>
    static ir_instruction create (ir_constant&& c, Args&&... args)
    {
      assert_not_abstract<Op> ();
      assert_no_return<Op> ();
      assert_valid_num_args<Op, 1 + sizeof...(Args)> ();
      return { metadata_v<Op>, std::nullopt, std::move (c), std::forward<Args> (args)... };
    }

    template <ir_opcode Op>
    static ir_instruction create (void)
    {
      assert_not_abstract<Op> ();
      assert_no_return<Op> ();
      assert_no_args<Op> ();
      return { metadata_v<Op>, std::nullopt };
    }

    template <ir_opcode Op, std::size_t N>
    static ir_instruction create (ir_variable& ret_var,
                                  const std::array<ir_operand_pre, N>& args)
    {
      assert_not_abstract<Op> ();
      assert_has_return<Op> ();
      assert_valid_num_args<Op, N> ();
      return { metadata_v<Op>, ret_var, args };
    }

    template <ir_opcode Op, std::size_t N>
    static ir_instruction create (const std::array<ir_operand_pre, N>& args)
    {
      assert_not_abstract<Op> ();
      assert_no_return<Op> ();
      assert_valid_num_args<Op, N> ();
      return { metadata_v<Op>, std::nullopt, args };
    }

    [[nodiscard]] auto  begin   (void)       noexcept { return m_args.begin ();   }
    [[nodiscard]] auto  begin   (void) const noexcept { return m_args.begin ();   }
    [[nodiscard]] auto  cbegin  (void) const noexcept { return m_args.cbegin ();  }

    [[nodiscard]] auto  end     (void)       noexcept { return m_args.end ();     }
    [[nodiscard]] auto  end     (void) const noexcept { return m_args.end ();     }
    [[nodiscard]] auto  cend    (void) const noexcept { return m_args.cend ();    }

    [[nodiscard]] auto  rbegin  (void)       noexcept { return m_args.rbegin ();  }
    [[nodiscard]] auto  rbegin  (void) const noexcept { return m_args.rbegin ();  }
    [[nodiscard]] auto  crbegin (void) const noexcept { return m_args.crbegin (); }

    [[nodiscard]] auto  rend    (void)       noexcept { return m_args.rend ();    }
    [[nodiscard]] auto  rend    (void) const noexcept { return m_args.rend ();    }
    [[nodiscard]] auto  crend   (void) const noexcept { return m_args.crend ();   }

    [[nodiscard]] auto& front   (void)       noexcept { return m_args.front ();   }
    [[nodiscard]] auto& front   (void) const noexcept { return m_args.front ();   }

    [[nodiscard]] auto& back    (void)       noexcept { return m_args.back ();    }
    [[nodiscard]] auto& back    (void) const noexcept { return m_args.back ();    }

    iter erase (citer pos) { return m_args.erase (pos); };
    iter erase (citer first, citer last) { return m_args.erase (first, last); };

    void set_return (optional_def_type&& ret)
    {
      if ((m_return = std::move (ret)))
        m_return->set_instruction (*this);
    }

    void set_args (args_container_type&& args)
    {
      m_args = std::move (args);
      std::for_each (m_args.begin (), m_args.end (),
                     [this](ir_operand& arg)
                     {
                       if (optional_ref<ir_use> u = get_if<ir_use> (arg))
                         u->set_instruction (*this);
                     });
    }

    template <typename ...Args>
    ir_operand& emplace_back (Args&&... args)
    {
      return m_args.emplace_back (std::forward<Args> (args)...);
    }

    template <typename ...Args>
    void emplace_before (citer pos, Args&&... args)
    {
      return m_args.emplace (pos, std::forward<Args> (args)...);
    }

    [[nodiscard]] constexpr metadata get_metadata (void) const noexcept { return m_metadata; }

    [[nodiscard]] constexpr       ir_def& get_return (void)       noexcept { return *m_return; }
    [[nodiscard]] constexpr const ir_def& get_return (void) const noexcept { return *m_return; }

  private:

    template <typename ...Args>
    ir_instruction (metadata m, optional_def_type&& def, Args&&... args)
      : m_metadata (m),
        m_return   (std::move (def)),
        m_args     ({ create_operand (std::forward<Args> (args))... })
    { }

    template <std::size_t N>
    ir_instruction (metadata m, optional_def_type&& def,
                    const std::array<ir_operand_pre, N>& args)
      : m_metadata (m),
        m_return   (std::move (def)),
        m_args     ()
    {
      std::transform (std::move_iterator { args.begin () },
                      std::move_iterator { args.end () },
                      std::back_inserter (m_args),
                      [this](ir_operand_pre&& op)
                      {
                        return op.construct (*this);
                      });
    }

    template <typename ...Ts>
    ir_instruction (metadata m, ir_variable& ret_var, Ts&&... args)
      : ir_instruction (m, optional_def_type (std::in_place, ret_var, *this),
                        std::forward<Ts> (args)...)
    { }

    ir_operand create_operand (ir_operand_pre& op)
    {
      return op.construct (*this);
    }

    ir_operand create_operand (ir_operand_pre&& op)
    {
      return op.construct (*this);
    }

    metadata            m_metadata;
    optional_def_type   m_return;
    args_container_type m_args;
  };

  template <ir_opcode Op>
  constexpr
  bool
  is_a (const ir_instruction& instr) noexcept
  {
    return instr.get_metadata ().is_a<Op> ();
  }

  constexpr
  bool
  has_return (const ir_instruction& instr) noexcept
  {
    return instr.get_metadata ().has_return ();
  }

  // class ir_phi : public ir_def_instruction
  // {
  // public:
  //
  //   ir_phi            (void)              = delete;
  //   ir_phi            (const ir_phi&)     = delete;
  //   ir_phi            (ir_phi&&) noexcept = delete;
  //   ir_phi& operator= (const ir_phi&)     = delete;
  //   ir_phi& operator= (ir_phi&&) noexcept = delete;
  //   ~ir_phi           (void) override     = default;
  //
  //   ir_phi (ir_basic_block& blk, ir_variable& var)
  //     : ir_def_instruction (blk, var)
  //   { }
  //
  //   template <typename It>
  //   ir_phi (ir_basic_block& blk, ir_variable& var, ir_type ty)
  //     : ir_def_instruction (blk, var, ty)
  //   { }
  //
  //   void append_incoming (ir_basic_block& blk, ir_use_timeline& ut)
  //   {
  //     emplace_back (ir_type_v<ir_basic_block *>, &blk);
  //     emplace_back (ut, *this);
  //   }
  //
  //   void append_indet (ir_basic_block& blk)
  //   {
  //     m_indets.emplace_back (blk);
  //   }
  //
  //   void set_indets (std::vector<nonnull_ptr<ir_basic_block>>&& indets)
  //   {
  //     m_indets = std::move (indets);
  //   }
  //
  //   iter erase (const ir_basic_block& blk);
  //
  //   [[nodiscard]]  iter find (const ir_basic_block& blk);
  //   [[nodiscard]] citer find (const ir_basic_block& blk) const;
  //
  //   optional_ref<ir_use> retrieve_use (const ir_basic_block& blk);
  //   optional_ref<ir_def> retrieve_def (const ir_basic_block& blk);
  //
  //   [[nodiscard]] auto  indet_begin   (void)       noexcept { return m_indets.begin ();   }
  //   [[nodiscard]] auto  indet_begin   (void) const noexcept { return m_indets.begin ();   }
  //   [[nodiscard]] auto  indet_cbegin  (void) const noexcept { return m_indets.cbegin ();  }
  //
  //   [[nodiscard]] auto  indet_end     (void)       noexcept { return m_indets.end ();     }
  //   [[nodiscard]] auto  indet_end     (void) const noexcept { return m_indets.end ();     }
  //   [[nodiscard]] auto  indet_cend    (void) const noexcept { return m_indets.cend ();    }
  //
  //   [[nodiscard]] auto  indet_rbegin  (void)       noexcept { return m_indets.rbegin ();  }
  //   [[nodiscard]] auto  indet_rbegin  (void) const noexcept { return m_indets.rbegin ();  }
  //   [[nodiscard]] auto  indet_crbegin (void) const noexcept { return m_indets.crbegin (); }
  //
  //   [[nodiscard]] auto  indet_rend    (void)       noexcept { return m_indets.rend ();    }
  //   [[nodiscard]] auto  indet_rend    (void) const noexcept { return m_indets.rend ();    }
  //   [[nodiscard]] auto  indet_crend   (void) const noexcept { return m_indets.crend ();   }
  //
  //   [[nodiscard]] auto& indet_front   (void)       noexcept { return m_indets.front ();   }
  //   [[nodiscard]] auto& indet_front   (void) const noexcept { return m_indets.front ();   }
  //
  //   [[nodiscard]] auto& indet_back    (void)       noexcept { return m_indets.back ();    }
  //   [[nodiscard]] auto& indet_back    (void) const noexcept { return m_indets.back ();    }
  //
  //   [[nodiscard]] std::size_t num_indet (void) const noexcept { return m_indets.size (); }
  //
  //   [[nodiscard]] bool has_indet (void) const noexcept { return m_indets.empty (); }
  //
  // private:
  //
  //   // blocks where the variable was undefined
  //   std::vector<nonnull_ptr<ir_basic_block>> m_indets;
  // };

}

#endif

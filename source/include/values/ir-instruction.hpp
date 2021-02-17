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

#include "utilities/ir-common.hpp"
#include "ir-variable.hpp"
#include "values/types/ir-type-ir.hpp"
#include "ir-instruction-fwd.hpp"
#include <unordered_map>
#include <list>
#include <vector>

#include <memory>
#include <utility>

namespace gch
{

  enum class ir_opcode : unsigned
  {
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
    enum class arity : signed
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
      const char *    m_name;
      const impl *    m_base;
      const ir_opcode m_opcode;
      const arity     m_arity;
      const bool      m_has_return;
      const bool      m_is_abstract;
    };

    static_assert (std::is_standard_layout_v<const char*> && std::is_trivial_v<const char*>);
    static_assert (std::is_standard_layout_v<const impl*> && std::is_trivial_v<const impl*>);
    static_assert (std::is_standard_layout_v<ir_opcode> && std::is_trivial_v<ir_opcode>);
    static_assert (std::is_standard_layout_v<arity> && std::is_trivial_v<arity>);
    static_assert (std::is_standard_layout_v<bool> && std::is_trivial_v<bool>);
    static_assert (std::is_standard_layout_v<impl>
               &&  std::is_trivially_constructible_v<impl, impl&>);

    template <ir_opcode Op>
    struct instance;

    constexpr
    ir_instruction_metadata (const impl& impl_ref) noexcept
      : m_ptr (impl_ref)
    { }

  public:
    ir_instruction_metadata            (void)                = delete;
    ir_instruction_metadata            (const ir_instruction_metadata&)     = default;
    ir_instruction_metadata            (ir_instruction_metadata&&) noexcept = default;
    ir_instruction_metadata& operator= (const ir_instruction_metadata&)     = default;
    ir_instruction_metadata& operator= (ir_instruction_metadata&&) noexcept = default;
    ~ir_instruction_metadata           (void)                = default;

    [[nodiscard]] constexpr
    ir_opcode
    get_opcode (void) const noexcept
    {
      return  m_ptr->m_opcode;
    }

    [[nodiscard]] constexpr
    const char *
    get_name (void) const noexcept
    {
      return  m_ptr->m_name;
    }

    [[nodiscard]] constexpr
    bool
    is_abstract (void) const noexcept
    {
      return  m_ptr->m_is_abstract;
    }

    [[nodiscard]] constexpr
    ir_instruction_metadata
    get_base (void) const noexcept
    {
      return *m_ptr->m_base;
    }

    [[nodiscard]] constexpr
    bool
    will_return (void) const noexcept
    {
      return  m_ptr->m_has_return;
    }

    [[nodiscard]] constexpr
    arity
    get_arity (void) const noexcept
    {
      return  m_ptr->m_arity;
    }

    [[nodiscard]] constexpr
    bool
    has_base (void) const noexcept
    {
      return m_ptr->m_base != nullptr;
    }

    [[nodiscard]] constexpr
    bool
    is_n_ary (void) const noexcept
    {
      return get_arity () == arity::n_ary;
    }

    [[nodiscard]] constexpr
    bool
    is_nullary (void) const noexcept
    {
      return get_arity () == arity::nullary;
    }

    [[nodiscard]] constexpr
    bool
    is_unary (void) const noexcept
    {
      return get_arity () == arity::unary;
    }

    [[nodiscard]] constexpr
    bool
    is_binary (void) const noexcept
    {
      return get_arity () == arity::binary;
    }

    [[nodiscard]] constexpr
    bool
    is_ternary (void) const noexcept
    {
      return get_arity () == arity::ternary;
    }

    friend constexpr
    bool
    operator== (const ir_instruction_metadata& lhs, const ir_instruction_metadata& rhs) noexcept;

    [[nodiscard]] constexpr
    bool
    is_a (ir_instruction_metadata cmp) const noexcept
    {
      return (cmp == *this) || (has_base () && get_base ().is_a (cmp));
    }

    [[nodiscard]] constexpr
    bool
    is_base_of (ir_instruction_metadata cmp) const noexcept
    {
      return cmp.is_a (*this);
    }

    template <ir_opcode Op>
    [[nodiscard]]
    static constexpr
    ir_instruction_metadata
    get (void) noexcept
    {
      return instance<Op>::data;
    }

  private:
    [[nodiscard]]
    static constexpr
    impl
    create_type (ir_opcode Op, const char* name, bool will_return, arity n = arity::n_ary,
                 bool is_abstract = false) noexcept
    {
      return { name, nullptr, Op, n, will_return, is_abstract };
    }

    [[nodiscard]]
    static constexpr
    impl
    create_type (ir_opcode Op, const char* name, bool will_return,
                 bool is_abstract = false) noexcept
    {
      return { name, nullptr, Op, arity::n_ary, will_return, is_abstract };
    }

    [[nodiscard]]
    static constexpr
    impl
    create_type (ir_opcode Op, const char* name, arity n = arity::n_ary,
                 bool is_abstract = false) noexcept
    {
      return { name, nullptr, Op, n, false, is_abstract };
    }

    [[nodiscard]] constexpr
    impl
    derive (ir_opcode Op, const char* name, bool will_return,
            bool is_abstract = false) const noexcept
    {
      return { name, m_ptr, Op, get_arity (), will_return, is_abstract };
    }

    [[nodiscard]] constexpr
    impl
    derive (ir_opcode Op, const char* name, bool will_return, arity n,
            bool is_abstract = false) const noexcept
    {
      return { name, m_ptr, Op, n, will_return, is_abstract };
    }

    [[nodiscard]] constexpr
    impl
    derive (ir_opcode Op, const char* name, arity n, bool is_abstract = false) const noexcept
    {
      return { name, m_ptr, Op, n, will_return (), is_abstract };
    }

    [[nodiscard]] constexpr
    impl
    derive (ir_opcode Op, const char* name, bool is_abstract = false) const noexcept
    {
      return { name, m_ptr, Op, get_arity (), will_return (), is_abstract };
    }

    nonnull_ptr<const impl> m_ptr;
  };

  [[nodiscard]] constexpr
  bool
  operator== (const ir_instruction_metadata& lhs, const ir_instruction_metadata& rhs) noexcept
  {
    return lhs.m_ptr == rhs.m_ptr;
  }

  [[nodiscard]] constexpr
  bool
  operator!= (const ir_instruction_metadata& lhs, const ir_instruction_metadata& rhs) noexcept
  {
    return ! (lhs == rhs);
  }

  template <> struct ir_instruction_metadata::instance<ir_opcode::phi>     { static constexpr impl data { create_type (ir_opcode::phi,     "phi",     true, arity::n_ary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::assign>  { static constexpr impl data { create_type (ir_opcode::assign,  "assign",  true, arity::unary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::call>    { static constexpr impl data { create_type (ir_opcode::call,    "call",    true, arity::n_ary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::fetch>   { static constexpr impl data { create_type (ir_opcode::fetch,   "fetch",   true, arity::unary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::convert> { static constexpr impl data { create_type (ir_opcode::convert, "convert", true, arity::unary) }; };

  template <> struct ir_instruction_metadata::instance<ir_opcode::branch>   { static constexpr impl data { create_type (ir_opcode::branch, "branch", false, arity::n_ary) }; };     // abstract
  template <> struct ir_instruction_metadata::instance<ir_opcode::cbranch>  { static constexpr impl data { get<ir_opcode::branch> ().derive (ir_opcode::cbranch,  "br",  arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::ucbranch> { static constexpr impl data { get<ir_opcode::branch> ().derive (ir_opcode::ucbranch, "ubr", arity::unary)  }; };

  template <> struct ir_instruction_metadata::instance<ir_opcode::relation> { static constexpr impl data { create_type (ir_opcode::relation, "relation", true, arity::binary, true) }; };     // abstract
  template <> struct ir_instruction_metadata::instance<ir_opcode::eq>       { static constexpr impl data { get<ir_opcode::relation> ().derive (ir_opcode::eq, "==") }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::ne>       { static constexpr impl data { get<ir_opcode::relation> ().derive (ir_opcode::ne, "!=") }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::lt>       { static constexpr impl data { get<ir_opcode::relation> ().derive (ir_opcode::lt, "<" ) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::le>       { static constexpr impl data { get<ir_opcode::relation> ().derive (ir_opcode::le, "<=") }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::gt>       { static constexpr impl data { get<ir_opcode::relation> ().derive (ir_opcode::gt, ">" ) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::ge>       { static constexpr impl data { get<ir_opcode::relation> ().derive (ir_opcode::ge, ">=") }; };

  template <> struct ir_instruction_metadata::instance<ir_opcode::arithmetic> { static constexpr impl data { create_type (ir_opcode::arithmetic, "arithmetic", true, true) }; };   // abstract
  template <> struct ir_instruction_metadata::instance<ir_opcode::add>        { static constexpr impl data { get<ir_opcode::arithmetic> ().derive (ir_opcode::add, "+",   arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::sub>        { static constexpr impl data { get<ir_opcode::arithmetic> ().derive (ir_opcode::sub, "-",   arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::mul>        { static constexpr impl data { get<ir_opcode::arithmetic> ().derive (ir_opcode::mul, "*",   arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::div>        { static constexpr impl data { get<ir_opcode::arithmetic> ().derive (ir_opcode::div, "/",   arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::mod>        { static constexpr impl data { get<ir_opcode::arithmetic> ().derive (ir_opcode::mod, "%",   arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::rem>        { static constexpr impl data { get<ir_opcode::arithmetic> ().derive (ir_opcode::rem, "rem", arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::neg>        { static constexpr impl data { get<ir_opcode::arithmetic> ().derive (ir_opcode::neg, "-",   arity::unary ) }; };

  template <> struct ir_instruction_metadata::instance<ir_opcode::logical> { static constexpr impl data { create_type (ir_opcode::logical, "logical", true, true) }; };      // abstract
  template <> struct ir_instruction_metadata::instance<ir_opcode::land>    { static constexpr impl data { get<ir_opcode::logical> ().derive (ir_opcode::land, "&&", arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::lor>     { static constexpr impl data { get<ir_opcode::logical> ().derive (ir_opcode::lor,  "||", arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::lnot>    { static constexpr impl data { get<ir_opcode::logical> ().derive (ir_opcode::lnot, "!",  arity::unary ) }; };

  template <> struct ir_instruction_metadata::instance<ir_opcode::bitwise>  { static constexpr impl data { create_type (ir_opcode::bitwise, "bitwise", true, true) }; };      // abstract
  template <> struct ir_instruction_metadata::instance<ir_opcode::band>     { static constexpr impl data { get<ir_opcode::bitwise> ().derive (ir_opcode::band,     "&",  arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::bor>      { static constexpr impl data { get<ir_opcode::bitwise> ().derive (ir_opcode::bor,      "|",  arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::bxor>     { static constexpr impl data { get<ir_opcode::bitwise> ().derive (ir_opcode::bxor,     "^",  arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::bshiftl>  { static constexpr impl data { get<ir_opcode::bitwise> ().derive (ir_opcode::bshiftl,  "<<", arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::bashiftr> { static constexpr impl data { get<ir_opcode::bitwise> ().derive (ir_opcode::bashiftr, ">>", arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::blshiftr> { static constexpr impl data { get<ir_opcode::bitwise> ().derive (ir_opcode::blshiftr, ">>", arity::binary) }; };
  template <> struct ir_instruction_metadata::instance<ir_opcode::bnot>     { static constexpr impl data { get<ir_opcode::bitwise> ().derive (ir_opcode::bnot,     "~",  arity::unary ) }; };

  template <ir_opcode Op>
  inline constexpr
  ir_instruction_metadata ir_instruction_metadata_v { ir_instruction_metadata::get<Op> () };

  template <ir_opcode Op>
  struct ir_instruction_traits
  {
    explicit
    ir_instruction_traits (void) = default;

    static constexpr auto metadata    = ir_instruction_metadata_v<Op>;
    static constexpr auto opcode      = metadata.get_opcode ();
    static constexpr auto name        = metadata.get_name ();
    static constexpr auto is_abstract = metadata.is_abstract ();
    static constexpr auto has_base    = metadata.has_base ();
    static constexpr auto will_return = metadata.will_return ();
    static constexpr auto arity       = metadata.get_arity ();

    static constexpr auto is_n_ary    = metadata.is_n_ary ();
    static constexpr auto is_nullary  = metadata.is_nullary ();
    static constexpr auto is_unary    = metadata.is_unary ();
    static constexpr auto is_binary   = metadata.is_binary ();
    static constexpr auto is_ternary  = metadata.is_ternary ();

    template <bool HasBase = has_base, std::enable_if_t<HasBase> * = nullptr>
    static constexpr
    auto
    base = metadata.get_base ();

    template <ir_opcode BaseOp>
    static constexpr
    bool
    is_a = metadata.is_a (ir_instruction_metadata_v<BaseOp>);

    template <ir_opcode OtherOp>
    static constexpr
    bool
    is_base_of = metadata.is_base_of (ir_instruction_metadata_v<OtherOp>);

    template <std::size_t N>
    static constexpr
    bool
    is_valid_num_args = is_n_ary || (static_cast<std::size_t> (arity) == N);
  };

  static_assert (ir_instruction_traits<ir_opcode::band>::is_a<ir_opcode::bitwise>);

  class ir_instruction
  {
    template <ir_opcode Op>
    using type = ir_instruction_traits<Op>;

  public:
    template <ir_opcode Op>
    using tag_t = type<Op>;

    template <ir_opcode Op>
    static constexpr tag_t<Op> tag { };

    using metadata_t = ir_instruction_metadata;

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

    ir_instruction            (void)                      = delete;
    ir_instruction            (const ir_instruction&)     = delete;
    ir_instruction            (ir_instruction&&) noexcept;
    ir_instruction& operator= (const ir_instruction&)     = delete;
//  ir_instruction& operator= (ir_instruction&&) noexcept = impl;
    ~ir_instruction           (void)                      = default;

    template <ir_opcode Op>
    static constexpr
    void
    assert_not_abstract (void)
    {
      static_assert (! type<Op>::is_abstract,
                     "Cannot instantiate abstract instruction");
    }

    template <ir_opcode Op>
    static constexpr
    void
    assert_has_return (void)
    {
      static_assert (type<Op>::will_return,
                     "Instruction metadata specified no such return.");
    }

    template <ir_opcode Op>
    static constexpr
    void
    assert_no_return (void)
    {
      static_assert (! type<Op>::will_return,
                     "Instruction metadata specified a return.");
    }

    template <ir_opcode Op, std::size_t N>
    static constexpr
    void
    assert_is_valid_num_args (void)
    {
      static_assert (type<Op>::template is_valid_num_args<N>,
                     "Instruction metadata specified a different number of arguments.");
    }

    template <ir_opcode Op>
    static constexpr
    void
    assert_no_args (void)
    {
      static_assert (type<Op>::template is_valid_num_args<0>,
                     "Instruction metadata specified at least one argument");
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction (tag_t<Op>, ir_variable& ret_var, Args&&... args)
      : m_metadata (type<Op>::metadata),
        m_return   (std::in_place, ret_var, *this),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_has_return<Op> ();

      initialize_with_pack<Op> (std::forward<Args> (args)...);
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction (tag_t<Op>, ir_operand_in&& op, Args&&... args)
      : m_metadata (type<Op>::metadata),
        m_return   (std::nullopt),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_no_return<Op> ();

      initialize_with_pack<Op> (std::move (op), std::forward<Args> (args)...);
    }

    template <ir_opcode Op, typename ...Args>
    ir_instruction (tag_t<Op>, ir_constant&& c, Args&&... args)
      : m_metadata (type<Op>::metadata),
        m_return   (std::nullopt),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_no_return<Op> ();

      initialize_with_pack<Op> (std::move (c), std::forward<Args> (args)...);
    }



    template <ir_opcode Op, std::size_t N>
    explicit
    ir_instruction (tag_t<Op>, ir_variable& ret_var, std::array<ir_operand_in, N>&& args)
      : m_metadata (type<Op>::metadata),
        m_return   (std::in_place, ret_var, *this),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_has_return<Op> ();

      initialize_with_array<Op> (std::move (args));
    }

    template <ir_opcode Op, std::size_t N>
    explicit
    ir_instruction (tag_t<Op>, std::array<ir_operand_in, N>&& args)
      : m_metadata (type<Op>::metadata),
        m_return   (std::nullopt),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_no_return<Op> ();

      initialize_with_array<Op> (std::move (args));
    }

    template <ir_opcode Op>
    explicit
    ir_instruction (tag_t<Op>)
      : m_metadata (type<Op>::metadata),
        m_return   (std::nullopt),
        m_args     ()
    {
      assert_not_abstract<Op> ();
      assert_no_return<Op> ();
      assert_no_args<Op> ();
    }

    ir_instruction&
    operator= (ir_instruction&& other) noexcept
    {
      m_metadata = other.m_metadata;
      set_return (std::move (other.m_return));
      set_args (std::move (other.m_args));
      return *this;
    }

    template <ir_opcode Op, typename ...Args>
    void
    initialize_with_pack (Args&&... args)
    {
      assert_is_valid_num_args<Op, sizeof... (Args)> ();

      m_args.reserve (sizeof... (Args));
      (m_args.emplace_back (*this, std::forward<Args> (args)), ...);
    }

    template <ir_opcode Op, std::size_t N>
    void
    initialize_with_array (std::array<ir_operand_in, N>&& args)
    {
      assert_is_valid_num_args<Op, N> ();

      m_args.reserve (N);
      std::for_each (std::move_iterator { args.begin () },
                     std::move_iterator { args.end () },
                     [this](ir_operand_in&& op)
                     {
                       emplace_back (std::move (op));
                     });
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

    // distinguish between expected num_args from arity
    [[nodiscard]]
    auto
    current_num_args (void) const noexcept
    {
      return m_args.size ();
    }

    // instead of empty () to distinguish between expected empty instruction and current state
    [[nodiscard]]
    bool
    is_currently_empty (void) const noexcept
    {
      return m_args.empty ();
    }

    iter
    erase (citer pos)
    {
      return m_args.erase (pos);
    };

    iter
    erase (citer first, citer last)
    {
      return m_args.erase (first, last);
    };

    void
    set_return (std::optional<ir_def>&& ret);

    void
    set_args (args_container_type&& args);

    template <typename ...Args>
    ir_operand&
    emplace_back (Args&&... args)
    {
      return m_args.emplace_back (*this, std::forward<Args> (args)...);
    }

    template <typename ...Args>
    iter
    emplace_before (citer pos, Args&&... args)
    {
      return m_args.emplace (pos, *this, std::forward<Args> (args)...);
    }

    [[nodiscard]] constexpr
    metadata_t
    get_metadata (void) const noexcept
    {
      return m_metadata;
    }

    [[nodiscard]] constexpr
    ir_def&
    get_return (void) noexcept
    {
      return *m_return;
    }
    [[nodiscard]] constexpr
    const ir_def&
    get_return (void) const noexcept
    {
      return *m_return;
    }

  private:
    metadata_t            m_metadata;
    std::optional<ir_def> m_return;
    args_container_type   m_args;
  };

  template <ir_opcode BaseOp>
  [[nodiscard]] constexpr
  bool
  is_a (const ir_instruction& instr) noexcept
  {
    return instr.get_metadata ().is_a (ir_instruction_metadata_v<BaseOp>);
  }

  [[nodiscard]] constexpr
  bool
  will_return (const ir_instruction& instr) noexcept
  {
    return instr.get_metadata ().will_return ();
  }

}

#endif

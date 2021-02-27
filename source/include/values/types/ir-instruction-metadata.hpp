/** ir-instruction-metadata.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_INSTRUCTION_METADATA_HPP
#define OCTAVE_IR_IR_INSTRUCTION_METADATA_HPP

#ifndef GCH_CPP20_CONSTEVAL
#  if defined (__cpp_consteval) && __cpp_consteval >= 201811L
#    define GCH_CPP20_CONSTEVAL consteval
#  else
#    define GCH_CPP20_CONSTEVAL constexpr
#  endif
#endif

#include "utilities/ir-functional.hpp"

#include <array>

namespace gch
{

  enum class ir_opcode : unsigned
  {
    _size_     =  35,

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

    struct flag
    {
      enum class arity : signed
      {
        n_ary    = -1,
        nullary  =  0,
        unary    =  1,
        binary   =  2,
        ternary  =  3,
      };

      enum class has_def : bool
      {
        yes = true,
        no  = false,
      };

      enum class is_abstract : bool
      {
        yes = true,
        no  = false,
      };
    };

  private:
    struct impl
    {
      const char *            m_name;
      const impl *            m_base;
      const ir_opcode         m_opcode;
      const flag::arity       m_arity;
      const flag::has_def     m_has_def;
      const flag::is_abstract m_abstract;
    };

    template <ir_opcode Op>
    struct instance;

    constexpr explicit
    ir_instruction_metadata (const impl& impl_ref) noexcept
      : m_ptr (&impl_ref)
    { }

  public:
    ir_instruction_metadata            (void)                               = delete;
    ir_instruction_metadata            (const ir_instruction_metadata&)     = default;
    ir_instruction_metadata            (ir_instruction_metadata&&) noexcept = default;
    ir_instruction_metadata& operator= (const ir_instruction_metadata&)     = default;
    ir_instruction_metadata& operator= (ir_instruction_metadata&&) noexcept = default;
    ~ir_instruction_metadata           (void)                               = default;

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    const char *
    get_name (void) const noexcept
    {
      return m_ptr->m_name;
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    ir_instruction_metadata
    get_base (void) const noexcept
    {
      return ir_instruction_metadata { *m_ptr->m_base };
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    ir_opcode
    get_opcode (void) const noexcept
    {
      return m_ptr->m_opcode;
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    flag::arity
    get_arity (void) const noexcept
    {
      return m_ptr->m_arity;
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    bool
    has_def (void) const noexcept
    {
      return m_ptr->m_has_def == flag::has_def::yes;
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    bool
    is_abstract (void) const noexcept
    {
      return m_ptr->m_abstract == flag::is_abstract::yes;
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    bool
    has_base (void) const noexcept
    {
      return m_ptr->m_base != nullptr;
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    bool
    is_n_ary (void) const noexcept
    {
      return get_arity () == flag::arity::n_ary;
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    bool
    is_nullary (void) const noexcept
    {
      return get_arity () == flag::arity::nullary;
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    bool
    is_unary (void) const noexcept
    {
      return get_arity () == flag::arity::unary;
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    bool
    is_binary (void) const noexcept
    {
      return get_arity () == flag::arity::binary;
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    bool
    is_ternary (void) const noexcept
    {
      return get_arity () == flag::arity::ternary;
    }

    friend GCH_CPP20_CONSTEVAL
    bool
    operator== (const ir_instruction_metadata& lhs, const ir_instruction_metadata& rhs) noexcept;

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    bool
    is_a (ir_instruction_metadata cmp) const noexcept
    {
      return (cmp == *this) || (has_base () && get_base ().is_a (cmp));
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    bool
    is_base_of (ir_instruction_metadata cmp) const noexcept
    {
      return cmp.is_a (*this);
    }

    template <ir_opcode Op>
    [[nodiscard]]
    static GCH_CPP20_CONSTEVAL
    ir_instruction_metadata
    get (void) noexcept
    {
      return ir_instruction_metadata { instance<Op>::data };
    }

    static constexpr
    std::size_t
    num_opcodes = static_cast<std::underlying_type_t<ir_opcode>> (ir_opcode::_size_);

  private:
    struct identity_projection
    {
      GCH_CPP20_CONSTEVAL
      ir_instruction_metadata
      operator() (ir_instruction_metadata m) const noexcept
      {
        return m;
      }
    };

    template <typename IndexSequence = std::make_index_sequence<num_opcodes>>
    struct array_generator;

  public:
    template <typename Projection = identity_projection>
    [[nodiscard]]
    static GCH_CPP20_CONSTEVAL
    std::array<std::invoke_result_t<Projection, ir_instruction_metadata>, num_opcodes>
    generate_array (Projection proj = { }) noexcept;

  private:
    [[nodiscard]]
    static GCH_CPP20_CONSTEVAL
    impl
    create_type (const char *      name,
                 ir_opcode         Op,
                 flag::has_def     def,
                 flag::arity       n,
                 flag::is_abstract abs) noexcept
    {
      return { name, nullptr, Op, n, def, abs };
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    impl
    derive (const char *      name,
            ir_opcode         Op,
            flag::has_def     def,
            flag::is_abstract abs = flag::is_abstract::no) const noexcept
    {
      return { name, m_ptr, Op, get_arity (), def, abs };
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    impl
    derive (const char *      name,
            ir_opcode         Op,
            flag::has_def     def,
            flag::arity       n,
            flag::is_abstract abs = flag::is_abstract::no) const noexcept
    {
      return { name, m_ptr, Op, n, def, abs };
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    impl
    derive (const char *      name,
            ir_opcode         Op,
            flag::arity       n,
            flag::is_abstract abs = flag::is_abstract::no) const noexcept
    {
      return { name, m_ptr, Op, n, m_ptr->m_has_def, abs };
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    impl
    derive (const char *      name,
            ir_opcode         Op,
            flag::is_abstract abs = flag::is_abstract::no) const noexcept
    {
      return { name, m_ptr, Op, get_arity (), m_ptr->m_has_def, abs };
    }

    const impl *m_ptr;
  };

  template <std::size_t ...Indices>
  struct ir_instruction_metadata::array_generator<std::index_sequence<Indices...>>
  {
    static_assert (sizeof...(Indices) == num_opcodes);

    template <typename Result>
    constexpr
    std::array<Result, num_opcodes>
    operator() (Result (ir_instruction_metadata::* proj) (void) const noexcept) const noexcept
    {
      return { (get<static_cast<ir_opcode> (Indices)> ().*proj) ()... };
    }

    template <typename Projection>
    constexpr
    std::array<std::invoke_result_t<Projection, ir_instruction_metadata>, num_opcodes>
    operator() (Projection proj) const noexcept
    {
      return { proj (get<static_cast<ir_opcode> (Indices)> ())... };
    }
  };

  template <typename Projection>
  GCH_CPP20_CONSTEVAL
  auto
  ir_instruction_metadata::
  generate_array (Projection proj) noexcept
    -> std::array<std::invoke_result_t<Projection, ir_instruction_metadata>, num_opcodes>
  {
    return array_generator<std::make_index_sequence<num_opcodes>> { } (proj);
  }

  [[nodiscard]] GCH_CPP20_CONSTEVAL
  bool
  operator== (const ir_instruction_metadata& lhs, const ir_instruction_metadata& rhs) noexcept
  {
    return lhs.m_ptr == rhs.m_ptr;
  }

  [[nodiscard]] GCH_CPP20_CONSTEVAL
  bool
  operator!= (const ir_instruction_metadata& lhs, const ir_instruction_metadata& rhs) noexcept
  {
    return ! (lhs == rhs);
  }

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::phi>
  {
    static constexpr
    impl
    data = create_type ("phi",
                        ir_opcode::        phi,
                        flag::has_def::    yes,
                        flag::arity::      n_ary,
                        flag::is_abstract::no);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::assign>
  {
    static constexpr
    impl
    data = create_type ("assign",
                        ir_opcode::        assign,
                        flag::has_def::    yes,
                        flag::arity::      unary,
                        flag::is_abstract::no);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::call>
  {
    static constexpr
    impl
    data = create_type ("call",
                        ir_opcode::        call,
                        flag::has_def::    yes,
                        flag::arity::      n_ary,
                        flag::is_abstract::no);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::fetch>
  {
    static constexpr
    impl
    data = create_type ("fetch",
                        ir_opcode::        fetch,
                        flag::has_def::    yes,
                        flag::arity::      unary,
                        flag::is_abstract::no);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::convert>
  {
    static constexpr
    impl
    data = create_type ("convert",
                        ir_opcode::        convert,
                        flag::has_def::    yes,
                        flag::arity::      unary,
                        flag::is_abstract::no);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::branch>
  {
    static constexpr
    impl
    data = create_type ("branch",
                        ir_opcode::        branch,
                        flag::has_def::    no,
                        flag::arity::      n_ary,
                        flag::is_abstract::yes);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::relation>
  {
    static constexpr
    impl
    data = create_type ("relation",
                        ir_opcode::        relation,
                        flag::has_def::    yes,
                        flag::arity::      binary,
                        flag::is_abstract::yes);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::arithmetic>
  {
    static constexpr
    impl
    data = create_type ("arithmetic",
                        ir_opcode::        arithmetic,
                        flag::has_def::    yes,
                        flag::arity::      n_ary,
                        flag::is_abstract::yes);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::logical>
  {
    static constexpr
    impl
    data = create_type ("logical",
                        ir_opcode::        logical,
                        flag::has_def::    yes,
                        flag::arity::      n_ary,
                        flag::is_abstract::yes) ;
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::bitwise>
  {
    static constexpr
    impl
    data = create_type ("bitwise",
                        ir_opcode::        bitwise,
                        flag::has_def::    yes,
                        flag::arity::      n_ary,
                        flag::is_abstract::yes);
  };

  /* branch */

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::cbranch>
  {
    static constexpr
    impl
    data = get<ir_opcode::branch> ().derive ("br",
                                             ir_opcode::  cbranch,
                                             flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::ucbranch>
  {
    static constexpr
    impl
    data = get<ir_opcode::branch> ().derive ("ubr",
                                             ir_opcode::  ucbranch,
                                             flag::arity::unary);
  };

  /* relation */

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::eq>
  {
    static constexpr
    impl
    data = get<ir_opcode::relation> ().derive ("==",
                                               ir_opcode::eq);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::ne>
  {
    static constexpr
    impl
    data = get<ir_opcode::relation> ().derive ("!=",
                                               ir_opcode::ne);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::lt>
  {
    static constexpr
    impl
    data = get<ir_opcode::relation> ().derive ("<" ,
                                               ir_opcode::lt);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::le>
  {
    static constexpr
    impl
    data = get<ir_opcode::relation> ().derive ("<=",
                                               ir_opcode::le);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::gt>
  {
    static constexpr
    impl
    data = get<ir_opcode::relation> ().derive (">" ,
                                               ir_opcode::gt);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::ge>
  {
    static constexpr
    impl
    data = get<ir_opcode::relation> ().derive (">=",
                                               ir_opcode::ge);
  };

  /* arithmetic */

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::add>
  {
    static constexpr
    impl
    data = get<ir_opcode::arithmetic> ().derive ("+",
                                                 ir_opcode::  add,
                                                 flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::sub>
  {
    static constexpr
    impl
    data = get<ir_opcode::arithmetic> ().derive ("-",
                                                 ir_opcode::  sub,
                                                 flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::mul>
  {
    static constexpr
    impl
    data = get<ir_opcode::arithmetic> ().derive ("*",
                                                 ir_opcode::  mul,
                                                 flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::div>
  {
    static constexpr
    impl
    data = get<ir_opcode::arithmetic> ().derive ("/",
                                                 ir_opcode::  div,
                                                 flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::mod>
  {
    static constexpr
    impl
    data = get<ir_opcode::arithmetic> ().derive ("%",
                                                 ir_opcode::  mod,
                                                 flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::rem>
  {
    static constexpr
    impl
    data = get<ir_opcode::arithmetic> ().derive ("rem",
                                                 ir_opcode::  rem,
                                                 flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::neg>
  {
    static constexpr
    impl
    data = get<ir_opcode::arithmetic> ().derive ("-",
                                                 ir_opcode::  neg,
                                                 flag::arity::unary );
  };

  /* logical */

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::land>
  {
    static constexpr
    impl
    data = get<ir_opcode::logical> ().derive ("&&",
                                              ir_opcode::  land,
                                              flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::lor>
  {
    static constexpr
    impl
    data = get<ir_opcode::logical> ().derive ("||",
                                              ir_opcode::  lor,
                                              flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::lnot>
  {
    static constexpr
    impl
    data = get<ir_opcode::logical> ().derive ("!",
                                              ir_opcode::  lnot,
                                              flag::arity::unary );
  };

  /* bitwise */

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::band>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive ("&",
                                              ir_opcode::  band,
                                              flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::bor>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive ("|",
                                              ir_opcode::  bor,
                                              flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::bxor>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive ("^",
                                              ir_opcode::  bxor,
                                              flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::bshiftl>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive ("<<",
                                              ir_opcode::  bshiftl,
                                              flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::bashiftr>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive (">>",
                                              ir_opcode::  bashiftr,
                                              flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::blshiftr>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive (">>",
                                              ir_opcode::  blshiftr,
                                              flag::arity::binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::bnot>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive ("~",
                                              ir_opcode::  bnot,
                                              flag::arity::unary );
  };

  template <ir_opcode Op>
  inline constexpr
  ir_instruction_metadata
  ir_instruction_metadata_v = ir_instruction_metadata::get<Op> ();

  constexpr
  ir_instruction_metadata
  get_metadata (ir_opcode op)
  {
    constexpr auto values = ir_instruction_metadata::generate_array ();
    return values[static_cast<std::underlying_type_t<ir_opcode>> (op)];
  }

  static_assert (ir_instruction_metadata_v<ir_opcode::ge> == get_metadata (ir_opcode::ge));

}

#endif // OCTAVE_IR_IR_INSTRUCTION_METADATA_HPP

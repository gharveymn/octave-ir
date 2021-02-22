/** ir-instruction-metadata.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_INSTRUCTION_METADATA_HPP
#define OCTAVE_IR_IR_INSTRUCTION_METADATA_HPP

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

    enum class returning : bool
    {
      yes = true,
      no  = false,
    };

    enum class abstract : bool
    {
      yes = true,
      no  = false,
    };

  private:
    struct impl
    {
      const char *    m_name;
      const impl *    m_base;
      const ir_opcode m_opcode;
      const arity     m_arity;
      const returning m_returning;
      const abstract  m_abstract;
    };

    [[nodiscard]] constexpr
    returning
    returning_state (void) const noexcept
    {
      return m_ptr->m_returning;
    }

    [[nodiscard]] constexpr
    abstract
    abstract_state (void) const noexcept
    {
      return m_ptr->m_abstract;
    }

    template <ir_opcode Op>
    struct instance;

    constexpr
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

    [[nodiscard]] constexpr
    const char *
    get_name (void) const noexcept
    {
      return m_ptr->m_name;
    }

    [[nodiscard]] constexpr
    ir_instruction_metadata
    get_base (void) const noexcept
    {
      return *m_ptr->m_base;
    }

    [[nodiscard]] constexpr
    ir_opcode
    get_opcode (void) const noexcept
    {
      return m_ptr->m_opcode;
    }

    [[nodiscard]] constexpr
    arity
    get_arity (void) const noexcept
    {
      return m_ptr->m_arity;
    }

    [[nodiscard]] constexpr
    bool
    will_return (void) const noexcept
    {
      return returning_state () == returning::yes;
    }

    [[nodiscard]] constexpr
    bool
    is_abstract (void) const noexcept
    {
      return abstract_state () == abstract::yes;
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
    create_type (const char* name, ir_opcode Op, returning ret, arity n, abstract abs) noexcept
    {
      return { name, nullptr, Op, n, ret, abs };
    }

    [[nodiscard]] constexpr
    impl
    derive (const char* name, ir_opcode Op, returning ret,
            abstract abs = abstract::no) const noexcept
    {
      return { name, m_ptr, Op, get_arity (), ret, abs };
    }

    [[nodiscard]] constexpr
    impl
    derive (const char* name, ir_opcode Op, returning ret, arity n,
            abstract abs = abstract::no) const noexcept
    {
      return { name, m_ptr, Op, n, ret, abs };
    }

    [[nodiscard]] constexpr
    impl
    derive (const char* name, ir_opcode Op, arity n, abstract abs = abstract::no) const noexcept
    {
      return { name, m_ptr, Op, n, returning_state (), abs };
    }

    [[nodiscard]] constexpr
    impl
    derive (const char* name, ir_opcode Op, abstract abs = abstract::no) const noexcept
    {
      return { name, m_ptr, Op, get_arity (), returning_state (), abs };
    }

    const impl *m_ptr;
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

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::phi>
  {
    static constexpr
    impl
    data = create_type ("phi",
                        ir_opcode::phi,
                        returning::yes,
                        arity::    n_ary,
                        abstract:: no);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::assign>
  {
    static constexpr
    impl
    data = create_type ("assign",
                        ir_opcode::assign,
                        returning::yes,
                        arity::    unary,
                        abstract:: no);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::call>
  {
    static constexpr
    impl
    data = create_type ("call",
                        ir_opcode::call,
                        returning::yes,
                        arity::    n_ary,
                        abstract:: no);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::fetch>
  {
    static constexpr
    impl
    data = create_type ("fetch",
                        ir_opcode::fetch,
                        returning::yes,
                        arity::    unary,
                        abstract:: no);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::convert>
  {
    static constexpr
    impl
    data = create_type ("convert",
                        ir_opcode::convert,
                        returning::yes,
                        arity::    unary,
                        abstract:: no);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::branch>
  {
    static constexpr
    impl
    data = create_type ("branch",
                        ir_opcode::branch,
                        returning::no,
                        arity::    n_ary,
                        abstract:: yes);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::relation>
  {
    static constexpr
    impl
    data = create_type ("relation",
                        ir_opcode::relation,
                        returning::yes,
                        arity::    binary,
                        abstract:: yes);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::arithmetic>
  {
    static constexpr
    impl
    data = create_type ("arithmetic",
                        ir_opcode::arithmetic,
                        returning::yes,
                        arity::    n_ary,
                        abstract:: yes);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::logical>
  {
    static constexpr
    impl
    data = create_type ("logical",
                        ir_opcode::logical,
                        returning::yes,
                        arity::    n_ary,
                        abstract:: yes) ;
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::bitwise>
  {
    static constexpr
    impl
    data = create_type ("bitwise",
                        ir_opcode::bitwise,
                        returning::yes,
                        arity::    n_ary,
                        abstract:: yes);
  };

  /* branch */

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::cbranch>
  {
    static constexpr
    impl
    data = get<ir_opcode::branch> ().derive ("br",
                                             ir_opcode::cbranch,
                                             arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::ucbranch>
  {
    static constexpr
    impl
    data = get<ir_opcode::branch> ().derive ("ubr",
                                             ir_opcode::ucbranch,
                                             arity::    unary);
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
                                                 ir_opcode::add,
                                                 arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::sub>
  {
    static constexpr
    impl
    data = get<ir_opcode::arithmetic> ().derive ("-",
                                                 ir_opcode::sub,
                                                 arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::mul>
  {
    static constexpr
    impl
    data = get<ir_opcode::arithmetic> ().derive ("*",
                                                 ir_opcode::mul,
                                                 arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::div>
  {
    static constexpr
    impl
    data = get<ir_opcode::arithmetic> ().derive ("/",
                                                 ir_opcode::div,
                                                 arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::mod>
  {
    static constexpr
    impl
    data = get<ir_opcode::arithmetic> ().derive ("%",
                                                 ir_opcode::mod,
                                                 arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::rem>
  {
    static constexpr
    impl
    data = get<ir_opcode::arithmetic> ().derive ("rem",
                                                 ir_opcode::rem,
                                                 arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::neg>
  {
    static constexpr
    impl
    data = get<ir_opcode::arithmetic> ().derive ("-",
                                                 ir_opcode::neg,
                                                 arity::    unary );
  };

  /* logical */

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::land>
  {
    static constexpr
    impl
    data = get<ir_opcode::logical> ().derive ("&&",
                                              ir_opcode::land,
                                              arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::lor>
  {
    static constexpr
    impl
    data = get<ir_opcode::logical> ().derive ("||",
                                              ir_opcode::lor,
                                              arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::lnot>
  {
    static constexpr
    impl
    data = get<ir_opcode::logical> ().derive ("!",
                                              ir_opcode::lnot,
                                              arity::    unary );
  };

  /* bitwise */

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::band>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive ("&",
                                              ir_opcode::band,
                                              arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::bor>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive ("|",
                                              ir_opcode::bor,
                                              arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::bxor>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive ("^",
                                              ir_opcode::bxor,
                                              arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::bshiftl>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive ("<<",
                                              ir_opcode::bshiftl,
                                              arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::bashiftr>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive (">>",
                                              ir_opcode::bashiftr,
                                              arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::blshiftr>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive (">>",
                                              ir_opcode::blshiftr,
                                              arity::    binary);
  };

  template <>
  struct ir_instruction_metadata::instance<ir_opcode::bnot>
  {
    static constexpr
    impl
    data = get<ir_opcode::bitwise> ().derive ("~",
                                              ir_opcode::bnot,
                                              arity::    unary );
  };

  template <ir_opcode Op>
  inline constexpr
  ir_instruction_metadata ir_instruction_metadata_v { ir_instruction_metadata::get<Op> () };

}

#endif // OCTAVE_IR_IR_INSTRUCTION_METADATA_HPP

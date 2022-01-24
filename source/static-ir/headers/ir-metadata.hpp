/** ir-instruction-metadata.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_STATIC_IR_IR_METADATA_HPP
#define OCTAVE_IR_STATIC_IR_IR_METADATA_HPP

#include "ir-common.hpp"
#include "ir-functional.hpp"
#include "ir-type-traits.hpp"
#include "ir-utility.hpp"

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace gch
{

  enum class ir_opcode : unsigned
  {
    phi        ,
    assign     ,
    call       ,
    fetch      ,
    convert    ,

    relation   , // abstract
    eq         ,
    ne         ,
    lt         ,
    le         ,
    gt         ,
    ge         ,

    arithmetic , // abstract
    add        ,
    sub        ,
    mul        ,
    div        ,
    mod        ,
    rem        ,
    neg        ,

    logical    , // abstract
    land       ,
    lor        ,
    lnot       ,

    bitwise    , // abstract
    band       ,
    bor        ,
    bxor       ,
    bshiftl    ,
    bashiftr   ,
    blshiftr   ,
    bnot       ,

    terminal   , // abstract
    branch     , // abstract
    cbranch    ,
    ucbranch   ,
    unreachable,
    terminate  ,
    ret        ,
  };

  inline constexpr
  std::size_t
  num_ir_opcodes = static_cast<std::underlying_type_t<ir_opcode>> (ir_opcode::ret) + 1;

  static_assert (num_ir_opcodes == 39);

  class ir_metadata
  {
  public:

    using opcode_indices = std::make_index_sequence<num_ir_opcodes>;
    using opcode_pack    = wrap_integer_sequence_t<opcode_indices, ir_opcode>;

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
    ir_metadata (const impl& impl_ref) noexcept
      : m_ptr (&impl_ref)
    { }

  public:
    ir_metadata            (void)                   = delete;
    ir_metadata            (const ir_metadata&)     = default;
    ir_metadata            (ir_metadata&&) noexcept = default;
    ir_metadata& operator= (const ir_metadata&)     = default;
    ir_metadata& operator= (ir_metadata&&) noexcept = default;
    ~ir_metadata           (void)                   = default;

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    const char *
    get_name (void) const noexcept
    {
      return m_ptr->m_name;
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    ir_metadata
    get_base (void) const noexcept
    {
      return ir_metadata { *m_ptr->m_base };
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
    flag::has_def
    get_has_def (void) const noexcept
    {
      return m_ptr->m_has_def;
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    flag::is_abstract
    get_is_abstract (void) const noexcept
    {
      return m_ptr->m_abstract;
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
    operator== (const ir_metadata& lhs, const ir_metadata& rhs) noexcept;

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    bool
    is_a (ir_metadata cmp) const noexcept
    {
      return (cmp == *this) || (has_base () && get_base ().is_a (cmp));
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    bool
    is_base_of (ir_metadata cmp) const noexcept
    {
      return cmp.is_a (*this);
    }

    template <ir_opcode Op>
    [[nodiscard]]
    static GCH_CPP20_CONSTEVAL
    ir_metadata
    get (void) noexcept
    {
      return ir_metadata { instance<Op>::data };
    }

    [[nodiscard]]
    static GCH_CPP20_CONSTEVAL
    ir_metadata
    get (ir_opcode op) noexcept;

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    std::underlying_type_t<ir_opcode>
    get_index (void) const noexcept
    {
      return underlying_cast (get_opcode ());
    }

  private:
    template <typename Value>
    struct value_map
    {
      constexpr
      const Value&
      operator[] (ir_opcode op) const noexcept
      {
        return data[static_cast<std::underlying_type_t<ir_opcode>> (op)];
      }

      constexpr
      const Value&
      operator[] (ir_metadata m) const noexcept
      {
        return data[m.get_index ()];
      }

      template <ir_opcode op>
      [[nodiscard]] constexpr
      const Value&
      get (void) const noexcept
      {
        return (*this)[op];
      }

      std::array<Value, num_ir_opcodes> data;
    };

    struct identity_projection
    {
      GCH_CPP20_CONSTEVAL
      ir_metadata
      operator() (ir_metadata m) const noexcept
      {
        return m;
      }
    };

    template <typename IndexSequence = opcode_indices>
    struct map_generator;

    template <template <ir_opcode> typename MapperT, typename IndexSequence = opcode_indices>
    struct map_generator_template;

    template <template <ir_opcode> typename MapperT, typename IndexSequence, typename ...Args>
    struct common_mapping_result_impl
    { };

    template <template <ir_opcode> typename MapperT, std::size_t ...Indices, typename ...Args>
    struct common_mapping_result_impl<MapperT, std::index_sequence<Indices...>, Args...>
      : std::common_type<std::remove_reference_t<std::invoke_result_t<
          MapperT<static_cast<ir_opcode> (Indices)>, Args...>>...>
    { };

    template <template <ir_opcode> typename MapperT, typename ...Args>
    struct common_mapping_result
      : common_mapping_result_impl<MapperT, opcode_indices, Args...>
    { };

    template <template <ir_opcode> typename MapperT, typename ...Args>
    using common_mapping_result_t =
      typename common_mapping_result<MapperT, Args...>::type;

  public:
    template <typename Projection = identity_projection>
    [[nodiscard]]
    static GCH_CPP20_CONSTEVAL
    value_map<std::remove_reference_t<std::invoke_result_t<Projection, ir_metadata>>>
    generate_map (Projection proj = { }) noexcept;

    template <template <ir_opcode> typename MapperT, typename ...Args>
    [[nodiscard]]
    static GCH_CPP20_CONSTEVAL
    value_map<common_mapping_result_t<MapperT, const Args&...>>
    generate_map (const Args&... args) noexcept;

  private:
    [[nodiscard]]
    static GCH_CPP20_CONSTEVAL
    impl
    create_type (const char *      name,
                 ir_opcode         op,
                 flag::has_def     def,
                 flag::arity       n,
                 flag::is_abstract abs) noexcept
    {
      return { name, nullptr, op, n, def, abs };
    }

    template <ir_opcode OpBase>
    [[nodiscard]]
    static GCH_CPP20_CONSTEVAL
    impl
    derive (const char *      name,
            ir_opcode         op,
            flag::has_def     def,
            flag::is_abstract abs = flag::is_abstract::no) noexcept
    {
      ir_metadata base = ir_metadata::get<OpBase> ();
      return { name, base.m_ptr, op, base.get_arity (), def, abs };
    }

    template <ir_opcode OpBase>
    [[nodiscard]]
    static GCH_CPP20_CONSTEVAL
    impl
    derive (const char *      name,
            ir_opcode         op,
            flag::has_def     def,
            flag::arity       n,
            flag::is_abstract abs = flag::is_abstract::no) noexcept
    {
      ir_metadata base = ir_metadata::get<OpBase> ();
      return { name, base.m_ptr, op, n, def, abs };
    }

    template <ir_opcode OpBase>
    [[nodiscard]]
    static GCH_CPP20_CONSTEVAL
    impl
    derive (const char *      name,
            ir_opcode         op,
            flag::arity       n,
            flag::is_abstract abs = flag::is_abstract::no) noexcept
    {
      ir_metadata base = ir_metadata::get<OpBase> ();
      return { name, base.m_ptr, op, n, base.m_ptr->m_has_def, abs };
    }

    template <ir_opcode OpBase>
    [[nodiscard]]
    static GCH_CPP20_CONSTEVAL
    impl
    derive (const char *      name,
            ir_opcode         op,
            flag::is_abstract abs = flag::is_abstract::no) noexcept
    {
      ir_metadata base = ir_metadata::get<OpBase> ();
      return { name, base.m_ptr, op, base.get_arity (), base.m_ptr->m_has_def, abs };
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    impl
    derive (const char *      name,
            ir_opcode         op,
            flag::has_def     def,
            flag::is_abstract abs = flag::is_abstract::no) const noexcept
    {
      return { name, m_ptr, op, get_arity (), def, abs };
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    impl
    derive (const char *      name,
            ir_opcode         op,
            flag::has_def     def,
            flag::arity       n,
            flag::is_abstract abs = flag::is_abstract::no) const noexcept
    {
      return { name, m_ptr, op, n, def, abs };
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    impl
    derive (const char *      name,
            ir_opcode         op,
            flag::arity       n,
            flag::is_abstract abs = flag::is_abstract::no) const noexcept
    {
      return { name, m_ptr, op, n, m_ptr->m_has_def, abs };
    }

    [[nodiscard]] GCH_CPP20_CONSTEVAL
    impl
    derive (const char *      name,
            ir_opcode         op,
            flag::is_abstract abs = flag::is_abstract::no) const noexcept
    {
      return { name, m_ptr, op, get_arity (), m_ptr->m_has_def, abs };
    }

    const impl *m_ptr;
  };

  template <std::size_t ...Indices>
  struct ir_metadata::map_generator<std::index_sequence<Indices...>>
  {
    static_assert (std::is_same_v<opcode_indices, std::index_sequence<Indices...>>);

    template <typename Projection>
    constexpr
    value_map<std::remove_reference_t<std::invoke_result_t<Projection, ir_metadata>>>
    operator() (Projection proj) const noexcept
    {
      return { gch::invoke (proj, get<static_cast<ir_opcode> (Indices)> ())... };
    }
  };

  template <template <ir_opcode> typename MapperT, std::size_t ...Indices>
  struct ir_metadata::map_generator_template<MapperT, std::index_sequence<Indices...>>
  {
    static_assert (std::is_same_v<opcode_indices, std::index_sequence<Indices...>>);

    template <typename ...Args>
    constexpr
    value_map<common_mapping_result_t<MapperT, const Args&...>>
    operator() (const Args&... args) const
      noexcept (noexcept (
        value_map<common_mapping_result_t<MapperT, const Args&...>> {
          gch::invoke (MapperT<static_cast<ir_opcode> (Indices)> { }, args...)... }))
    {
      return { gch::invoke (MapperT<static_cast<ir_opcode> (Indices)> { }, args...)... };
    }
  };

  template <typename Projection>
  GCH_CPP20_CONSTEVAL
  auto
  ir_metadata::
  generate_map (Projection proj) noexcept
    -> value_map<std::remove_reference_t<std::invoke_result_t<Projection, ir_metadata>>>
  {
    return gch::invoke (map_generator<> { }, proj);
  }

  template <template <ir_opcode> typename MapperT, typename ...Args>
  GCH_CPP20_CONSTEVAL
  auto
  ir_metadata::
  generate_map (const Args&... args) noexcept
    -> value_map<common_mapping_result_t<MapperT, const Args&...>>
  {
    return gch::invoke (map_generator_template<MapperT> { }, args...);
  }

  [[nodiscard]] GCH_CPP20_CONSTEVAL
  bool
  operator== (const ir_metadata& lhs, const ir_metadata& rhs) noexcept
  {
    return lhs.m_ptr->m_opcode == rhs.m_ptr->m_opcode;
  }

  [[nodiscard]] GCH_CPP20_CONSTEVAL
  bool
  operator!= (const ir_metadata& lhs, const ir_metadata& rhs) noexcept
  {
    return ! (lhs == rhs);
  }

  template <>
  struct ir_metadata::instance<ir_opcode::phi>
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
  struct ir_metadata::instance<ir_opcode::assign>
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
  struct ir_metadata::instance<ir_opcode::call>
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
  struct ir_metadata::instance<ir_opcode::fetch>
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
  struct ir_metadata::instance<ir_opcode::convert>
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
  struct ir_metadata::instance<ir_opcode::relation>
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
  struct ir_metadata::instance<ir_opcode::arithmetic>
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
  struct ir_metadata::instance<ir_opcode::logical>
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
  struct ir_metadata::instance<ir_opcode::bitwise>
  {
    static constexpr
    impl
    data = create_type ("bitwise",
                        ir_opcode::        bitwise,
                        flag::has_def::    yes,
                        flag::arity::      n_ary,
                        flag::is_abstract::yes);
  };

  /* terminal */

  template <>
  struct ir_metadata::instance<ir_opcode::terminal>
  {
    static constexpr
    impl
    data = create_type ("terminate",
                        ir_opcode::        terminal,
                        flag::has_def::    no,
                        flag::arity::      n_ary,
                        flag::is_abstract::yes);
  };

  /* branch */

  template <>
  struct ir_metadata::instance<ir_opcode::branch>
  {
    static constexpr
    impl
    data = derive<ir_opcode::terminal> ("branch",
                                        ir_opcode::        branch,
                                        flag::is_abstract::yes);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::cbranch>
  {
    static constexpr
    impl
    data = derive<ir_opcode::branch> ("br",
                                      ir_opcode::  cbranch,
                                      flag::arity::ternary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::ucbranch>
  {
    static constexpr
    impl
    data = derive<ir_opcode::branch> ("ubr",
                                      ir_opcode::  ucbranch,
                                      flag::arity::unary);
  };


  template <>
  struct ir_metadata::instance<ir_opcode::unreachable>
  {
    static constexpr
    impl
    data = derive<ir_opcode::terminal> ("unreachable",
                                        ir_opcode::  unreachable,
                                        flag::arity::nullary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::terminate>
  {
    static constexpr
    impl
    data = derive<ir_opcode::terminal> ("terminate",
                                        ir_opcode::  terminate,
                                        flag::arity::nullary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::ret>
  {
    static constexpr
    impl
    data = derive<ir_opcode::terminal> ("return",
                                        ir_opcode::  ret,
                                        flag::arity::unary);
  };

  /* relation */

  template <>
  struct ir_metadata::instance<ir_opcode::eq>
  {
    static constexpr
    impl
    data = derive<ir_opcode::relation> ("==", ir_opcode::eq);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::ne>
  {
    static constexpr
    impl
    data = derive<ir_opcode::relation> ("!=", ir_opcode::ne);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::lt>
  {
    static constexpr
    impl
    data = derive<ir_opcode::relation> ("<" , ir_opcode::lt);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::le>
  {
    static constexpr
    impl
    data = derive<ir_opcode::relation> ("<=", ir_opcode::le);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::gt>
  {
    static constexpr
    impl
    data = derive<ir_opcode::relation> (">" , ir_opcode::gt);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::ge>
  {
    static constexpr
    impl
    data = derive<ir_opcode::relation> (">=", ir_opcode::ge);
  };

  /* arithmetic */

  template <>
  struct ir_metadata::instance<ir_opcode::add>
  {
    static constexpr
    impl
    data = derive<ir_opcode::arithmetic> ("+", ir_opcode::add, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::sub>
  {
    static constexpr
    impl
    data = derive<ir_opcode::arithmetic> ("-", ir_opcode::sub, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::mul>
  {
    static constexpr
    impl
    data = derive<ir_opcode::arithmetic> ("*", ir_opcode::mul, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::div>
  {
    static constexpr
    impl
    data = derive<ir_opcode::arithmetic> ("/", ir_opcode::div, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::mod>
  {
    static constexpr
    impl
    data = derive<ir_opcode::arithmetic> ("%", ir_opcode::mod, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::rem>
  {
    static constexpr
    impl
    data = derive<ir_opcode::arithmetic> ("rem", ir_opcode::rem, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::neg>
  {
    static constexpr
    impl
    data = derive<ir_opcode::arithmetic> ("-", ir_opcode::neg, flag::arity::unary );
  };

  /* logical */

  template <>
  struct ir_metadata::instance<ir_opcode::land>
  {
    static constexpr
    impl
    data = derive<ir_opcode::logical> ("&&", ir_opcode::land, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::lor>
  {
    static constexpr
    impl
    data = derive<ir_opcode::logical> ("||", ir_opcode::lor, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::lnot>
  {
    static constexpr
    impl
    data = derive<ir_opcode::logical> ("!", ir_opcode::lnot, flag::arity::unary );
  };

  /* bitwise */

  template <>
  struct ir_metadata::instance<ir_opcode::band>
  {
    static constexpr
    impl
    data = derive<ir_opcode::bitwise> ("&", ir_opcode::band, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::bor>
  {
    static constexpr
    impl
    data = derive<ir_opcode::bitwise> ("|", ir_opcode::bor, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::bxor>
  {
    static constexpr
    impl
    data = derive<ir_opcode::bitwise> ("^", ir_opcode::bxor, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::bshiftl>
  {
    static constexpr
    impl
    data = derive<ir_opcode::bitwise> ("<<", ir_opcode::bshiftl, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::bashiftr>
  {
    static constexpr
    impl
    data = derive<ir_opcode::bitwise> (">>", ir_opcode::bashiftr, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::blshiftr>
  {
    static constexpr
    impl
    data = derive<ir_opcode::bitwise> (">>", ir_opcode::blshiftr, flag::arity::binary);
  };

  template <>
  struct ir_metadata::instance<ir_opcode::bnot>
  {
    static constexpr
    impl
    data = derive<ir_opcode::bitwise> ("~", ir_opcode::bnot, flag::arity::unary );
  };

  template <ir_opcode Op>
  inline constexpr
  ir_metadata
  ir_metadata_v = ir_metadata::get<Op> ();

  GCH_CPP20_CONSTEVAL
  ir_metadata
  ir_metadata::
  get (ir_opcode op) noexcept
  {
    constexpr auto map = generate_map ();
    return map[op];
  }

  template <ir_opcode Op>
  struct is_comparison_op
    : std::bool_constant<Op == ir_opcode::eq
                      || Op == ir_opcode::ne
                      || Op == ir_opcode::lt
                      || Op == ir_opcode::le
                      || Op == ir_opcode::gt
                      || Op == ir_opcode::ge>
  { };

  template <ir_opcode Op>
  inline constexpr
  bool
  is_comparison_op_v = is_comparison_op<Op>::value;

  template <ir_opcode Op>
  struct ir_instruction_traits
  {
    explicit
    ir_instruction_traits (void) = default;

    static constexpr auto metadata = ir_metadata_v<Op>;

    template <ir_opcode BaseOp>
    static constexpr
    bool
    is_a = metadata.is_a (ir_metadata_v<BaseOp>);

    template <ir_opcode OtherOp>
    static constexpr
    bool
    is_base_of = metadata.is_base_of (ir_metadata_v<OtherOp>);

    static constexpr auto opcode      = metadata.get_opcode ();
    static constexpr auto name        = metadata.get_name ();
    static constexpr auto is_abstract = metadata.is_abstract ();
    static constexpr auto has_base    = metadata.has_base ();
    static constexpr auto has_def     = metadata.has_def ();
    static constexpr auto arity       = metadata.get_arity ();

    static constexpr auto is_n_ary    = metadata.is_n_ary ();
    static constexpr auto is_nullary  = metadata.is_nullary ();
    static constexpr auto is_unary    = metadata.is_unary ();
    static constexpr auto is_binary   = metadata.is_binary ();
    static constexpr auto is_ternary  = metadata.is_ternary ();

    static constexpr auto is_arithmetic = is_a<ir_opcode::arithmetic>;
    static constexpr auto is_bitwise    = is_a<ir_opcode::bitwise>;
    static constexpr auto is_branch     = is_a<ir_opcode::branch>;
    static constexpr auto is_logical    = is_a<ir_opcode::logical>;
    static constexpr auto is_relation   = is_a<ir_opcode::relation>;

    template <bool HasBase = has_base, std::enable_if_t<HasBase> * = nullptr>
    static constexpr
    auto
    base = metadata.get_base ();

    template <std::size_t N>
    static constexpr
    bool
    is_valid_num_args = is_n_ary || (static_cast<std::size_t> (arity) == N);
  };

}

#endif // OCTAVE_IR_STATIC_IR_IR_METADATA_HPP

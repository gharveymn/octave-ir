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
  enum class ir_opcode : unsigned
  {
    _span_    =  35,
    
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
  
  class ir_instruction2
  {
  public:
    using args_container_type = std::vector<ir_operand>;
    using optional_def_type   = std::optional<ir_def>;
  
    class metadata
    {
    public:
      enum class arity : int
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
        const char*     m_name;
        const impl*     m_base;
        const ir_opcode m_opcode;
        const arity     m_arity;
        const bool      m_has_return;
        const bool      m_is_abstract;
      };
    
      static_assert (std::is_standard_layout_v<impl> && std::is_trivial_v<impl>);
    
      constexpr metadata (const impl& impl_ref) noexcept
        : m_ptr (impl_ref)
      { }
  
    public:
      metadata            (void)                = delete;
      metadata            (const metadata&)     = default;
      metadata            (metadata&&) noexcept = default;
      metadata& operator= (const metadata&)     = default;
      metadata& operator= (metadata&&) noexcept = default;
      ~metadata           (void)                = default;
    
      [[nodiscard]] constexpr bool is_a (metadata cmp) const noexcept
      {
        return cmp == *this ? true : (has_base () ? get_base ().is_a (cmp) : false);
      }
    
      [[nodiscard]] constexpr ir_opcode   get_opcode  (void) const noexcept { return  m_ptr->m_opcode;      }
      [[nodiscard]] constexpr const char* get_name    (void) const noexcept { return  m_ptr->m_name;        }
      [[nodiscard]] constexpr bool        is_abstract (void) const noexcept { return  m_ptr->m_is_abstract; }
      [[nodiscard]] constexpr metadata    get_base    (void) const noexcept { return *m_ptr->m_base;        }
      [[nodiscard]] constexpr bool        has_return  (void) const noexcept { return  m_ptr->m_has_return;  }
      [[nodiscard]] constexpr arity       get_arity   (void) const noexcept { return  m_ptr->m_arity;       }
    
      [[nodiscard]] constexpr bool has_base (void) const noexcept { return m_ptr->m_base != nullptr; }
    
      [[nodiscard]] constexpr bool is_n_ary   (void)  const noexcept  { return  get_arity () ==  arity::n_ary;   }
      [[nodiscard]] constexpr bool is_nullary (void)  const noexcept  { return  get_arity () ==  arity::nullary; }
      [[nodiscard]] constexpr bool is_unary   (void)  const noexcept  { return  get_arity () ==  arity::unary;   }
      [[nodiscard]] constexpr bool is_binary  (void)  const noexcept  { return  get_arity () ==  arity::binary;  }
      [[nodiscard]] constexpr bool is_ternary (void)  const noexcept  { return  get_arity () ==  arity::ternary; }
      
      [[nodiscard]] constexpr bool valid_num_args (std::size_t n) const noexcept
      {
        return (n > 3  && is_n_ary ()) || (n == static_cast<std::size_t> (get_arity ()));
      }
    
      [[nodiscard]] constexpr bool operator== (const metadata& other) const noexcept
      {
        return m_ptr == other.m_ptr;
      }
    
      [[nodiscard]] constexpr bool operator!= (const metadata& other) const noexcept
      {
        return ! (other == *this);
      }
    
      template <ir_opcode Tag>
      [[nodiscard]] static constexpr metadata get (void) noexcept { return instance<Tag>; }
    
      template <ir_opcode Tag>
      [[nodiscard]]
      static constexpr bool get_name (void) noexcept
      {
        return get<Tag> ().get_name ();
      }
    
      template <ir_opcode Tag>
      [[nodiscard]]
      static constexpr bool is_abstract (void) noexcept
      {
        return get<Tag> ().is_abstract ();
      }
    
      template <ir_opcode Tag>
      [[nodiscard]]
      static constexpr bool has_return (void) noexcept
      {
        return get<Tag> ().has_return ();
      }
    
      template <ir_opcode Tag>
      [[nodiscard]]
      static constexpr arity get_arity (void) noexcept
      {
        return get<Tag> ().get_arity ();
      }
    
      template <ir_opcode Tag>
      [[nodiscard]]
      static constexpr std::enable_if_t<get<Tag> ().has_base (), metadata> get_base (void) noexcept
      {
        return get<Tag> ().get_base ();
      }
    
      template <ir_opcode BaseTag>
      [[nodiscard]]
      static constexpr bool is_a (metadata m) noexcept
      {
        return m.is_a (get<BaseTag> ());
      }
    
      template <ir_opcode Tag, ir_opcode BaseTag>
      [[nodiscard]]
      static constexpr bool is_a (void) noexcept
      {
        return is_a<BaseTag> (get<Tag> ());
      }
  
    private:
    
      template <ir_opcode Tag>
      [[nodiscard]]
      static constexpr const impl* get_pointer (void) noexcept
      {
        return get<Tag> ().m_ptr;
      }
    
      template <ir_opcode Tag, ir_opcode BaseTag, bool HasReturn, arity Arity = get_arity<BaseTag> (), bool IsAbstract = false>
      static constexpr impl create_type (const char* name) noexcept
      {
        return { name, get_pointer<BaseTag> (), Tag, Arity, HasReturn, IsAbstract };
      }
    
      template <ir_opcode Tag, ir_opcode BaseTag, arity Arity, bool IsAbstract = false>
      static constexpr impl create_type (const char* name) noexcept
      {
        return { name, get_pointer<BaseTag> (), Tag, Arity, has_return<BaseTag> (), IsAbstract };
      }
    
      template <ir_opcode Tag, ir_opcode BaseTag, bool IsAbstract = false>
      static constexpr impl create_type (const char* name) noexcept
      {
        return { name, get_pointer<BaseTag> (), Tag, get_arity<BaseTag> (), has_return<BaseTag> (), IsAbstract };
      }
    
      template <ir_opcode Tag, bool HasReturn, arity Arity = arity::n_ary, bool IsAbstract = false>
      static constexpr impl create_type (const char* name) noexcept
      {
        return { name, nullptr, Tag, Arity, HasReturn, IsAbstract };
      }
    
      template <ir_opcode Tag, bool HasReturn, bool IsAbstract = false>
      static constexpr impl create_type (const char* name) noexcept
      {
        return { name, nullptr, Tag, arity::n_ary, HasReturn, IsAbstract };
      }
    
      template <ir_opcode Tag, arity Arity = arity::n_ary, bool IsAbstract = false>
      static constexpr impl create_type (const char* name) noexcept
      {
        return { name, nullptr, Tag, Arity, false, IsAbstract };
      }
    
      template <ir_opcode Tag>
      static inline constexpr impl instance = { nullptr, nullptr, Tag, arity::n_ary, false, true };
    
      nonnull_ptr<const impl> m_ptr;
    };
  
    template <ir_opcode Tag>
    static inline constexpr metadata metadata_v = metadata::get<Tag> ();
    
    template <ir_opcode Tag, typename ...Ts>
    ir_instruction2 (ir_def&& ret, Ts&&... ts)
      : m_metadata (metadata_v<Tag>),
        m_return   (std::move (ret)),
        m_args     ({ std::forward<Ts> (ts)... })
    {
      static_assert (metadata_v<Tag>.has_return (),
                     "Instruction metadata specified no such return.");
      
      static_assert (metadata_v<Tag>.valid_num_args (sizeof...(Ts)),
                     "Instruction metadata specified a different number of arguments.");
    }
  
    template <ir_opcode Tag, typename ...Ts>
    ir_instruction2 (ir_operand&& operand, Ts&&... ts)
      : m_metadata (metadata_v<Tag>),
        m_return   (std::nullopt),
        m_args     ({ std::move (operand), std::forward<Ts> (ts)... })
    {
      static_assert (! metadata_v<Tag>.has_return (),
                     "Instruction metadata specified a return.");
    
      static_assert (metadata_v<Tag>.valid_num_args (sizeof...(Ts)),
                     "Instruction metadata specified a different number of arguments.");
    }
    
    metadata            m_metadata;
    optional_def_type   m_return;
    args_container_type m_args;
  };
  
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::phi>     = create_type<ir_opcode::phi,     true, arity::n_ary> ("phi");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::assign>  = create_type<ir_opcode::assign,  true, arity::unary> ("assign");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::call>    = create_type<ir_opcode::call,    true, arity::n_ary> ("call");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::fetch>   = create_type<ir_opcode::fetch,   true, arity::unary> ("fetch");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::convert> = create_type<ir_opcode::convert, true, arity::unary> ("convert");
  
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::branch>   = create_type<ir_opcode::branch, false, arity::n_ary> ("branch");    // abstract
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::cbranch>  = create_type<ir_opcode::cbranch,  ir_opcode::branch, arity::binary> ("br");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::ucbranch> = create_type<ir_opcode::ucbranch, ir_opcode::branch, arity::unary>  ("ubr");
  
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::relation> = create_type<ir_opcode::relation, true, arity::binary, true> ("relation");    // abstract
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::eq>       = create_type<ir_opcode::eq, ir_opcode::relation> ("==");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::ne>       = create_type<ir_opcode::ne, ir_opcode::relation> ("!=");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::lt>       = create_type<ir_opcode::lt, ir_opcode::relation> ("<");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::le>       = create_type<ir_opcode::le, ir_opcode::relation> ("<=");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::gt>       = create_type<ir_opcode::gt, ir_opcode::relation> (">");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::ge>       = create_type<ir_opcode::ge, ir_opcode::relation> (">=");
  
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::arithmetic> = create_type<ir_opcode::arithmetic, true, true> ("arithmetic");  // abstract
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::add>        = create_type<ir_opcode::add, ir_opcode::arithmetic, arity::binary> ("+");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::sub>        = create_type<ir_opcode::sub, ir_opcode::arithmetic, arity::binary> ("-");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::mul>        = create_type<ir_opcode::mul, ir_opcode::arithmetic, arity::binary> ("*");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::div>        = create_type<ir_opcode::div, ir_opcode::arithmetic, arity::binary> ("/");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::mod>        = create_type<ir_opcode::mod, ir_opcode::arithmetic, arity::binary> ("%");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::rem>        = create_type<ir_opcode::rem, ir_opcode::arithmetic, arity::binary> ("rem");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::neg>        = create_type<ir_opcode::neg, ir_opcode::arithmetic, arity::unary>  ("-");
  
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::logical> = create_type<ir_opcode::logical, true, true> ("logical");     // abstract
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::land>    = create_type<ir_opcode::land, ir_opcode::logical, arity::binary> ("&&");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::lor>     = create_type<ir_opcode::lor,  ir_opcode::logical, arity::binary> ("||");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::lnot>    = create_type<ir_opcode::lnot, ir_opcode::logical, arity::unary>  ("!");
  
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::bitwise>  = create_type<ir_opcode::bitwise, true, true> ("bitwise");     // abstract
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::band>     = create_type<ir_opcode::band,     ir_opcode::bitwise, arity::binary> ("&");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::bor>      = create_type<ir_opcode::bor,      ir_opcode::bitwise, arity::binary> ("|");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::bxor>     = create_type<ir_opcode::bxor,     ir_opcode::bitwise, arity::binary> ("^");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::bshiftl>  = create_type<ir_opcode::bshiftl,  ir_opcode::bitwise, arity::binary> ("<<");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::bashiftr> = create_type<ir_opcode::bashiftr, ir_opcode::bitwise, arity::binary> (">>");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::blshiftr> = create_type<ir_opcode::blshiftr, ir_opcode::bitwise, arity::binary> (">>");
  template <> inline constexpr auto ir_instruction2::metadata::instance<ir_opcode::bnot>     = create_type<ir_opcode::bnot,     ir_opcode::bitwise, arity::unary>  ("~");

  // imports
  class ir_visitor;

  template <typename>
  struct ir_printer;

  // base class for instructions
  // each instruction must have a parent block
  // instructions are NOT symbols and they are not available as operands
  // each instruction contains an argument list of type ir_operand
  class ir_instruction
  {
  public:

    using instr_iter      = instr_list::iterator;
    using instr_citer     = instr_list::const_iterator;
    using instr_riter     = instr_list::reverse_iterator;
    using instr_criter    = instr_list::const_reverse_iterator;
    using instr_ref       = instr_list::reference;
    using instr_cref      = instr_list::const_reference;

    using args_vect = std::vector<ir_operand>;
    using iter      = args_vect::iterator;
    using citer     = args_vect::const_iterator;
    using ref       = args_vect::reference;
    using cref      = args_vect::const_reference;

    ir_instruction            (void)                      = delete;
    ir_instruction            (const ir_instruction&)     = delete;
    ir_instruction            (ir_instruction&&) noexcept = delete;
    ir_instruction& operator= (const ir_instruction&)     = delete;
    ir_instruction& operator= (ir_instruction&&) noexcept = delete;
    virtual ~ir_instruction   (void)                      = default;

    explicit ir_instruction (ir_basic_block& blk)
      : m_block (blk)
    { }

    ir_instruction (ir_instruction&& other, ir_basic_block& blk)
      : m_block (blk),
        m_args  (std::move (other.m_args))
    { }

    //! Print the instruction.
    //! @param os an output stream
    //! @return the output stream
    // virtual std::ostream& print (std::ostream& os) const = 0;

    void set_block (ir_basic_block& blk) noexcept
    {
      m_block.emplace (blk);
    }

    [[nodiscard]]
    constexpr ir_basic_block& get_block (void) const noexcept
    {
      return *m_block;
    }

    // virtual void accept (ir_visitor& visitor) = 0;
  
    
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
    
    void set_args (args_vect&& args)
    {
      m_args = std::move (args);
    }

    // relink objects connected to contents of this instruction to
    // objects before the specified iterator.
    virtual void unlink_propagate (instr_citer) { };

    friend struct ir_printer<ir_instruction>;

  protected:

    template <typename ...Args>
    ir_operand& emplace_back (Args&&... args)
    {
      return m_args.emplace_back (std::forward<Args> (args)...);
    }

    template <typename ...Args>
    emplace_before (citer pos, Args&&... args)
    {
      return m_args.emplace (pos, std::forward<Args> (args)...);
    }

  private:

    nonnull_ptr<ir_basic_block> m_block;
    args_vect m_args;

  };

  class ir_def_instruction : public ir_instruction
  {
  public:
    ir_def_instruction (ir_basic_block& blk, ir_variable& ret_var,
                        ir_type ret_ty);
  
    ir_def_instruction (ir_basic_block& blk, ir_variable& ret_var)
      : ir_def_instruction (blk, ret_var, ir_type_v<void>)
    { }

    ~ir_def_instruction (void) override;

    void unlink_propagate (instr_citer pos) override;

    [[nodiscard]] constexpr       ir_def& get_def (void)       noexcept { return m_ret; }
    [[nodiscard]] constexpr const ir_def& get_def (void) const noexcept { return m_ret; }

  private:
    ir_def m_ret;
  };

  // assign some variable
  class ir_assign : public ir_def_instruction
  {
  public:

    // template <typename ...Args>
    // ir_assign (ir_basic_block& blk, ir_variable& ret_var,
    //            const ir_constant<Args...>& c)
    //   : ir_def_instruction (blk, ret_var, c.get_type ()),
    //     m_src (emplace_back<ir_constant<Args...>> (c))
    // { }
    //
    // template <typename ...Args>
    // ir_assign (ir_basic_block& blk, ir_variable& ret_var,
    //            ir_constant<Args...>&& c)
    //   : ir_def_instruction (blk, ret_var, c.get_type ()),
    //     m_src (emplace_back<ir_constant<Args...>> (std::forward<ir_constant<Args...>> (c)))
    // { }

    ir_assign (ir_basic_block& blk, ir_variable& dst, ir_def& src);

    [[nodiscard]] const ir_operand& get_assigner (void) const { return **m_src; }

  private:

    // make sure m_src doesnt change type!

    const iter m_src;

  };

  // call a function
  class ir_call : public ir_def_instruction
  {
    using arbitrary_function = void * (*) (void);
    
  public:
    ir_call            (void)               = default;
    ir_call            (const ir_call&)     = default;
    ir_call            (ir_call&&) noexcept = default;
    ir_call& operator= (const ir_call&)     = default;
    ir_call& operator= (ir_call&&) noexcept = default;
    ~ir_call           (void) override      = default;
  
    ir_call (ir_basic_block& blk, ir_variable& var)
      : ir_def_instruction (blk, var)
    { }
    
    // finds which function this call will be linked to
    void resolve (void);
  
    std::string        m_func_name;
    arbitrary_function m_func;
  };

  class ir_fetch : public ir_def_instruction
  {

    // fetch a variable with a certain name
    // this is used if the variable is not defined in a code path

  public:
    ir_fetch (ir_basic_block& blk, ir_variable& ret_var);

  private:
    const iter m_name;
  };

  // unconditional
  class ir_branch : public ir_instruction
  {
  public:
    ir_branch (ir_basic_block& blk, ir_basic_block& dst);

  private:
    const iter m_dest_block;
  };

  class ir_cbranch : public ir_instruction
  {
  public:
    ir_cbranch (ir_basic_block& blk, ir_def& d,
                ir_basic_block& tblk, ir_basic_block& fblk);

  private:
    const iter m_condvar;
    const iter m_tblock;
    const iter m_fblock;
  };

  class ir_convert : public ir_def_instruction
  {

  public:

    ir_convert (ir_basic_block& blk, ir_type ty, ir_def& d);

    ir_convert (ir_basic_block& blk, ir_variable& ret_var, ir_type ty,
                ir_def& d);

  private:
    const iter m_src;
  };

  class ir_phi : public ir_def_instruction
  {
  public:
    
    ir_phi            (void)              = delete;
    ir_phi            (const ir_phi&)     = delete;
    ir_phi            (ir_phi&&) noexcept = delete;
    ir_phi& operator= (const ir_phi&)     = delete;
    ir_phi& operator= (ir_phi&&) noexcept = delete;
    ~ir_phi           (void) override     = default;
    
    ir_phi (ir_basic_block& blk, ir_variable& var)
      : ir_def_instruction (blk, var)
    { }

    template <typename It>
    ir_phi (ir_basic_block& blk, ir_variable& var, ir_type ty)
      : ir_def_instruction (blk, var, ty)
    { }

    void append_incoming (ir_basic_block& blk, ir_use_timeline& ut)
    {
      emplace_back (ir_type_v<ir_basic_block *>, &blk);
      emplace_back (ut, *this);
    }
    
    void append_indet (ir_basic_block& blk)
    {
      m_indets.emplace_back (blk);
    }
    
    void set_indets (std::vector<nonnull_ptr<ir_basic_block>>&& indets)
    {
      m_indets = std::move (indets);
    }

    iter erase (const ir_basic_block& blk);
    
    [[nodiscard]]  iter find (const ir_basic_block& blk);
    [[nodiscard]] citer find (const ir_basic_block& blk) const;
    
    optional_ref<ir_use> retrieve_use (const ir_basic_block& blk);
    optional_ref<ir_def> retrieve_def (const ir_basic_block& blk);
  
    [[nodiscard]] auto  indet_begin   (void)       noexcept { return m_indets.begin ();   }
    [[nodiscard]] auto  indet_begin   (void) const noexcept { return m_indets.begin ();   }
    [[nodiscard]] auto  indet_cbegin  (void) const noexcept { return m_indets.cbegin ();  }
  
    [[nodiscard]] auto  indet_end     (void)       noexcept { return m_indets.end ();     }
    [[nodiscard]] auto  indet_end     (void) const noexcept { return m_indets.end ();     }
    [[nodiscard]] auto  indet_cend    (void) const noexcept { return m_indets.cend ();    }
  
    [[nodiscard]] auto  indet_rbegin  (void)       noexcept { return m_indets.rbegin ();  }
    [[nodiscard]] auto  indet_rbegin  (void) const noexcept { return m_indets.rbegin ();  }
    [[nodiscard]] auto  indet_crbegin (void) const noexcept { return m_indets.crbegin (); }
  
    [[nodiscard]] auto  indet_rend    (void)       noexcept { return m_indets.rend ();    }
    [[nodiscard]] auto  indet_rend    (void) const noexcept { return m_indets.rend ();    }
    [[nodiscard]] auto  indet_crend   (void) const noexcept { return m_indets.crend ();   }
  
    [[nodiscard]] auto& indet_front   (void)       noexcept { return m_indets.front ();   }
    [[nodiscard]] auto& indet_front   (void) const noexcept { return m_indets.front ();   }
  
    [[nodiscard]] auto& indet_back    (void)       noexcept { return m_indets.back ();    }
    [[nodiscard]] auto& indet_back    (void) const noexcept { return m_indets.back ();    }
  
    [[nodiscard]] std::size_t num_indet (void) const noexcept { return m_indets.size (); }
    
    [[nodiscard]] bool has_indet (void) const noexcept { return m_indets.empty (); }
    
  private:

    // blocks where the variable was undefined
    std::vector<nonnull_ptr<ir_basic_block>> m_indets;
  };

}

#endif

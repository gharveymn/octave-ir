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
#include "ir-type-base.hpp"
#include "ir-instruction-fwd.hpp"
#include <unordered_map>
#include <list>
#include <vector>

#include <memory>
#include <utility>

namespace gch
{

  // imports
  class ir_visitor;

  template <typename>
  struct ir_printer;

  using ir_block_ref = ir_constant<ir_basic_block *>;

  template <>
  struct ir_type::instance<ir_block_ref>
  {
    using type = ir_block_ref;
    static constexpr
    impl m_impl = create_type<type> ("ir_block_ref");
  };

  using ir_def_ref = ir_constant<ir_def *>;

  template <>
  struct ir_type::instance<ir_def_ref>
  {
    using type = ir_def_ref;
    static constexpr
    impl m_impl = create_type<type> ("ir_def_ref");
  };

  using ir_phi_arg = ir_constant<ir_block_ref, ir_def_ref>;

  template <>
  struct ir_type::instance<ir_phi_arg::value_type>
  {
    using type = ir_phi_arg::value_type;
    static constexpr ir_type m_members[]
    {
      get<ir_block_ref> (),
      get<ir_def_ref> (),
    };

    static_assert (ir_type_array (m_members).get_size () == sizeof (type),
                   "Type size is not equal to its IR counterpart.");
    static constexpr
    impl m_impl = create_compound_type<type> ("phi_pair", m_members);
  };

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

    // using vector instead of a map because we shouldn't be accessing
    // elements very often (hence this is optimized for memory usage).
    using base_arg_type = std::unique_ptr<ir_operand>;
    using args_vect = std::vector<base_arg_type>;
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
  
    [[nodiscard]] iter  begin (void)       { return m_args.begin (); }
    [[nodiscard]] citer begin (void) const { return m_args.begin (); }
    [[nodiscard]] iter  end   (void)       { return m_args.end (); }
    [[nodiscard]] citer end   (void) const { return m_args.end (); }
    
    iter erase (citer pos);
    
    void set_args (args_vect&& args)
    {
      m_args = std::move (args);
    }

    // relink objects connected to contents of this instruction to
    // objects before the specified iterator.
    virtual void unlink_propagate (instr_citer) { };

    friend struct ir_printer<ir_instruction>;

  protected:

    template <typename T, typename ...Args>
    std::enable_if_t<std::is_base_of<ir_operand, T>::value, iter>
    emplace_back (Args&&... args)
    {
      return emplace_before<T> (m_args.end (), std::forward<Args> (args)...);
    }

    template <typename T, typename ...Args>
    std::enable_if_t<std::is_base_of<ir_operand, T>::value, iter>
    emplace_before (citer pos, Args&&... args)
    {
      return m_args.emplace (pos, std::make_unique<T> (std::forward<Args> (args)...));
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

    template <typename ...Args>
    ir_assign (ir_basic_block& blk, ir_variable& ret_var,
               const ir_constant<Args...>& c)
      : ir_def_instruction (blk, ret_var, c.get_type ()),
        m_src (emplace_back<ir_constant<Args...>> (c))
    { }

    template <typename ...Args>
    ir_assign (ir_basic_block& blk, ir_variable& ret_var,
               ir_constant<Args...>&& c)
      : ir_def_instruction (blk, ret_var, c.get_type ()),
        m_src (emplace_back<ir_constant<Args...>> (std::forward<ir_constant<Args...>> (c)))
    { }

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
  
    std::string             m_func_name;
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
    ir_phi            (const ir_phi&)     = default;
    ir_phi            (ir_phi&&) noexcept = default;
    ir_phi& operator= (const ir_phi&)     = default;
    ir_phi& operator= (ir_phi&&) noexcept = default;
    ~ir_phi           (void) override     = default;
    
    ir_phi (ir_basic_block& blk, ir_variable& var)
      : ir_def_instruction (blk, var)
    { }

    template <typename It>
    ir_phi (ir_basic_block& blk, ir_variable& var, ir_type ty, It first,
            It last)
      : ir_def_instruction (blk, var, ty)
    {
      std::for_each (first, last,
        [this] (const std::pair<ir_basic_block&, ir_def *>& p)
        {
          append (&p.first, p.second);
        });
    }

    void append (ir_basic_block& blk, ir_use_timeline& ut)
    {
      emplace_back (blk, { ut, *this });
    }

    iter erase (const ir_basic_block *blk);

    ir_def * find (const ir_basic_block *blk);

    bool has_indeterminate_preds (void) { return ! m_indet_preds.empty (); }

    const std::vector<ir_basic_block *>& get_undefined_preds (void)
    {
      return m_indet_preds;
    }

  private:

    // blocks where the variable was undefined
    std::vector<ir_basic_block *> m_indet_preds;
  };

  class ir_relation : public ir_instruction
  {
  public:
    ~ir_relation () override = 0;
  };

  class ir_rel_eq : public ir_relation
  {

  };

  class ir_rel_ne : public ir_relation
  {

  };

  class ir_rel_lt : public ir_relation
  {

  };

  class ir_rel_le : public ir_relation
  {

  };

  class ir_rel_gt : public ir_relation
  {

  };

  class ir_rel_ge : public ir_relation
  {

  };

  class ir_arithmetic
  {
  public:
    virtual ~ir_arithmetic () = 0;
  };

  class ir_unary_op
  {
  public:
    virtual ~ir_unary_op () = 0;
  private:

  };

  class ir_arith_add : public ir_arithmetic
  {

  };

  class ir_arith_sub : public ir_arithmetic
  {

  };

  class ir_arith_mul : public ir_arithmetic
  {

  };

  class ir_arith_div : public ir_arithmetic
  {

  };


}

#endif

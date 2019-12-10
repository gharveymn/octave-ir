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

#include "octave-config.h"

#include "ir-common.h"
#include "ir-variable.h"
#include "ir-type-base.h"
#include "ir-instruction-fwd.h"
#include <unordered_map>
#include <list>
#include <vector>

#include <memory>
#include <utility>

namespace octave
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
    using arg_list = std::vector<base_arg_type>;
    using iter = arg_list::iterator;
    using citer = arg_list::const_iterator;
    using ref = arg_list::reference;
    using cref = arg_list::const_reference;

    explicit ir_instruction (ir_basic_block& blk)
      : m_block (blk)
    { }

    virtual ~ir_instruction (void) = default;

    virtual bool infer (void) { return false; }

    //! Print the instruction.
    //! @param os an output stream
    //! @return the output stream
    // virtual std::ostream& print (std::ostream& os) const = 0;

    constexpr ir_basic_block& get_block (void) const noexcept
    {
      return m_block;
    }

    // virtual void accept (ir_visitor& visitor) = 0;

    iter  begin (void)       { return m_args.begin (); }
    citer begin (void) const { return m_args.begin (); }
    iter  end   (void)       { return m_args.end (); }
    citer end   (void) const { return m_args.end (); }

    iter erase (citer pos);

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
      return m_args.insert (pos,
                            std::make_unique<T> (std::forward<Args> (args)...));
    }

  private:

    ir_basic_block& m_block;
    arg_list m_args;

  };

  class ir_def_instruction : public ir_instruction
  {
  public:
    ir_def_instruction (ir_basic_block& blk, ir_variable& ret_var,
                        ir_type ret_ty);

    ~ir_def_instruction (void) override;

    void unlink_propagate (instr_citer pos) override;

    constexpr const ir_def& get_return (void) const noexcept { return m_ret; }

    ir_def& get_return (void) noexcept { return m_ret; }

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

    ir_assign (ir_basic_block& blk, ir_variable& var, ir_def& src);

    const ir_operand& get_assignor (void) const { return **m_src; }

  private:

    // make sure m_src doesnt change type!

    const iter m_src;

  };

  // call a function
  class ir_call : public ir_instruction
  {

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

    ir_convert (ir_basic_block& blk, ir_variable& ret_var, ir_type ty,
                ir_def& d);

  private:
    const iter m_src;
  };

  class ir_phi : public ir_def_instruction
  {
  public:

    ir_phi (void) = delete;

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

    void append (ir_basic_block *blk, ir_def *dptr);

    iter erase (const ir_basic_block *blk);

    ir_def * find (const ir_basic_block *blk);

    bool has_undefined_blocks (void) { return ! m_undef_blocks.empty (); }

    const std::vector<ir_basic_block *>& get_undefined_blocks (void)
    {
      return m_undef_blocks;
    }

  private:

    // blocks where the variable was undefined
    std::vector<ir_basic_block *> m_undef_blocks;
  };

  class ir_relation : public ir_instruction
  {
  public:
    virtual ~ir_relation () = 0;
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

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
#include <unordered_map>
#include <vector>

namespace octave
{

  // defined here
  class ir_instruction;
    class ir_assign; // assign some variable (x = y)
    class ir_call;   // call a function
    class ir_define; // define a variable (used on entry)
    class ir_operator;
      class ir_unop;
      class ir_biop;
    class ir_phi;
    class ir_relation;
      class ir_rel_eq;
      class ir_rel_ne;
      class ir_rel_lt;
      class ir_rel_le;
      class ir_rel_gt;
      class ir_rel_ge;
    class ir_operation;
      class ir_biop;
        class ir_add;

  // imports
  class ir_visitor;
  class ir_basic_block;

  template <typename>
  struct ir_printer;
  
  using ir_block_ref = ir_constant<const ir_basic_block&>;
  
  template <>
  struct ir_type::instance<ir_block_ref>
  {
    using type = ir_block_ref;
    static constexpr
    impl m_impl = create_type<type> ("ir_block_ref");
  };
  
  using ir_phi_arg = ir_constant<ir_block_ref, ir_variable::use>;
  
  template <>
  struct ir_type::instance<ir_phi_arg::value_type>
  {
    using type = ir_phi_arg::value_type;
    static constexpr ir_type m_members[]
      {
        get<ir_block_ref> (),
        get<ir_variable::use> (),
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

    using use = ir_variable::use;
    using def = ir_variable::def;
    
    // using vector instead of a map because we shouldn't be accessing
    // elements very often (hence this is optimized for memory usage).
    using base_arg_type = std::unique_ptr<ir_operand>;
    using arg_list = std::vector<base_arg_type>;
    using iter = arg_list::iterator;
    using citer = arg_list::const_iterator;
    using ref = arg_list::reference;
    using cref = arg_list::const_reference;

    explicit ir_instruction (const ir_basic_block& blk);

    virtual ~ir_instruction (void) = default;

    virtual bool infer (void) { return false; }

    //! Print the instruction.
    //! @param os an output stream
    //! @return the output stream
    // virtual std::ostream& print (std::ostream& os) const = 0;

    const ir_basic_block& get_block (void) const
    {
      return m_block;
    }

    // virtual void accept (ir_visitor& visitor) = 0;
    
    virtual bool has_return (void) { return false; };

    virtual def& get_return (void);
  
    iter begin (void) { return m_args.begin (); }
    citer begin (void) const { return m_args.begin (); }
    iter end (void) { return m_args.end (); }
    citer end (void) const { return m_args.end (); }
    
    iter erase (citer pos);

    friend struct ir_printer<ir_instruction>;
    
  protected:
  
    template <typename T, typename ...Args>
    enable_if_t<std::is_base_of<ir_operand, T>::value, const T> *
    emplace_back (Args&&... args);
  
    template <typename T, typename ...Args>
    enable_if_t<std::is_base_of<ir_operand, T>::value, citer>
    emplace_before (citer pos, Args&&... args);

  private:

    const ir_basic_block& m_block;
    arg_list m_args;

  };
  
  class ir_def_instruction : public ir_instruction
  {
  public:
    ir_def_instruction (const ir_basic_block& blk, ir_variable& var,
                             ir_type ty);
    
    ~ir_def_instruction (void) override;
    
    bool has_return (void) override { return true; };
  
    def& get_return (void) override;
    
  private:
    def m_ret;
  };

  // assign some variable the value of another variable
  class ir_assign : public ir_def_instruction
  {
  public:
  private:
  };

  // call a function
  class ir_call : public ir_instruction
  {

  };

  // define existence of a variable
  class ir_define : public ir_instruction
  {

  };

  class ir_fetch : public ir_def_instruction
  {

    // fetch a variable with a certain name
    // this is used if the variable is not defined in a code path

  public:
    ir_fetch (const ir_basic_block& blk, ir_variable& var);

  private:
    const ir_constant<std::string> *m_name;
  };

  // unconditional
  class ir_branch : public ir_instruction
  {
  public:
    ir_branch (const ir_basic_block& blk, const ir_basic_block& dst);
    
  private:
    const ir_block_ref * m_dest_block;
  };
  
  class ir_cbranch : public ir_instruction
  {
    const use *m_condvar;
    const ir_block_ref *m_tblock;
    const ir_block_ref *m_fblock;
  };

  class ir_convert : public ir_def_instruction
  {

  public:

    ir_convert (const ir_basic_block& blk, ir_variable& ret_var, ir_type ty,
                def& d);
    
  private:
    const use *m_use;
  };

  class ir_phi : public ir_def_instruction
  {
  public:
    using arg_type = ir_constant<ir_block_ref, ir_variable::use>;

    using input_type = ir_variable::block_def_pair;
    using input_vec = ir_variable::block_def_vec;

    ir_phi (void) = delete;

    ir_phi (const ir_basic_block& blk, ir_variable& var, ir_type ty,
            const input_vec& pairs);

    void append (const ir_basic_block& blk, ir_variable::def& d);

    iter erase (const ir_basic_block *blk);

    const ir_variable::use& find (const ir_basic_block *blk);

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

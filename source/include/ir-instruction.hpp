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

    // using vector instead of a map because we shouldn't be accessing
    // elements very often (hence this is optimized for memory usage).
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

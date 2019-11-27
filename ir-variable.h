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

#if ! defined (octave_ir_variable_h)
#define octave_ir_variable_h 1

#include "octave-config.h"
#include "ir-common-util.h"
#include "ir-operand.h"
#include "ir-type-base.h"
#include "ir-block.h"
#include "observer.h"

#include <deque>
#include <memory>
#include <unordered_set>
#include <list>
#include <vector>

namespace octave
{
  class ir_instruction;
  class ir_def_instruction;
  class ir_basic_block;
  class ir_function;
  class ir_basic_block;

  class ir_def;
  class ir_use;

  using instr_list_type = std::unique_ptr<ir_instruction>;
  using instr_list      = std::list<std::unique_ptr<ir_instruction>>;
  using instr_iter      = instr_list::iterator;
  using instr_citer     = instr_list::const_iterator;
  using instr_riter     = instr_list::reverse_iterator;
  using instr_criter    = instr_list::const_reverse_iterator;
  using instr_ref       = instr_list::reference;
  using instr_cref      = instr_list::const_reference;

  using def_list = std::list<ir_def *>;
  using def_iter = def_list::iterator;
  using def_citer = def_list::const_iterator;
  using def_ref = def_list::reference;
  using def_cref = def_list::const_reference;

  using use_list = std::list<ir_use *>;
  using use_iter = use_list::iterator;
  using use_citer = use_list::const_iterator;
  using use_ref = use_list::reference;
  using use_cref = use_list::const_reference;

  class ir_variable
  {

    using block_def_pair = std::pair<ir_basic_block&, ir_def *>;
    using block_def_vect = std::vector<block_def_pair>;

  public:

    ir_def create_def (ir_type ty, const ir_def_instruction& instr);

    static constexpr const char anonymous_name[] = "<@>";

    // variables own both defs and uses
    // defs and uses may allow references elsewhere, but these singular
    // and may not be copied or addressed (may be moved).

    ir_variable (ir_function& m, std::string name);

    ir_variable (void)                          = delete;

    ir_variable (const ir_variable&)            = delete;
    ir_variable& operator= (const ir_variable&) = delete;

    // can't allow movement because others rely on a pointer
    ir_variable (ir_variable&& o)               = delete;
    ir_variable& operator= (ir_variable&&)      = delete;

    ~ir_variable (void) noexcept;

    // has side-effects
    static ir_type normalize_types (block_def_vect& pairs);

    // variables create defs and hold a pointer to that ir_def
    // these pointers must be adjusted when moving or deleting.

    std::size_t get_num_defs (void) const noexcept
    {
      return m_def_observer.num_children ();
    }

    constexpr const std::string& get_name (void) const { return m_name; }

    constexpr ir_function& get_module (void) const { return m_function; }

    bool has_sentinel (void) const noexcept
    {
      return m_sentinel != nullptr;
    }

    ir_variable& get_sentinel (void);

    std::string get_sentinel_name (void) const;

    void mark_uninit (ir_basic_block& blk);

    constexpr const ir_function& get_function (void) const noexcept
    {
      return m_function;
    }

    ir_function& get_function (void) noexcept
    {
      return m_function;
    }

    ir_def& join (ir_basic_block& blk);

    ir_def& join (ir_basic_block& blk, instr_citer pos);

   private:

    void initialize_sentinel (void);

    def_citer find_def (const ir_def *d) const;

    ir_function& m_function;

    //! The variable name. The default is a synonym for 'anonymous'.
    std::string m_name = anonymous_name;

    observer<ir_def, ir_variable> m_def_observer;

    // Used to indicate if the variable is uninitialized in the current
    // code path. Lazily initialized.
    std::unique_ptr<ir_variable> m_sentinel;

  };

  //! An ssa variable def. It holds very little information about itself,
  //! it's more of just an indicating stub for the variable.
  class ir_def : public observee<ir_def, ir_variable>
  {

  public:

    ir_def (observee::observer_type& var, ir_type ty,
             const ir_def_instruction& instr);

    ir_def (void)                     = delete;

    ir_def (const ir_def&)            = delete;
    ir_def (ir_def&& d) noexcept;

    ir_def& operator= (const ir_def&) = delete;
    ir_def& operator= (ir_def&&)      = delete;

    //! Get the def. Don't use this within the scope of ir_variable. The
    //! def pointer is only nullptr when it is in an invalid state. It
    //! should be set to a phony def if it has no real parent.
    //! @return the parent def
    ir_variable& get_var (void) const noexcept (false);

    ir_use create_use (const ir_instruction& instr);

    template <typename InputIt>
    static ir_type common_type (InputIt first, InputIt last);

    constexpr const ir_def_instruction& get_instruction (void) const
    {
      return m_instr;
    }

    constexpr ir_basic_block& get_block (void) const;

    std::ostream& print (std::ostream& os) const;

    std::string get_name (void) const;

    std::size_t get_id (void) const;

    bool has_uses (void) const noexcept
    {
      return ! m_use_tracker.has_children ();
    }

    constexpr ir_type get_type (void) const
    {
      return m_type;
    }

    void set_needs_init_check (bool state) noexcept
    {
      m_needs_init_check = state;
    };

    bool needs_init_check (void) const noexcept
    {
      return m_needs_init_check;
    }

    friend ir_def ir_variable::create_def (ir_type ty, const ir_def_instruction& instr);

  private:

    observer<ir_use, ir_def> m_use_tracker;

    const ir_type m_type;

    // where this ir_def occurs
    const ir_def_instruction& m_instr;

    bool m_needs_init_check;

  };

  class ir_use : public ir_operand, public observee<ir_use, ir_def>
  {
  public:

    // uses can only be explicitly created by defs
    // They may not be copied, but they may be moved.
    // defs maintain a pointer to each use it created

    //! Create a use with a parent def.
    ir_use (observee::observer_type& obs, const ir_instruction& instr);

    constexpr ir_use (void)           = delete;

    ir_use (const ir_use&)            = delete;
    ir_use& operator= (const ir_use&) = delete;

    ir_use (ir_use&& u) noexcept;
    ir_use& operator= (ir_use&&)      = delete;

    //! Get the def. Don't use this within the scope of ir_variable. The
    //! def pointer is only nullptr when it is in an invalid state. It
    //! should be set to a phony def if it has no real parent.
    //! @return the parent def
    ir_def& get_def (void) const noexcept (false);

    std::string get_name (void) const;

    std::size_t get_id (void);

    ir_type get_type (void) const override;

    friend ir_use ir_def::create_use (const ir_instruction& instr);

  private:

    //! Where this use occurs.
    const ir_instruction *m_instr;

  };

  template <>
  struct ir_type::instance<ir_def *>
  {
    using type = ir_def;
    static constexpr
    impl m_impl = create_type<type> ("ir_def *");
  };

  template <>
  struct ir_type::instance<ir_use *>
  {
    using type = ir_use;
    static constexpr
    impl m_impl = create_type<type> ("ir_use *");
  };

}

#endif

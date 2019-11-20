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
#include "ir-common.h"
#include "ir-common-util.h"
#include "ir-operand.h"
#include "ir-type-base.h"

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

  class ir_variable
  {

  public:

    class def;
    class use;

    using def_list = std::list<def *>;
    using def_iter = def_list::iterator;
    using def_citer = def_list::const_iterator;
    using def_ref = def_list::reference;
    using def_cref = def_list::const_reference;

    using use_set = std::list<use *>;
    using use_iter = use_set::iterator;
    using use_citer = use_set::const_iterator;
    using use_ref = use_set::reference;
    using use_cref = use_set::const_reference;

    using block_def_pair = std::pair<ir_basic_block&, def *>;
    using block_def_vec  = std::vector<block_def_pair>;

    def create_def (ir_type ty, const ir_def_instruction& instr);

    //! An ssa variable def. It holds very little information about itself,
    //! it's more of just an indicating stub for the variable.
    class def
    {
    public:

      def (void)                  = delete;

      def (const def&)            = delete;
      def& operator= (const def&) = delete;

      def (def&& d) noexcept;
      def& operator= (def&&)      = delete;

      ~def (void) noexcept;

      void invalidate (void) noexcept { m_var = nullptr; }

      //! Get the def. Don't use this within the scope of ir_variable. The
      //! def pointer is only nullptr when it is in an invalid state. It
      //! should be set to a phony def if it has no real parent.
      //! @return the parent def
      ir_variable& get_var (void) const noexcept (false);

      use create_use (const ir_instruction& instr);

      use_iter track_use (use *u);

      void untrack_use (use_citer cit);

      template <typename InputIt>
      static ir_type common_type (InputIt first, InputIt last);

      use_iter begin (void) noexcept { return m_uses.begin (); }
      use_citer begin (void) const noexcept { return m_uses.begin (); }
      use_iter end (void) noexcept { return m_uses.end (); }
      use_citer end (void) const noexcept { return m_uses.end (); }

      constexpr const ir_def_instruction&
      get_instruction (void) const
      {
        return m_instr;
      }

      std::ostream& print (std::ostream& os) const;

      std::string get_name (void) const;

      std::size_t get_id (void) const;

      constexpr bool has_var (void) const noexcept
      {
        return m_var != nullptr;
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

      friend def
      ir_variable::create_def (ir_type ty, const ir_def_instruction& instr);

    private:

      def (ir_variable& var, ir_type ty, const ir_def_instruction& instr);

      ir_variable *m_var;

      def_iter m_self_iter;

      const ir_type m_type;

      // where this def occurs
      const ir_def_instruction& m_instr;

      // all uses which can be reached from this def
      use_set m_uses;

      bool m_needs_init_check;

    };

    class use : public ir_operand
    {
    public:

      // uses can only be explicitly created by defs
      // They may not be copied, but they may be moved.
      // defs maintain a pointer to each use it created

      constexpr use (void)        = delete;

      use (const use&)            = delete;
      use& operator= (const use&) = delete;

      use (use&& u) noexcept;
      use& operator= (use&&)      = delete;

      ~use (void) noexcept override;

      void invalidate (void) noexcept { m_def = nullptr; }

      //! Replace the parent def for this use.
      //! @param new_def the replacement
      //! @return a reference to this use
      void replace_def (def& new_def) noexcept (false);

      //! Get the def. Don't use this within the scope of ir_variable. The
      //! def pointer is only nullptr when it is in an invalid state. It
      //! should be set to a phony def if it has no real parent.
      //! @return the parent def
      def& get_def (void) const noexcept (false);

      std::string get_name (void) const;

      std::size_t get_id (void);

      constexpr bool
      has_def (void) const noexcept
      {
        return m_def != nullptr;
      }

      ir_type get_type (void) const override;

      friend use ir_variable::def::create_use (const ir_instruction& instr);

    private:

      //! Create a use with a parent def.
      use (def& d, const ir_instruction& instr);

      //! The def for this use
      def *m_def;

      // iterator for position in def list
      use_iter m_self_iter;

      //! Where this use occurs.
      const ir_instruction *m_instr;

    };

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

    def_iter track_def (def *u)
    {
      return m_defs.insert (m_defs.end (), u);
    }

    void untrack_def (def_citer cit) noexcept
    {
      m_defs.erase (cit);
    }

    // has side-effects
    static ir_type normalize_types (block_def_vec& pairs);

    // variables create defs and hold a pointer to that def
    // these pointers must be adjusted when moving or deleting.

    std::size_t get_num_defs (void) const noexcept { return m_defs.size (); }

    def_iter begin (void) noexcept { return m_defs.begin (); }
    def_citer begin (void) const noexcept { return m_defs.begin (); }
    def_iter end (void) noexcept { return m_defs.end (); }
    def_citer end (void) const noexcept { return m_defs.end (); }

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

   private:
    
    void initialize_sentinel (void);

    def_citer find_def (const def *d) const;

    ir_function& m_function;

    //! The variable name. The default is a synonym for 'anonymous'.
    std::string m_name = anonymous_name;

    //! A bag of all ssa defs.
    def_list m_defs;

    // Used to indicate if the variable is uninitialized in the current
    // code path. Lazily initialized.
    std::unique_ptr<ir_variable> m_sentinel;

  };

  template <>
  struct ir_type::instance<ir_variable::def>
  {
    using type = ir_variable::def;
    static constexpr
    impl m_impl = create_type<type> ("def");
  };

  template <>
  struct ir_type::instance<ir_variable::use>
  {
    using type = ir_variable::use;
    static constexpr
    impl m_impl = create_type<type> ("use");
  };
}

#endif

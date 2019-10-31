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
#include <vector>

namespace octave
{

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

    //! An ssa variable def. It holds very little information about itself,
    //! it's more of just an indicating stub for the variable.
    class def
    {
    public:

      def (ir_variable& var, ir_type ty, const ir_instruction& instr);

      def (void)                  = delete;

      def (const def&)            = delete;
      def& operator= (const def&) = delete;

      def (def&& d);
      def& operator= (def&&)      = delete;

      ~def (void) noexcept
      {
        orphan ();

        for (use *u : m_uses)
          u->orphan ();
      }

      void orphan (void) noexcept
      {
        if (m_var != nullptr)
          m_var->untrack_def (m_self_iter);
        invalidate ();
      }
      
      void invalidate (void) noexcept { m_var = nullptr; }

      void replace_var (ir_variable& v) noexcept (false)
      {
        def_iter it = v.track_def (this);
        orphan ();
        m_var = &v;
        m_self_iter = it;
      }

      //! Get the def. Don't use this within the scope of ir_variable. The
      //! def pointer is only nullptr when it is in an invalid state. It
      //! should be set to a phony def if it has no real parent.
      //! @return the parent def
      ir_variable& get_var (void) const noexcept (false)
      {
        if (m_var == nullptr)
          throw ir_exception ("Pointer to variable was nullptr.");
        return *m_var;
      }

      use create_use (const ir_instruction& instr);

      use_iter track_use (use *u)
      {
        return m_uses.insert (m_uses.end (), u);
      }
      
      void untrack_use (use_citer cit)
      {
        m_uses.erase (cit);
      }
      
      template <typename InputIt>
      static ir_type common_type (InputIt first, InputIt last);
  
      use_iter begin (void) noexcept { return m_uses.begin (); }
      use_citer begin (void) const noexcept { return m_uses.begin (); }
      use_iter end (void) noexcept { return m_uses.end (); }
      use_citer end (void) const noexcept { return m_uses.end (); }

      //! if this is not a real def the instruction will be a certain type.
      constexpr bool is_virtual (void) const;

      const ir_instruction* get_instruction (void) const { return &m_instr; }

      std::ostream& print (std::ostream& os) const;

      std::string get_name (void) const;

      std::size_t get_id (void) const;
      
      constexpr ir_type get_type (void) const { return m_type; }

    private:

      ir_variable *m_var;
      
      def_iter m_self_iter;
  
      ir_type m_type;

      // where this def occurs
      const ir_instruction& m_instr;

      // all uses which can be reached from this def
      use_set m_uses;

    };

    class use : public ir_operand
    {
    public:

      //! Create a use with a parent def.
      use (def& d, const ir_instruction& instr);

      // uses can only be explicitly created by defs
      // They may not be copied, but they may be moved.
      // defs maintain a pointer to each use it created

      constexpr use (void)        = delete;

      use (const use&)            = delete;
      use& operator= (const use&) = delete;

      use (use&& u) noexcept;
      use& operator= (use&&)      = delete;

      ~use (void) noexcept override
      {
        orphan ();
      }

      void orphan (void) noexcept
      {
        if (m_def != nullptr)
          m_def->untrack_use (m_self_iter);
        invalidate ();
      }
  
      void invalidate (void) noexcept { m_def = nullptr; }

      //! Replace the parent def for this use.
      //! @param d the replacement
      //! @return a reference to this use
      void replace_parent (def& d) noexcept (false)
      {
        use_iter it = d.track_use (this);
        orphan ();
        m_def = &d;
        m_self_iter = it;
      }

      //! Get the def. Don't use this within the scope of ir_variable. The
      //! def pointer is only nullptr when it is in an invalid state. It
      //! should be set to a phony def if it has no real parent.
      //! @return the parent def
      def& get_parent (void) const noexcept (false)
      {
        if (m_def == nullptr)
          throw ir_exception ("Pointer to def was nullptr.");
        return *m_def;
      }

      std::ostream& print (std::ostream& os) const override;

      std::string get_name (void) const;

      std::size_t get_id (void);

    private:

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

    ir_variable (void) = default;

    ~ir_variable (void) noexcept
    {
      // Remove references to this in the tracked defs and uses
      for (def *d : m_defs)
        d->orphan ();
    }

    explicit ir_variable (std::string name)
      : m_name (std::move (name))
    { }
  
    def create_def (ir_type ty, const ir_instruction& instr);

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

    void mark_uninitialized (ir_basic_block& blk);

   private:

    def_citer find_def (const def *d) const;

    //! The variable name. The default is a synonym for 'anonymous'.
    std::string m_name = anonymous_name;

    //! A bag of all ssa defs.
    def_list m_defs;

    // Used to indicate if the variable is uninitialized in the current
    // code path. Lazily initialized.
    std::unique_ptr<ir_variable> m_uninit_sentinel;

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

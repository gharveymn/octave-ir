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

#if !defined(ir_value_h)
#define ir_value_h 1

#include "octave-config.h"

#include "ir-common-util.h"

#include <unordered_set>

namespace octave
{
  
  class ir_instruction;

  class ir_def;
  class ir_use;

  using def_set = std::unordered_set<ir_def *, ptr_noexcept_hash<ir_def>,
  ptr_noexcept_equal_to<ir_def>>;
  using def_iter = def_set::iterator;
  using def_citer = def_set::const_iterator;
  using def_ref = def_set::reference;
  using def_cref = def_set::const_reference;

  using use_set = std::unordered_set<ir_use *, ptr_noexcept_hash<ir_use>,
  ptr_noexcept_equal_to<ir_use>>;
  using use_iter = use_set::iterator;
  using use_citer = use_set::const_iterator;
  using use_ref = use_set::reference;
  using use_cref = use_set::const_reference;

  //! An ssa variable def. It holds very little information about itself,
  //! it's more of just an indicating stub for the variable.
  class ir_def
  {
  public:

    // We use the same idiom as above.

    ir_def (void)                  = delete;

    ir_def (const ir_def&)            = delete;
    ir_def& operator= (const ir_def&) = delete;

    ir_def (ir_def&& d) noexcept      = default; // <-
    ir_def& operator= (ir_def&&)      = delete;

    ~ir_def (void) noexcept
    {
      abandon_children ();
    }

    std::unique_ptr<ir_use> create_use (const ir_instruction& instr);

    void track_use (ir_use *u)
    {
      m_uses.insert (u);
    }

    void untrack_use (ir_use *u) noexcept
    {
      m_uses.erase (u);
    }

    //! :(
    void abandon_children (void) noexcept;

    use_citer begin (void) const noexcept { return m_uses.begin (); }
    use_citer end (void) const noexcept { return m_uses.end (); }

    //! if this is not a real def the instruction will be a certain type.
    constexpr bool is_virtual (void) const;

    ir_instruction *get_instruction (void) const;

    std::ostream& print_use (std::ostream& os) const;

  private:

    // explicit creation only may occur from the variable parent
    ir_def (ir_type ty, const ir_instruction& instr);

    use_citer find_use (use_cref u) const { return m_uses.find (u); }

    // where this ir_def occurs
    const ir_instruction * m_instr;

    // all uses which can be reached from this ir_def
    use_set m_uses;

  };

  class ir_use
  {
  public:

    // uses can only be explicitly created by defs
    // They may not be copied, but they may be moved.
    // defs maintain a pointer to each use it created

    constexpr ir_use (void)           = delete;

    constexpr ir_use (const ir_use&)     = delete;
    ir_use& operator= (const ir_use&)    = delete;

    constexpr ir_use (ir_use&&) noexcept = default; // <-
    ir_use& operator= (ir_use&&)         = delete;

    ~ir_use (void) noexcept
    {
      if (m_def != nullptr)
        m_def->untrack_use (this);
    }

    //! Replace the parent def for this use.
    //! @param d the replacement
    //! @return a reference to this use
    ir_use& replace_def (ir_def& d) noexcept (false)
    {
      make_invalid ();
      d.track_use (this);
      m_def = &d;
      return *this;
    }

    //! Get the def. Don't use this within the scope of ir_variable. The
    //! def pointer is only nullptr when it is in an invalid state. It
    //! should be set to a phony def if it has no real parent.
    //! @return the parent def
    ir_def& get_def (void) noexcept (false)
    {
      if (m_def == nullptr)
        throw ir_exception ("Pointer to ir_def was nullptr.");
      return *m_def;
    }

    friend void ir_def::abandon_children (void) noexcept;

  private:

    void make_invalid (void) noexcept
    {
      m_def->untrack_use (this);
      m_def = nullptr;
    }

    //! Create a use with a parent def.
    constexpr ir_use (ir_def& d, const ir_instruction& instr);

    //! The def for this use (nullptr is a invalid but defined state)
    ir_def *m_def;

    //! Where this use occurs.
    const ir_instruction& m_instr;

  };
}

#endif

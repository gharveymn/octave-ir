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

#include "ir-common-util.hpp"
#include "ir-type-base.hpp"
#include "ir-instruction-fwd.hpp"
#include "ir-constant.hpp"

#include <gch/optional_ref.hpp>
#include <gch/nonnull_ptr.hpp>
#include <gch/tracker.hpp>
#include <plf_list.h>

#include <deque>
#include <unordered_set>
#include <vector>
#include <variant>
#include <any>

namespace gch
{
  class ir_instruction;
  class ir_basic_block;
  class ir_function;
  class ir_basic_block;

  class ir_def;
  class ir_use;

  class ir_variable
  {
  public:

    using block_def_pair = std::pair<nonnull_ptr<ir_basic_block>, nonnull_ptr<ir_def>>;
    using block_def_vect = std::vector<block_def_pair>;

    static constexpr const char anonymous_name[] = "<@>";

    // variables own both defs and uses
    // defs and uses may allow references elsewhere, but these singular
    // and may not be copied or addressed (may be moved).

    ir_variable            (void)                   = delete;
    ir_variable            (const ir_variable&)     = delete;
    ir_variable            (ir_variable&&) noexcept = delete;
    ir_variable& operator= (const ir_variable&)     = delete;
    ir_variable& operator= (ir_variable&&) noexcept = delete;
    ~ir_variable           (void)                   = default;

    ir_variable (ir_function& m, std::string name);

    // has side-effects
    static ir_type normalize_types (block_def_vect& pairs);

    // variables create defs and hold a pointer to that ir_def
    // these pointers must be adjusted when moving or deleting.

    [[nodiscard]]
    constexpr const std::string& get_name (void) const { return m_name; }

    [[nodiscard]]
    constexpr ir_function& get_module (void) const { return m_function; }

    [[nodiscard]]
    bool has_sentinel (void) const noexcept
    {
      return m_undef_var != nullptr;
    }

    [[nodiscard]]
    ir_variable& get_sentinel (void);

    void mark_uninit (ir_basic_block& blk);

    [[nodiscard]]
    constexpr ir_function& get_function (void) noexcept
    {
      return m_function;
    }

    [[nodiscard]]
    constexpr const ir_function& get_function (void) const noexcept
    {
      return m_function;
    }

    [[nodiscard]] constexpr ir_type get_type (void) const noexcept { return m_type; }

    constexpr void set_type (ir_type ty) { m_type = ty; };

    constexpr int create_id (void) noexcept
    {
      return curr_id++;
    }

    ir_variable& get_undef_var (void);

   private:

    // citer find (const ir_def& d) const;

    ir_function& m_function;

    //! The variable name. The default is a synonym for 'anonymous'.
    std::string m_name = anonymous_name;

    ir_type m_type = ir_type_v<void>;

    // Used to indicate if the variable is uninitialized in the current
    // code path. Lazily initialized.
    std::unique_ptr<ir_variable> m_undef_var;

    int curr_id = 0;
  };

  [[nodiscard]]
  std::string undef_name (ir_variable& var)
  {
    return "_undef_" + var.get_name ();
  }

  class ir_use_timeline;

  //! An ssa variable def. It holds very little information about itself,
  //! it's more of just an indicating stub for the variable.
  class ir_def
  {
  public:

    ir_def            (void)                = delete;
    ir_def            (const ir_def&)       = delete;
    ir_def            (ir_def&& d) noexcept = default;
    ir_def& operator= (const ir_def&)       = delete;
    ir_def& operator= (ir_def&&)   noexcept = default;
    ~ir_def           (void)                = default;

    constexpr ir_def (ir_variable& var, ir_instruction& instr)
      : m_var (var),
        m_instr (instr),
        m_id  (var.create_id ())
    { }

    constexpr ir_def (ir_def&& other, ir_instruction& new_instr)
      : m_var (other.m_var),
        m_instr (new_instr),
        m_id  (other.m_id)
    { }

    [[nodiscard]]
    constexpr ir_variable& get_var (void) noexcept { return *m_var; };

    [[nodiscard]]
    constexpr const ir_variable& get_var (void) const noexcept { return *m_var; };

    std::ostream& print (std::ostream& os) const;

    [[nodiscard]]
    const std::string& get_name (void) const;

    [[nodiscard]]
    constexpr auto get_id (void) const { return m_id; };

    [[nodiscard]]
    constexpr ir_type get_type (void) const noexcept { return get_var ().get_type (); }

//    template <typename ...Args>
//    void transfer_bindings (ir_def& src, Args&&... args)
//    {
//      m_use_tracker.transfer_bindings (src.m_use_tracker);
//      m_cache_tracker.transfer_bindings (src.m_cache_tracker);
//    }

    [[nodiscard]]
    constexpr ir_instruction& get_instruction (void) noexcept { return *m_instr; }

    [[nodiscard]]
    constexpr const ir_instruction& get_instruction (void) const noexcept { return *m_instr; }

    constexpr void set_instruction (ir_instruction& instr) noexcept { m_instr.emplace (instr); }

  private:
    nonnull_ptr<ir_variable>    m_var;
    nonnull_ptr<ir_instruction> m_instr;
    int                         m_id;
  };

  class ir_use
    : public intrusive_reporter<ir_use, remote::intrusive_tracker<ir_use_timeline>>
  {
  public:

    using base          = intrusive_reporter<ir_use, remote::intrusive_tracker<ir_use_timeline>>;
    using reporter_type = base;

    // uses can only be explicitly created by defs
    // They may not be copied, but they may be moved.
    // defs maintain a pointer to each use it created

    ir_use            (void)              = delete;
    ir_use            (const ir_use&)     = delete;
    ir_use            (ir_use&&) noexcept = default;
    ir_use& operator= (const ir_use&)     = delete;
    ir_use& operator= (ir_use&&) noexcept = default;
    ~ir_use           (void)              = default;

    ir_use (ir_instruction& instr, ir_use_timeline& ut);

    template <typename It>
    ir_use (ir_instruction& instr, ir_use_timeline& ut, It ut_pos);

    ir_use (ir_instruction& instr, std::insert_iterator<ir_use_timeline>& ut);

    [[nodiscard]] ir_use_timeline&       get_timeline (void)       noexcept;
    [[nodiscard]] const ir_use_timeline& get_timeline (void) const noexcept;

    [[nodiscard]] ir_def&       get_def (void)       noexcept;
    [[nodiscard]] const ir_def& get_def (void) const noexcept;

    [[nodiscard]] ir_variable&       get_var (void)       noexcept;
    [[nodiscard]] const ir_variable& get_var (void) const noexcept;

    [[nodiscard]] ir_type get_type (void) const noexcept;

    [[nodiscard]] const std::string& get_name (void) const;

    [[nodiscard]] std::size_t get_id (void);

    [[nodiscard]]
    ir_instruction&
    get_instruction (void) noexcept
    {
      return *m_instr;
    }

    [[nodiscard]]
    const ir_instruction&
    get_instruction (void) const noexcept
    {
      return *m_instr;
    }

    void
    set_instruction (ir_instruction& instr) noexcept
    {
      m_instr.emplace (instr);
    }

  private:
    nonnull_ptr<ir_instruction> m_instr;
  };

  template <typename InputIt>
  ir_type common_type (InputIt first, InputIt last);

  class ir_use_timeline
    : public intrusive_tracker<ir_use_timeline, remote::intrusive_reporter<ir_use>>
  {
  public:
    using use_tracker  = intrusive_tracker<ir_use_timeline, remote::intrusive_reporter<ir_use>>;

    ir_use_timeline (void)                                  = delete;
    ir_use_timeline (const ir_use_timeline&)                = delete;
    ir_use_timeline (ir_use_timeline&&) noexcept            = default;
    ir_use_timeline& operator= (const ir_use_timeline&)     = delete;
    ir_use_timeline& operator= (ir_use_timeline&&) noexcept = default;
    ~ir_use_timeline (void)                                 = default;

    template <typename ...TrackerArgs>
    /* implicit */
    ir_use_timeline (ir_instruction_iter origin_pos, TrackerArgs&&... args)
      : use_tracker (std::forward<TrackerArgs> (args)...),
        m_instruction_pos (origin_pos)
    { }

    [[nodiscard]]       ir_def& get_def (void)       noexcept;
    [[nodiscard]] const ir_def& get_def (void) const noexcept;

    [[nodiscard]]
    ir_instruction_iter
    get_def_pos (void) noexcept
    {
      return m_instruction_pos;
    }

    [[nodiscard]]
    ir_instruction_citer
    get_def_pos (void) const noexcept
    {
      return m_instruction_pos;
    }

    [[nodiscard]]
    ir_instruction&
    get_def_instruction (void) noexcept
    {
      return *get_def_pos ();
    }

    [[nodiscard]]
    const ir_instruction&
    get_def_instruction (void) const noexcept
    {
      return *get_def_pos ();
    }

    void
    set_instruction_pos (ir_instruction_iter instr) noexcept
    {
      m_instruction_pos = instr;
    }

  private:
    ir_instruction_iter m_instruction_pos;
  };

  class ir_operand
  {
  public:
    ir_operand            (void)                  = delete;
    ir_operand            (const ir_operand&)     = delete;
    ir_operand            (ir_operand&&) noexcept = default;
    ir_operand& operator= (const ir_operand&)     = delete;
    ir_operand& operator= (ir_operand&&) noexcept = default;
    ~ir_operand           (void)                  = default;

    ir_operand (ir_instruction& instr, ir_use_timeline& tl, ir_use_timeline::citer pos)
      : m_data (std::in_place_type<ir_use>, instr, tl, pos)
    { }

    template <typename T>
    constexpr ir_operand (ir_type type, T&& data)
      : m_data (std::in_place_type<ir_constant>, type, std::forward<T> (data))
    { }

    constexpr ir_operand (const ir_constant& c)
      : m_data (c)
    { }

    constexpr ir_operand (ir_constant&& c)
      : m_data (std::move (c))
    { }

    [[nodiscard]]
    constexpr ir_type get_type (void) const
    {
      return std::visit ([] (auto&& x) -> ir_type { return x.get_type (); }, m_data);
    }

    template <typename T>
    [[nodiscard]] constexpr T& as_type (void) { return std::get<T> (m_data); }

    template <typename T>
    [[nodiscard]] constexpr const T& as_type (void) const { return std::get<T> (m_data); }

    [[nodiscard]]
    constexpr bool is_constant (void) const noexcept
    {
      return std::holds_alternative<ir_constant> (m_data);
    }

    [[nodiscard]]
    constexpr bool is_use (void) const noexcept
    {
      return std::holds_alternative<ir_use> (m_data);
    }

    friend struct ir_printer<ir_operand>;

    template <typename T>
    friend constexpr optional_ref<T> get_if (ir_operand& op) noexcept
    {
      return std::get_if<T> (&op.m_data);
    }

    template <typename T>
    friend constexpr optional_ref<const T> get_if (const ir_operand& op) noexcept
    {
      return std::get_if<T> (&op.m_data);
    }

  private:
    std::variant<ir_constant, ir_use> m_data;
  };

}

#endif

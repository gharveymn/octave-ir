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

#ifndef OCTAVE_IR_IR_VARIABLE_HPP
#define OCTAVE_IR_IR_VARIABLE_HPP

#include "utilities/ir-common-util.hpp"
#include "utilities/ir-type-traits.hpp"
#include "values/types/ir-type-base.hpp"
#include "values/ir-constant.hpp"
#include "values/ir-instruction-fwd.hpp"

#include <gch/optional_ref.hpp>
#include <gch/nonnull_ptr.hpp>
#include <gch/tracker.hpp>
#include <plf_list.h>

#include <vector>
#include <variant>
#include <any>

namespace gch
{
  class ir_instruction;
  class ir_block;
  class ir_function;
  class ir_block;

  class ir_def;
  class ir_use;

  class ir_variable
  {
  public:

    using block_def_pair = std::pair<nonnull_ptr<ir_block>, nonnull_ptr<ir_def>>;
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

    void mark_uninit (ir_block& blk);

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

  [[nodiscard]] inline
  std::string
  make_undef_name (ir_variable& var)
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
    constexpr ir_variable& get_variable (void) noexcept { return *m_var; };

    [[nodiscard]]
    constexpr const ir_variable& get_variable (void) const noexcept { return *m_var; };

    std::ostream& print (std::ostream& os) const;

    [[nodiscard]]
    const std::string& get_name (void) const;

    [[nodiscard]]
    constexpr auto get_id (void) const { return m_id; };

    [[nodiscard]]
    constexpr ir_type get_type (void) const noexcept { return get_variable ().get_type (); }

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

  class ir_use_info;

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

    ir_use (ir_instruction& instr, ir_use_timeline& ut, base::remote_interface_type::citer pos);

    ir_use (ir_instruction& instr, const ir_use_info& info);

    [[nodiscard]]
    ir_use_timeline&
    get_timeline (void) noexcept;

    [[nodiscard]]
    const ir_use_timeline&
    get_timeline (void) const noexcept
    {
      return as_mutable (*this).get_timeline ();
    }

    [[nodiscard]]
    ir_def&
    get_def (void) noexcept;

    [[nodiscard]]
    const ir_def&
    get_def (void) const noexcept
    {
      return as_mutable (*this).get_def ();
    }

    [[nodiscard]]
    ir_variable&
    get_variable (void) noexcept;

    [[nodiscard]]
    const ir_variable&
    get_variable (void) const noexcept
    {
      return as_mutable (*this).get_variable ();
    }

    [[nodiscard]]
    ir_type
    get_type (void) const noexcept;

    [[nodiscard]]
    const std::string&
    get_name (void) const;

    [[nodiscard]]
    std::size_t
    get_id (void);

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
      return as_mutable (*this).get_instruction ();
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

    [[nodiscard]]
    ir_def&
    get_def (void) noexcept;

    [[nodiscard]]
    const ir_def&
    get_def (void) const noexcept
    {
      return as_mutable (*this).get_def ();
    }

    [[nodiscard]]
    ir_instruction_iter
    get_def_pos (void) noexcept;

    [[nodiscard]]
    ir_instruction_citer
    get_def_pos (void) const noexcept
    {
      return as_mutable (*this).get_def_pos ();
    }

    [[nodiscard]]
    ir_instruction&
    get_def_instruction (void) noexcept;

    [[nodiscard]]
    const ir_instruction&
    get_def_instruction (void) const noexcept
    {
      return as_mutable (*this).get_def_instruction ();
    }

    [[nodiscard]]
    ir_variable&
    get_variable (void) noexcept;

    [[nodiscard]]
    const ir_variable&
    get_variable (void) const noexcept
    {
      return as_mutable (*this).get_variable ();
    }

    void
    set_instruction_pos (ir_instruction_iter instr) noexcept;

  private:
    ir_instruction_iter m_instruction_pos;
  };

  [[nodiscard]] inline
  bool
  has_same_def (const ir_use_timeline& l, const ir_use_timeline& r) noexcept
  {
    return l.get_def_pos () == r.get_def_pos ();
  }

  [[nodiscard]] inline
  bool
  has_same_def (optional_ref<const ir_use_timeline> lhs, const ir_use_timeline& r) noexcept
  {
    return lhs >>= [&r](const ir_use_timeline& l) noexcept { return has_same_def (l, r); };
  }

  [[nodiscard]] inline
  bool
  has_same_def (const ir_use_timeline& l, optional_ref<const ir_use_timeline> rhs) noexcept
  {
    return has_same_def (rhs, l);
  }

  [[nodiscard]] inline
  bool
  has_same_def (optional_ref<const ir_use_timeline> lhs, optional_ref<const ir_use_timeline> rhs)
    noexcept
  {
    return lhs >>= [rhs](const ir_use_timeline& l) noexcept { return has_same_def (l, rhs); };
  }

  class ir_use_info
  {
  public:
    ir_use_info            (void)                   = delete;
    ir_use_info            (const ir_use_info&)     = default;
    ir_use_info            (ir_use_info&&) noexcept = default;
    ir_use_info& operator= (const ir_use_info&)     = default;
    ir_use_info& operator= (ir_use_info&&) noexcept = default;
    ~ir_use_info           (void)                   = default;

    ir_use_info (ir_use_timeline& ut, ir_use_timeline::citer pos)
      : m_timeline (ut),
        m_insertion_position (pos)
    { }

    [[nodiscard]]
    ir_use_timeline&
    get_timeline (void) const noexcept
    {
      return *m_timeline;
    }

    [[nodiscard]]
    ir_use_timeline::citer
    get_insertion_position (void) const noexcept
    {
      return m_insertion_position;
    }

  private:
    nonnull_ptr<ir_use_timeline> m_timeline;
    ir_use_timeline::citer       m_insertion_position;
  };

  class ir_operand_in
  {
  public:
    ir_operand_in (void)                                = delete;
    ir_operand_in (const ir_operand_in&)                = default;
    ir_operand_in (ir_operand_in&&) noexcept            = default;
    ir_operand_in& operator= (const ir_operand_in&)     = default;
    ir_operand_in& operator= (ir_operand_in&&) noexcept = default;
    ~ir_operand_in (void)                               = default;

    ir_operand_in (const ir_use_info& info)
      : m_data (std::in_place_type<ir_use_info>, info)
    { }

    ir_operand_in (ir_constant&& c)
      : m_data (std::in_place_type<ir_constant>, std::move (c))
    { }

    [[nodiscard]] constexpr
    std::size_t
    index (void) const noexcept
    {
      return m_data.index ();
    }

    template <typename T>
    friend constexpr
    optional_ref<T>
    maybe_get (ir_operand_in&);

    template <typename T>
    friend constexpr
    optional_cref<T>
    maybe_get (const ir_operand_in&);

    template <typename T, typename U>
    friend constexpr
    std::enable_if_t<std::is_same_v<std::decay_t<U>, ir_operand_in>, match_cvref_t<U, T>>
    get (U&&);

  private:
    std::variant<ir_constant, ir_use_info> m_data;
  };

  template <typename T>
  [[nodiscard]] constexpr
  optional_ref<T>
  maybe_get (ir_operand_in& in)
  {
    return std::get_if<T> (&in.m_data);
  }

  template <typename T>
  [[nodiscard]] constexpr
  optional_cref<T>
  maybe_get (const ir_operand_in& in)
  {
    return std::get_if<T> (&in.m_data);
  }

  template <typename T, typename U>
  [[nodiscard]] constexpr
  std::enable_if_t<std::is_same_v<std::decay_t<U>, ir_operand_in>, match_cvref_t<U, T>>
  get (U&& in)
  {
    return static_cast<match_cvref_t<U, T>> (*std::get_if<T> (&in.m_data));
  }

  class ir_operand
  {
  public:
    ir_operand            (void)                  = delete;
    ir_operand            (const ir_operand&)     = delete;
    ir_operand            (ir_operand&&) noexcept = default;
    ir_operand& operator= (const ir_operand&)     = delete;
    ir_operand& operator= (ir_operand&&) noexcept = default;
    ~ir_operand           (void)                  = default;

    ir_operand (ir_instruction& instr, ir_operand_in&& in)
    {
      if (optional_ref<ir_constant> c = maybe_get<ir_constant> (in))
        m_data.emplace<ir_constant> (std::move (*c));
      else
        m_data.emplace<ir_use> (instr, get<ir_use_info> (std::move (in)));
    }

    ir_operand (ir_instruction&, ir_constant&& c)
      : m_data (std::in_place_type<ir_constant>, std::move (c))
    { }

    template <typename T>
    friend constexpr
    optional_ref<T>
    maybe_get (ir_operand&);

    template <typename T>
    friend constexpr
    optional_cref<T>
    maybe_get (const ir_operand&);

    template <typename T, typename U>
    friend constexpr
    std::enable_if_t<std::is_same_v<std::decay_t<U>, ir_operand>, match_cvref_t<U, T>>
    get (U&&);

    [[nodiscard]] constexpr
    ir_type
    get_type (void) const noexcept
    {
      if (auto u = maybe_get<ir_use> (*this))
        return u->get_type ();
      return std::visit ([] (auto&& x) noexcept -> ir_type { return x.get_type (); }, m_data);
    }

    [[nodiscard]] constexpr
    bool
    is_constant (void) const noexcept
    {
      return std::holds_alternative<ir_constant> (m_data);
    }

    [[nodiscard]] constexpr
    bool
    is_use (void) const noexcept
    {
      return std::holds_alternative<ir_use> (m_data);
    }

  private:
    std::variant<ir_constant, ir_use> m_data;
  };

  template <typename T>
  [[nodiscard]] constexpr
  optional_ref<T>
  maybe_get (ir_operand& op)
  {
    return std::get_if<T> (&op.m_data);
  }

  template <typename T>
  [[nodiscard]] constexpr
  optional_cref<T>
  maybe_get (const ir_operand& op)
  {
    return std::get_if<T> (&op.m_data);
  }

  template <typename T, typename U>
  [[nodiscard]] constexpr
  std::enable_if_t<std::is_same_v<std::decay_t<U>, ir_operand>, match_cvref_t<U, T>>
  get (U&& op)
  {
    return static_cast<match_cvref_t<U, T>> (*std::get_if<T> (&op.m_data));
  }

  [[nodiscard]] constexpr
  ir_type
  get_type (const ir_operand& op) noexcept
  {
    if (optional_cref<ir_use> u = maybe_get<ir_use> (op))
      return u->get_type ();
    return get<ir_constant> (op).get_type ();
  }

}

#endif

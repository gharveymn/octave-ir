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


#if ! defined (ir_block_h)
#define ir_block_h 1

#include "octave-config.h"

#include "ir-common-util.h"
#include "ir-component.h"
#include "ir-type.h"
#include "ir-instruction-fwd.h"

#include <plf_list.h>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <stack>
#include <vector>

namespace octave
{
  class ir_phi;
  class ir_structure;
  class ir_instruction;
  class ir_def_instruction;

  class ir_variable;
  class ir_def;
  class ir_use;

  class ir_basic_block : public ir_component
  {
  public:

    using instr_iter      = instr_list::iterator;
    using instr_citer     = instr_list::const_iterator;
    using instr_riter     = instr_list::reverse_iterator;
    using instr_criter    = instr_list::const_reverse_iterator;
    using instr_ref       = instr_list::reference;
    using instr_cref      = instr_list::const_reference;

    using phi_list_type = ir_phi;
    using phi_list      = plf::list<ir_phi>;
    using phi_iter      = phi_list::iterator;
    using phi_citer     = phi_list::const_iterator;
    using phi_riter     = phi_list::reverse_iterator;
    using phi_criter    = phi_list::const_reverse_iterator;
    using phi_ref       = phi_list::reference;
    using phi_cref      = phi_list::const_reference;

    using block_def_pair = std::pair<ir_basic_block&, ir_def *>;
    using block_def_vect = std::vector<block_def_pair>;

    class def_timeline
    {

    public:
      using element_type = std::pair<instr_citer, ir_def *>;
      using def_deque = std::deque<element_type>;
      using iter = def_deque::iterator;
      using citer = def_deque::const_iterator;
      using riter = def_deque::reverse_iterator;
      using criter = def_deque::const_reverse_iterator;

      iter   begin  (void)       noexcept { return m_timeline.begin (); }
      citer  begin  (void) const noexcept { return m_timeline.begin (); }
      iter   end    (void)       noexcept { return m_timeline.end (); }
      citer  end    (void) const noexcept { return m_timeline.end (); }

      riter  rbegin (void)       noexcept { return m_timeline.rbegin (); }
      criter rbegin (void) const noexcept { return m_timeline.rbegin (); }
      riter  rend   (void)       noexcept { return m_timeline.rend (); }
      criter rend   (void) const noexcept { return m_timeline.rend (); }

      constexpr ir_def * fetch_cache (void) const noexcept
      {
        return m_lookback_cache;
      }

      void set_cache (ir_def& latest) noexcept
      {
        m_lookback_cache = &latest;
      }

      void emplace_front (instr_citer pos, ir_def& d)
      {
        if (m_timeline.empty ())
          set_cache (d);
        m_timeline.emplace_front (pos, &d);
      }

      void emplace_back (instr_citer pos, ir_def& d)
      {
        m_timeline.emplace_back (pos, &d);
        set_cache (d);
      }

      void emplace_before (citer dt_pos, instr_citer pos, ir_def& d)
      {
        if (dt_pos == end ())
          set_cache (d);
        m_timeline.emplace (dt_pos, pos, &d);
      }

      void remove (instr_citer instr_cit);

      citer find (instr_citer instr_cit) const;

      void clear_lookback_cache (void) noexcept
      {
        m_lookback_cache = nullptr;
      }

      void track_phi_def (phi_citer pos, ir_def& d)
      {
        if (has_phi_def ())
          throw ir_exception ("block already has a phi def for the variable");
        m_phi_def = std::make_pair (pos, &d);
      }

      constexpr bool has_phi_def (void) const noexcept
      {
        return m_phi_def.second != nullptr;
      }

      def_deque::size_type size (void) const noexcept
      {
        return m_timeline.size ();
      }

    private:

      ir_def *m_lookback_cache = nullptr;

      std::pair<phi_citer, ir_def *> m_phi_def = {{}, nullptr};

      //! A timeline of defs created in this block.
      def_deque m_timeline;

    };

  private:

    using var_timeline_map = std::unordered_map<ir_variable *, def_timeline>;
    using vtm_iter = var_timeline_map::iterator;
    using vtm_citer = var_timeline_map::const_iterator;

  public:

    ir_def * fetch_cached_def (ir_variable& var) const;

    ir_def * fetch_proximate_def (ir_variable& var, instr_citer pos) const;

    virtual ir_def * join_pred_defs (ir_variable& var);

    void set_cached_def (ir_def& d);

    explicit ir_basic_block (ir_structure& parent);

    ~ir_basic_block (void) noexcept override;

    // No copying!
    ir_basic_block (const ir_basic_block &) = delete;

    ir_basic_block& operator=(const ir_basic_block &) = delete;

    // all

    // front and back won't throw because the constructor will
    // always emplace a return instruction
    instr_iter   begin (void)         noexcept { return m_instrs.begin (); }
    instr_citer  begin (void)   const noexcept { return m_instrs.begin (); }
    instr_citer  cbegin (void)  const noexcept { return m_instrs.cbegin (); }

    instr_iter   end (void)           noexcept { return m_instrs.end ();   }
    instr_citer  end (void)     const noexcept { return m_instrs.end ();   }
    instr_citer  cend (void)    const noexcept { return m_instrs.cbegin (); }

    instr_riter  rbegin (void)        noexcept { return m_instrs.rbegin (); }
    instr_criter rbegin (void)  const noexcept { return m_instrs.rbegin (); }
    instr_criter crbegin (void) const noexcept { return m_instrs.crbegin (); }

    instr_riter  rend (void)          noexcept { return m_instrs.rend (); }
    instr_criter rend (void)    const noexcept { return m_instrs.rend (); }
    instr_criter crend (void)   const noexcept { return m_instrs.crend (); }

    instr_ref    front (void)         noexcept { return m_instrs.front (); }
    instr_cref   front (void)   const noexcept { return m_instrs.front (); }

    instr_ref    back (void)          noexcept { return m_instrs.back (); }
    instr_cref   back (void)    const noexcept { return m_instrs.back (); }

    size_t       size (void)    const noexcept { return m_instrs.size (); }
    bool         empty (void)   const noexcept { return m_instrs.empty (); }

    // phi

    template <typename ...Args>
    ir_phi * create_phi (Args&&... args);

    instr_iter   phi_begin (void)        noexcept { return m_instrs.begin (); }
    instr_citer  phi_begin (void)  const noexcept { return m_instrs.begin (); }

    instr_iter   phi_end (void)          noexcept { return m_body_begin; }
    instr_citer  phi_end (void)    const noexcept { return m_body_begin; }

    instr_riter  phi_rbegin (void)       noexcept { return instr_riter (phi_end ()); }
    instr_criter phi_rbegin (void) const noexcept { return instr_criter (phi_end ()); }

    instr_riter  phi_rend (void)         noexcept { return instr_riter (phi_begin ()); }
    instr_criter phi_rend (void)   const noexcept { return instr_criter (phi_begin ()); }

    size_t       num_phi (void)    const noexcept { return m_num_phi; }
    bool         has_phi (void)    const noexcept { return phi_begin () != phi_end (); }

    instr_iter   erase_phi (instr_citer pos);

    // body

    instr_iter   body_begin (void)        noexcept { return m_body_begin; }
    instr_citer  body_begin (void)  const noexcept { return m_body_begin; }

    instr_iter   body_end (void)          noexcept { return m_terminator;   }
    instr_citer  body_end (void)    const noexcept { return m_terminator;   }

    instr_riter  body_rbegin (void)       noexcept { return instr_riter (body_end ()); }
    instr_criter body_rbegin (void) const noexcept { return instr_criter (body_end ()); }

    instr_riter  body_rend (void)         noexcept { return instr_riter (body_begin ()); }
    instr_criter body_rend (void)   const noexcept { return instr_criter (body_begin ()); }

    size_t       num_body (void)    const noexcept { return size () - m_num_phi - 1; }

    bool         has_body (void)    const noexcept { return phi_begin () != phi_end (); }

    template <typename T>
    using is_instruction = std::is_base_of<ir_instruction, T>;

    template <typename T>
    using is_phi = std::is_same<ir_phi, T>;

    template <typename T>
    using is_nonphi = negation<is_phi<T>>;

    template <typename T>
    using is_nonphi_instruction = conjunction<is_nonphi<T>, is_instruction<T>>;

    template <typename T>
    using has_return = std::is_base_of<ir_def_instruction, T>;

    template <typename T>
    using is_nonphi_return_instruction
      = conjunction<is_nonphi_instruction<T>, has_return<T>>;

    template <typename T>
    using is_nonphi_nonreturn_instruction
      = conjunction<is_nonphi_instruction<T>, negation<has_return<T>>>;

    template <typename T, typename ...Args,
      enable_if_t<is_nonphi_return_instruction<T>::value>* = nullptr>
    T& emplace_front (Args&&... args)
    {
      std::unique_ptr<T> u = create_instruction<T> (std::forward<Args> (args)...);
      T *ret = u.get ();
      m_body_begin = m_instrs.insert (body_begin (), std::move (u));
      try
        {
          def_emplace (m_body_begin, ret->get_return ());
        }
      catch (const std::exception& e)
        {
          m_body_begin = m_instrs.erase (m_body_begin);
          throw e;
        }
      return *ret;
    }

    template <typename T, typename ...Args,
      enable_if_t<is_nonphi_return_instruction<T>::value>* = nullptr>
    T& emplace_back (Args&&... args)
    {
      std::unique_ptr<T> u = create_instruction<T> (std::forward<Args> (args)...);
      T *ret = u.get ();
      instr_iter it = m_instrs.insert (body_end (), std::move (u));
      try
        {
          def_emplace (it, ret->get_return());
        }
      catch (const std::exception& e)
        {
          m_instrs.erase (it);
          throw e;
        }
      if (m_body_begin == body_end ())
        --m_body_begin;
      return *ret;
    }

    template <typename T, typename ...Args,
      enable_if_t<is_nonphi_return_instruction<T>::value>* = nullptr>
    T& emplace_before (instr_citer pos, Args&&... args)
    {
      if (pos == m_instrs.end () || is_phi_iter (pos))
        throw ir_exception ("instruction must be placed within the body");
      std::unique_ptr<T> u = create_instruction<T> (std::forward<Args> (args)...);
      T *ret = u.get ();
      instr_iter it = m_instrs.insert (pos, std::move (u));
      try
        {
          def_emplace (it, ret->get_return());
        }
      catch (const std::exception& e)
        {
          m_instrs.erase (it);
          throw e;
        }
      if (m_body_begin == pos)
        m_body_begin = it;
      return *ret;
    }

    template <typename T, typename ...Args,
      enable_if_t<is_nonphi_nonreturn_instruction<T>::value>* = nullptr>
    T& emplace_front (Args&&... args)
    {
      std::unique_ptr<T> u = create_instruction<T> (std::forward<Args> (args)...);
      T *ret = u.get ();
      m_body_begin = m_instrs.insert (m_body_begin, std::move (u));
      return *ret;
    }

    template <typename T, typename ...Args,
      enable_if_t<is_nonphi_nonreturn_instruction<T>::value>* = nullptr>
    T& emplace_back (Args&&... args)
    {
      std::unique_ptr<T> u = create_instruction<T> (std::forward<Args> (args)...);
      T *ret = u.get ();
      m_instrs.push_back (std::move (u));
      if (m_body_begin == body_end ())
        --m_body_begin;
      return *ret;
    }

    template <typename T, typename ...Args,
      enable_if_t<is_nonphi_nonreturn_instruction<T>::value>* = nullptr>
    T& emplace_before (instr_citer pos, Args&&... args)
    {
      if (pos == end () || is_phi_iter (pos))
        throw ir_exception ("instruction must be placed within the body");
      std::unique_ptr<T> u = create_instruction<T> (std::forward<Args> (args)...);
      T *ret = u.get ();
      instr_iter it = m_instrs.insert (pos, std::move (u));
      if (m_body_begin == pos)
        m_body_begin = it;
      return *ret;
    }

    instr_iter erase (instr_citer pos);

    instr_iter erase (instr_citer first, instr_citer last) noexcept;

    // predecessors

    link_iter pred_begin (void);
    link_iter pred_end   (void);
    ir_basic_block * pred_front (void) { return *pred_begin (); }
    ir_basic_block * pred_back (void) { return *(--pred_end ()); }
    std::size_t num_preds (void);
    bool has_preds (void);
    bool has_multiple_preds (void);

    // successors
    link_iter succ_begin (void);
    link_iter succ_end (void);
    ir_basic_block * succ_front (void) { return *succ_begin (); }
    ir_basic_block * succ_back (void) { return *(--succ_end ()); }
    std::size_t num_succs (void);
    bool has_succs (void);
    bool has_multiple_succs (void);

    link_iter leaf_begin (void) noexcept override { return link_iter (this); }

    link_iter leaf_end (void) noexcept override { return ++link_iter (this); }

    ir_basic_block * get_entry_block (void) noexcept override { return this; }

    ir_function& get_function (void) noexcept override;

    const ir_function& get_function (void) const noexcept override;

    void reset (void) noexcept override;

  protected:

    void def_emplace (instr_citer pos, ir_def& d);
    void def_emplace_front (ir_def& d);
    void def_emplace_back (ir_def& d);

  private:

    static bool is_phi_iter (instr_citer cit);

    template <typename T, typename ...Args>
    std::unique_ptr<T> create_instruction (Args&&... args)
    {
      return octave::make_unique<T> (*this, std::forward<Args> (args)...);
    }

    ir_structure& m_parent;

    // list of instructions
    instr_list m_instrs;

    std::size_t m_num_phi = 0;
    instr_iter m_body_begin;
    instr_iter m_terminator;

    // map of variables to the ir_def timeline for this block

    // predecessors and successors to this block

    // map from ir_variable to ir_def-timeline structs
    var_timeline_map m_vt_map;

  };

  class ir_condition_block : public ir_basic_block
  {
  public:
    using ir_basic_block::ir_basic_block;
  };

  class ir_loop_condition_block : public ir_basic_block
  {
  public:

    using ir_basic_block::ir_basic_block;

//    ir_def * join_pred_defs (ir_variable& var) override;

  private:

  };

  template <>
  struct ir_type::instance<ir_basic_block *>
  {
    using type = ir_basic_block;
    static constexpr
    impl m_impl = create_type<type> ("block_ptr");
  };
}

#endif

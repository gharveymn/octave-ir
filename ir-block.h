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

#include <list>
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
    
    using instr_list_type = std::unique_ptr<ir_instruction>;
    using instr_list = std::list<std::unique_ptr<ir_instruction>>;
    using iter = instr_list::iterator;
    using citer = instr_list::const_iterator;
    using riter = instr_list::reverse_iterator;
    using criter = instr_list::const_reverse_iterator;
    using ref = instr_list::reference;
    using cref = instr_list::const_reference;
  
    using block_def_pair = std::pair<ir_basic_block&, ir_def*>;
    using block_def_vect = std::vector<block_def_pair>;
  
  private:
    
    class def_timeline
    {
    
    public:
      using instr_citer = ir_basic_block::citer;
      
      using element_type = std::pair<instr_citer, ir_def *>;
      using def_deque = std::deque<element_type>;
      using iter = def_deque::iterator;
      using citer = def_deque::const_iterator;
      using riter = def_deque::reverse_iterator;
      using criter = def_deque::const_reverse_iterator;
      
      iter begin (void) { return m_timeline.begin (); }
      citer begin (void) const { return m_timeline.begin (); }
      iter end (void) { return m_timeline.end (); }
      citer end (void) const { return m_timeline.end (); }
      riter rbegin (void) { return m_timeline.rbegin (); }
      criter rbegin (void) const { return m_timeline.rbegin (); }
      riter rend (void) { return m_timeline.rend (); }
      criter rend (void) const { return m_timeline.rend (); }
      
      
      constexpr ir_def * fetch_cache (void) const noexcept
      {
        return m_cache;
      }
      
      void set_cache (ir_def& latest) noexcept
      {
        m_cache = &latest;
      }
      
      void emplace (citer dt_pos, instr_citer pos, ir_def& d)
      {
        if (dt_pos == end ())
          set_cache (d);
        m_timeline.emplace (dt_pos, pos, &d);
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
      
      def_deque::size_type size (void) const noexcept
      {
        return m_timeline.size ();
      }
    
    private:
      
      //! The latest ir_def (which may or may not have been created here)
      ir_def * m_cache = nullptr;
      
      //! A timeline of defs created in this block.
      def_deque m_timeline;
      
    };
    
    using var_timeline_map = std::unordered_map<ir_variable *, def_timeline>;
    using vtm_iter = var_timeline_map::iterator;
    using vtm_citer = var_timeline_map::const_iterator;
  
  public:
    
    ir_def * fetch_cached_def (ir_variable& var) const;
    
    ir_def * fetch_proximate_def (ir_variable& var, citer pos) const;
    
    // side effects!
    ir_def * join_defs (ir_variable& var);
    
    // side effects!
    ir_def * join_defs (ir_variable& var, citer pos);
    
    virtual ir_def * join_pred_defs (ir_variable& var);
    
    void set_cached_def (ir_def& d);
    
    ir_basic_block (ir_module& mod, ir_structure& parent);
    
    ~ir_basic_block (void) noexcept override;
    
    // No copying!
    ir_basic_block (const ir_basic_block &) = delete;
    
    ir_basic_block& operator=(const ir_basic_block &) = delete;
    
    // all
    
    // front and back won't throw because the constructor will
    // always emplace a return instruction
    iter   begin (void)         noexcept { return m_instrs.begin (); }
    citer  begin (void)   const noexcept { return m_instrs.begin (); }
    citer  cbegin (void)  const noexcept { return m_instrs.cbegin (); }
    
    iter   end (void)           noexcept { return m_instrs.end ();   }
    citer  end (void)     const noexcept { return m_instrs.end ();   }
    citer  cend (void)    const noexcept { return m_instrs.cbegin (); }
    
    riter  rbegin (void)        noexcept { return m_instrs.rbegin (); }
    criter rbegin (void)  const noexcept { return m_instrs.rbegin (); }
    criter crbegin (void) const noexcept { return m_instrs.crbegin (); }
    
    riter  rend (void)          noexcept { return m_instrs.rend (); }
    criter rend (void)    const noexcept { return m_instrs.rend (); }
    criter crend (void)   const noexcept { return m_instrs.crend (); }
    
    ref    front (void)         noexcept { return m_instrs.front (); }
    cref   front (void)   const noexcept { return m_instrs.front (); }
    
    ref    back (void)          noexcept { return m_instrs.back (); }
    cref   back (void)    const noexcept { return m_instrs.back (); }
    
    size_t size (void)    const noexcept { return m_instrs.size (); }
    
    bool   empty (void)   const noexcept { return m_instrs.empty (); }
    
    // phi
    
    iter   phi_begin (void)        noexcept { return m_instrs.begin (); }
    citer  phi_begin (void)  const noexcept { return m_instrs.begin (); }
    
    iter   phi_end (void)          noexcept { return m_body_begin; }
    citer  phi_end (void)    const noexcept { return m_body_begin; }
    
    riter  phi_rbegin (void)       noexcept { return riter (phi_end ()); }
    criter phi_rbegin (void) const noexcept { return criter (phi_end ()); }
    
    riter  phi_rend (void)         noexcept { return riter (phi_begin ()); }
    criter phi_rend (void)   const noexcept { return criter (phi_begin ()); }
    
    size_t num_phi (void)    const noexcept { return m_num_phi; }
    
    bool   has_phi (void)    const noexcept { return phi_begin () != phi_end (); }
    
    // body
    
    iter   body_begin (void)        noexcept { return m_body_begin; }
    citer  body_begin (void)  const noexcept { return m_body_begin; }
    
    iter   body_end (void)          noexcept { return m_terminator;   }
    citer  body_end (void)    const noexcept { return m_terminator;   }
    
    riter  body_rbegin (void)       noexcept { return riter (body_end ()); }
    criter body_rbegin (void) const noexcept { return criter (body_end ()); }
    
    riter  body_rend (void)         noexcept { return riter (body_begin ()); }
    criter body_rend (void)   const noexcept { return criter (body_begin ()); }
    
    size_t num_body (void)    const noexcept { return size () - m_num_phi - 1; }
    
    bool   has_body (void)    const noexcept { return phi_begin () != phi_end (); }
    
    ir_phi * create_phi (ir_variable& var, ir_type ty,
                         const block_def_vect& pairs);
    
    iter remove_phi (citer pos);
    
    template <typename T, typename = void>
    struct is_instruction : std::false_type
    { };
    
    template <typename T>
    struct is_instruction<T, enable_if_t<std::is_base_of<ir_instruction,
      T>::value>>
      : std::true_type
    { };
    
    template <typename T, typename = void>
    struct is_phi : std::false_type
    { };
    
    template <typename T>
    struct is_phi<T, enable_if_t<std::is_same<ir_phi, T>::value>>
      : std::true_type
    { };
    
    template <typename T, typename = void>
    struct is_nonphi_instruction : std::false_type
    { };
    
    template <typename T>
    struct is_nonphi_instruction<T,
      enable_if_t<is_instruction<T>::value && ! is_phi<T>::value>>
      : std::true_type
    { };
    
    template <typename T, typename = void>
    struct has_return : std::false_type
    { };
    
    template <typename T>
    struct has_return<T,
      enable_if_t<std::is_base_of<ir_def_instruction, T>::value>>
      : std::true_type
    { };
  
    template <typename T, typename ...Args>
    enable_if_t<ir_basic_block::is_nonphi_instruction<T>::value
                && ir_basic_block::has_return<T>::value, T>&
    emplace_front (Args&&... args)
    {
      std::unique_ptr<T> u = octave::make_unique<T> (*this,
                                                     std::forward<Args> (args)...);
      T *ret = u.get ();
      m_body_begin = m_instrs.insert (m_body_begin, std::move (u));
      try
        {
          def_emplace (m_body_begin, ret->get_return ());
        }
      catch (const std::exception& e)
        {
          m_body_begin = erase (m_body_begin);
          throw e;
        }
      return *ret;
    }
  
    template <typename T, typename ...Args>
    enable_if_t<ir_basic_block::is_nonphi_instruction<T>::value
                && ir_basic_block::has_return<T>::value, T>&
    emplace_back (Args&&... args)
    {
      std::unique_ptr<T> u = octave::make_unique<T> (*this,
                                                     std::forward<Args> (args)...);
      T *ret = u.get ();
      iter it = m_instrs.insert (body_end (), std::move (u));
      try
        {
          def_emplace (it, ret->get_return());
        }
      catch (const std::exception& e)
        {
          erase (it);
          throw e;
        }
      if (m_body_begin == body_end ())
        --m_body_begin;
      return *ret;
    }
  
    template <typename T, typename ...Args>
    enable_if_t<ir_basic_block::is_nonphi_instruction<T>::value
                && ir_basic_block::has_return<T>::value, T>&
    emplace_before (citer pos, Args&&... args)
    {
      if (! is_normal_instruction (pos))
        throw ir_exception ("instruction must be placed within the body");
      std::unique_ptr<T> u = octave::make_unique<T> (*this,
                                                     std::forward<Args> (args)...);
      T *ret = u.get ();
      iter it = m_instrs.insert (pos, std::move (u));
      try
        {
          def_emplace (it, ret->get_return());
        }
      catch (const std::exception& e)
        {
          erase (it);
          throw e;
        }
      if (m_body_begin == pos)
        m_body_begin = it;
      return *ret;
    }
  
    template <typename T, typename ...Args>
    enable_if_t<ir_basic_block::is_nonphi_instruction<T>::value
                && ! ir_basic_block::has_return<T>::value, T>&
    emplace_front (Args&&... args)
    {
      std::unique_ptr<T> u = octave::make_unique<T> (*this,
                                                     std::forward<Args> (args)...);
      T *ret = u.get ();
      m_body_begin = m_instrs.insert (m_body_begin, std::move (u));
      return *ret;
    }
  
    template <typename T, typename ...Args>
    enable_if_t<ir_basic_block::is_nonphi_instruction<T>::value
                && ! ir_basic_block::has_return<T>::value, T>&
    emplace_back (Args&&... args)
    {
      std::unique_ptr<T> u = octave::make_unique<T> (*this,
                                                     std::forward<Args> (args)...);
      T *ret = u.get ();
      m_instrs.push_back (std::move (u));
      if (m_body_begin == body_end ())
        --m_body_begin;
      return *ret;
    }
  
    template <typename T, typename ...Args>
    enable_if_t<ir_basic_block::is_nonphi_instruction<T>::value
                && ! ir_basic_block::has_return<T>::value, T>&
    emplace_before (citer pos, Args&&... args)
    {
      if (! is_normal_instruction (pos))
        throw ir_exception ("instruction must be placed within the body");
      std::unique_ptr<T> u = octave::make_unique<T> (*this,
                                                     std::forward<Args> (args)...);
      T *ret = u.get ();
      iter it = m_instrs.insert (pos, std::move (u));
      if (m_body_begin == pos)
        m_body_begin = it;
      return *ret;
    }
    
    bool is_normal_instruction (citer pos) const;
    
    iter erase (citer pos) noexcept;
    
    iter erase (citer first, citer last) noexcept;
    
    // predecessors
    
    link_iter pred_begin (void);
    link_iter pred_end (void);
    std::size_t num_preds (void);
    bool has_preds (void);
    bool has_multiple_preds (void);
    
    // successors
    link_iter succ_begin (void);
    link_iter succ_end (void);
    std::size_t num_succs (void);
    bool has_succs (void);
    bool has_multiple_succs (void);
    
    link_iter leaf_begin (void) noexcept override { return link_iter (this); }
    
    link_iter leaf_end (void) noexcept override { return ++link_iter (this); }
    
    ir_basic_block * get_entry_block (void) override { return this; }
  
  protected:
    
    void def_emplace (citer pos, ir_def& d);
    void def_emplace_front (ir_def& d);
    void def_emplace_back (ir_def& d);
  
  private:
    
    ir_structure& m_parent;
    
    // list of instructions
    instr_list m_instrs;
    
    std::size_t m_num_phi = 0;
    iter m_body_begin;
    iter m_terminator;
    
    // map of variables to the ir_def timeline for this block
    
    // predecessors and successors to this block
    
    // map from ir_variable to ir_def-timeline structs
    var_timeline_map m_vt_map;
    
  };
  
  class ir_condition_block : public ir_basic_block
  {
  public:
    ir_condition_block (ir_module& mod, ir_structure& parent)
      : ir_basic_block (mod, parent)
    { }
  
    ~ir_condition_block (void) noexcept override;
  };
  
  class ir_loop_condition_block : public ir_basic_block
  {
  public:
    
    ir_loop_condition_block (ir_module& mod, ir_structure& parent)
      : ir_basic_block (mod, parent)
    { }
    
    ~ir_loop_condition_block (void) noexcept override;
    
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

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

#include "ir-common.h"
#include "ir-component.h"
#include "ir-variable.h"

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
  class ir_def_instruction;
  
  class ir_basic_block : public ir_component
  {
    
    using def = ir_variable::def;
    using use = ir_variable::use;
  public:
    
    using instr_list_type = std::unique_ptr<ir_instruction>;
    using instr_list = std::list<std::unique_ptr<ir_instruction>>;
    using iter = instr_list::iterator;
    using citer = instr_list::const_iterator;
    using riter = instr_list::reverse_iterator;
    using criter = instr_list::const_reverse_iterator;
    using ref = instr_list::reference;
    using cref = instr_list::const_reference;
  
  private:
    
    class def_timeline
    {
    
    public:
      using def = ir_variable::def;
      using instr_citer = ir_basic_block::citer;
      
      using element_type = std::pair<instr_citer, def *>;
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
      
      
      constexpr def * fetch_cache (void) const noexcept
      {
        return m_cache;
      }
      
      void set_cache (def& latest) noexcept
      {
        m_cache = &latest;
      }
      
      void emplace (citer dt_pos, instr_citer pos, def& d)
      {
        if (dt_pos == end ())
          set_cache (d);
        m_timeline.emplace (dt_pos, pos, &d);
      }
      
      void emplace_front (instr_citer pos, def& d)
      {
        if (m_timeline.empty ())
          set_cache (d);
        m_timeline.emplace_front (pos, &d);
      }
      
      void emplace_back (instr_citer pos, def& d)
      {
        m_timeline.emplace_back (pos, &d);
        set_cache (d);
      }
      
      def_deque::size_type size (void) const noexcept
      {
        return m_timeline.size ();
      }
    
    private:
      
      //! The latest def (which may or may not have been created here)
      def * m_cache = nullptr;
      
      //! A timeline of defs created in this block.
      def_deque m_timeline;
      
    };
    
    using var_timeline_map = std::unordered_map<ir_variable *, def_timeline>;
    using vtm_iter = var_timeline_map::iterator;
    using vtm_citer = var_timeline_map::const_iterator;
  
  public:
    
    def * fetch_cached_def (ir_variable& var) const;
    
    def * fetch_proximate_def (ir_variable& var, citer pos) const;
    
    // side effects!
    def * join_defs (ir_variable& var);
    
    // side effects!
    def * join_defs (ir_variable& var, citer pos);
    
    virtual def * join_pred_defs (ir_variable& var);
    
    void set_cached_def (def& d);
    
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
    
    template <typename ...Args>
    ir_phi * create_phi (Args&&... args);
    
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
    enable_if_t<is_nonphi_instruction<T>::value && has_return<T>::value, T>&
    emplace_front (Args&&... args);
    
    template <typename T, typename ...Args>
    enable_if_t<is_nonphi_instruction<T>::value && has_return<T>::value, T>&
    emplace_back (Args&&... args);
    
    template <typename T, typename ...Args>
    enable_if_t<is_nonphi_instruction<T>::value && has_return<T>::value, T>&
    emplace_before (citer pos, Args&&... args);
    
    template <typename T, typename ...Args>
    enable_if_t<is_nonphi_instruction<T>::value && ! has_return<T>::value, T>&
    emplace_front (Args&&... args);
    
    template <typename T, typename ...Args>
    enable_if_t<is_nonphi_instruction<T>::value && ! has_return<T>::value, T>&
    emplace_back (Args&&... args);
    
    template <typename T, typename ...Args>
    enable_if_t<is_nonphi_instruction<T>::value && ! has_return<T>::value, T>&
    emplace_before (citer pos, Args&&... args);
    
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
    
    void def_emplace (citer pos, def& d);
    void def_emplace_front (def& d);
    void def_emplace_back (def& d);
  
  private:
    
    ir_structure& m_parent;
    
    // list of instructions
    instr_list m_instrs;
    
    std::size_t m_num_phi = 0;
    iter m_body_begin;
    iter m_terminator;
    
    // map of variables to the def timeline for this block
    
    // predecessors and successors to this block
    
    // map from ir_variable to def-timeline structs
    var_timeline_map m_vt_map;
    
  };
  
  class ir_condition_block : public ir_basic_block
  {
  public:
    ir_condition_block (ir_module& mod, ir_structure& parent)
      : ir_basic_block (mod, parent)
    { }
  };
  
  class ir_loop_condition_block : public ir_basic_block
  {
  public:
    
    ir_loop_condition_block (ir_module& mod, ir_structure& parent)
      : ir_basic_block (mod, parent)
    { }
    
    ir_variable::def * join_pred_defs (ir_variable& var) override;
  
  private:
  
  };
  
  template <>
  struct ir_type::instance<ir_basic_block>
  {
    using type = ir_basic_block;
    static constexpr
    impl m_impl = create_type<type> ("block");
  };
}

#endif

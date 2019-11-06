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

#if ! defined (ir_component_h)
#define ir_component_h 1

#include "octave-config.h"
#include "ir-component.h"
#include "ir-variable.h"

#include <stack>

namespace octave
{

  class block;
  class ir_component;
  class ir_module;

  class ir_structure : public ir_component
  {
  public:
    ir_structure (ir_module& mod, ir_structure *parent)
      : ir_component (mod, parent)
    { }
  
    ~ir_structure (void) override = 0;

    virtual link_iter pred_begin (ir_component *c)  = 0;
    virtual link_iter pred_end (ir_component *c) = 0;

    virtual void invalidate_leaf_cache (void) noexcept { }
    
  private:
  };

  class ir_component_sequence : public ir_structure
  {
  public:

    ir_component_sequence (ir_module& mod, ir_structure& parent)
      : ir_structure (mod, &parent)
    { }

    bool empty (void) const { return m_body.empty (); }

    comp_citer begin (void) const noexcept { return m_body.begin (); }
    comp_citer end (void) const noexcept { return m_body.end (); }

    comp_cref front (void) const { return m_body.front (); }
    comp_cref back (void) const { return m_body.back (); }

    comp_citer last (void) const
    {
      if (m_body.empty ())
        throw ir_exception ("Component sequence was empty.");
      return --end ();
    }

    comp_citer find (ir_component *blk) const;

    link_iter pred_begin (ir_component *c) override;
    link_iter pred_end (ir_component *c) override;

    link_iter leaf_begin (void) noexcept override;
    link_citer leaf_begin (void) const noexcept override;
    link_iter leaf_end (void) noexcept override;
    link_citer leaf_end (void) const noexcept override;

    template <typename T, typename ...Args>
    enable_if_t<std::is_base_of<ir_component, T>::value, T>&
    emplace_back (Args&&... args);

    friend class ir_module;

  private:

    ir_component_sequence (ir_module& mod)
      : ir_structure (mod, nullptr)
    { }

    comp_list m_body;

  };

  class ir_fork_component : public ir_structure
  {
  public:

    ir_fork_component (ir_module& mod, ir_structure& parent)
      : ir_structure (mod, &parent),
        m_condition (mod, parent)
    { }

    link_iter pred_begin (ir_component *c) override;
    link_iter pred_end (ir_component *c) override;

    void invalidate_leaf_cache (void) noexcept override;
    void generate_leaf_cache (void);

    link_iter leaf_begin (void) override;
    link_citer leaf_begin (void) const noexcept override;
    link_iter leaf_end (void) override;
    link_citer leaf_end (void) const noexcept override;

  private:

    ir_condition_block m_condition;
    comp_list m_subcomponents;

    link_vec m_leaf_cache;

  };

  //!
  class ir_loop_component : public ir_structure
  {
  public:

    ir_loop_component (ir_module& mod, ir_structure& parent)
      : ir_structure (mod, &parent),
        m_entry (mod, *this),
        m_body (mod, *this),
        m_update (mod, *this),
        m_condition (mod, *this)
    {
      m_leaf.push_back (&m_condition);

      m_cond_preds.push_back (&m_entry);
      m_cond_preds.push_back (&m_update);
    }

    link_iter pred_begin (ir_component *c) override;
    link_iter pred_end (ir_component *c) override;

    link_iter leaf_begin (void) noexcept override;
    link_citer leaf_begin (void) const noexcept override;
    link_iter leaf_end (void) noexcept override;
    link_citer leaf_end (void) const noexcept override;

  private:

    ir_basic_block m_entry;

    ir_component_sequence m_body;

    // the loop update block; always unconditionally branches back to
    // condition block
    ir_basic_block m_update;


    // preds: entry, update
    // succ: body, exit
    ir_loop_condition_block m_condition;

    link_vec m_leaf;

    link_vec m_cond_preds;

  };

}

#endif

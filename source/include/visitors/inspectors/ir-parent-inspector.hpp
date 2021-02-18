/** ir-parent-inspector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_PARENT_INSPECTOR_HPP
#define OCTAVE_IR_IR_PARENT_INSPECTOR_HPP

#include "components/ir-component-fwd.hpp"
#include "components/ir-structure.hpp"

namespace gch
{

  template <typename Derived, typename Result>
  class ir_parent_inspector
  {
  public:
    ir_parent_inspector            (void)                           = default;
    ir_parent_inspector            (const ir_parent_inspector&)     = default;
    ir_parent_inspector            (ir_parent_inspector&&) noexcept = default;
    ir_parent_inspector& operator= (const ir_parent_inspector&)     = default;
    ir_parent_inspector& operator= (ir_parent_inspector&&) noexcept = default;
    ~ir_parent_inspector           (void)                           = default;

    Result
    operator() (const ir_subcomponent& sub)
    {
      set_subcomponent (sub);
      sub.get_parent ().accept (static_cast<Derived&> (*this));
      return std::move (m_result);
    }

  protected:
    const ir_subcomponent&
    set_subcomponent (const ir_subcomponent& sub) noexcept
    {
      return *(m_subcomponent = &sub);
    }

    const ir_subcomponent&
    get_subcomponent (void) noexcept
    {
      return *m_subcomponent;
    }

    void
    set_result (Result&& result) noexcept
    {
      m_result = std::move (result);
    }

  private:
    const ir_subcomponent * m_subcomponent;
    Result                  m_result;
  };







}



#endif // OCTAVE_IR_IR_PARENT_INSPECTOR_HPP

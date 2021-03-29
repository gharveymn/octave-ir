/** ir-abstract-inspector.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_ABSTRACT_INSPECTOR_HPP
#define OCTAVE_IR_IR_ABSTRACT_INSPECTOR_HPP

namespace gch
{

  template <typename ...Types>
  class abstract_inspector
    : public abstract_inspector<Types>...
  {
  public:
    abstract_inspector            (void)                          = default;
    abstract_inspector            (const abstract_inspector&)     = default;
    abstract_inspector            (abstract_inspector&&) noexcept = default;
    abstract_inspector& operator= (const abstract_inspector&)     = default;
    abstract_inspector& operator= (abstract_inspector&&) noexcept = default;
    ~abstract_inspector           (void) override                 = 0;

    using abstract_inspector<Types>::visit...;
  };

  template <typename T>
  class abstract_inspector<T>
  {
  public:
    abstract_inspector            (void)                          = default;
    abstract_inspector            (const abstract_inspector&)     = default;
    abstract_inspector            (abstract_inspector&&) noexcept = default;
    abstract_inspector& operator= (const abstract_inspector&)     = default;
    abstract_inspector& operator= (abstract_inspector&&) noexcept = default;
    virtual ~abstract_inspector   (void)                          = 0;

    virtual
    void
    visit (const T&) = 0;
  };

}

#endif // OCTAVE_IR_IR_ABSTRACT_INSPECTOR_HPP

/** ir-abstract-mutator.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_ABSTRACT_MUTATOR_HPP
#define OCTAVE_IR_IR_ABSTRACT_MUTATOR_HPP

namespace gch
{

  template <typename ...Types>
  class abstract_mutator
    : public abstract_mutator<Types>...
  {
  public:
    using abstract_mutator<Types>::visit...;
  };

  template <typename T>
  class abstract_mutator<T>
  {
  public:
    abstract_mutator            (void)                        = default;
    abstract_mutator            (const abstract_mutator&)     = default;
    abstract_mutator            (abstract_mutator&&) noexcept = default;
    abstract_mutator& operator= (const abstract_mutator&)     = default;
    abstract_mutator& operator= (abstract_mutator&&) noexcept = default;
    virtual ~abstract_mutator   (void)                        = 0;

    virtual
    void
    visit (T&) = 0;
  };

}

#endif // OCTAVE_IR_IR_ABSTRACT_MUTATOR_HPP

/** ir-visitor.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_VISITOR_HPP
#define OCTAVE_IR_IR_VISITOR_HPP

namespace gch
{

  //
  // visitor_types
  //

  template <typename ...Visitors>
  struct visitor_types;

  template <typename Inspector, typename Result = void>
  struct inspector;

  template <typename Mutator, typename Result = void>
  struct mutator;

  //
  // abstract_inspector
  //

  template <typename ...Types>
  struct abstract_inspector
    : abstract_inspector<Types>...
  {
    using abstract_inspector<Types>::visit...;
  };

  template <typename T>
  struct abstract_inspector<T>
  {
    abstract_inspector            (void)                          = default;
    abstract_inspector            (const abstract_inspector&)     = default;
    abstract_inspector            (abstract_inspector&&) noexcept = default;
    abstract_inspector& operator= (const abstract_inspector&)     = default;
    abstract_inspector& operator= (abstract_inspector&&) noexcept = default;
    virtual ~abstract_inspector   (void)                          = default;

    virtual
    void
    visit (const T&) = 0;
  };

  //
  // abstract_mutator
  //

  template <typename ...Types>
  struct abstract_mutator
    : abstract_mutator<Types>...
  {
    using abstract_mutator<Types>::visit...;
  };

  template <typename T>
  struct abstract_mutator<T>
  {
    abstract_mutator            (void)                        = default;
    abstract_mutator            (const abstract_mutator&)     = default;
    abstract_mutator            (abstract_mutator&&) noexcept = default;
    abstract_mutator& operator= (const abstract_mutator&)     = default;
    abstract_mutator& operator= (abstract_mutator&&) noexcept = default;
    virtual ~abstract_mutator   (void)                        = default;

    virtual
    void
    visit (T&) = 0;
  };

  //
  // abstract_visitable
  //

  template <typename Visitor>
  struct abstract_acceptor;

  template <typename Inspector, typename Result>
  struct abstract_acceptor<inspector<Inspector, Result>>
  {
    abstract_acceptor            (void)                         = default;
    abstract_acceptor            (const abstract_acceptor&)     = default;
    abstract_acceptor            (abstract_acceptor&&) noexcept = default;
    abstract_acceptor& operator= (const abstract_acceptor&)     = default;
    abstract_acceptor& operator= (abstract_acceptor&&) noexcept = default;
    virtual ~abstract_acceptor   (void)                         = default;

    virtual
    Result
    accept (Inspector&) const = 0;
  };

  template <typename Mutator, typename Result>
  struct abstract_acceptor<mutator<Mutator, Result>>
  {
    abstract_acceptor            (void)                         = default;
    abstract_acceptor            (const abstract_acceptor&)     = default;
    abstract_acceptor            (abstract_acceptor&&) noexcept = default;
    abstract_acceptor& operator= (const abstract_acceptor&)     = default;
    abstract_acceptor& operator= (abstract_acceptor&&) noexcept = default;
    virtual ~abstract_acceptor   (void)                         = default;

    virtual
    Result
    accept (Mutator&) = 0;
  };

  template <typename VisitorTypes>
  struct abstract_visitable;

  template <typename Inspector, typename Result>
  struct abstract_visitable<inspector<Inspector, Result>>
    : virtual abstract_acceptor<inspector<Inspector, Result>>
  {
    using abstract_acceptor<inspector<Inspector, Result>>::accept;
  };

  template <typename Mutator, typename Result>
  struct abstract_visitable<mutator<Mutator, Result>>
    : virtual abstract_acceptor<mutator<Mutator, Result>>
  {
    using abstract_acceptor<mutator<Mutator, Result>>::accept;
  };

  template <typename ...VisitorSubTypes>
  struct abstract_visitable<visitor_types<VisitorSubTypes...>>
    : abstract_visitable<VisitorSubTypes>...
  {
    using abstract_visitable<VisitorSubTypes>::accept...;
  };

  //
  // visitable
  //

  template <typename Derived, typename Visitor>
  struct acceptor;

  template <typename Derived, typename Inspector, typename Result>
  struct acceptor<Derived, inspector<Inspector, Result>>
    : virtual abstract_acceptor<inspector<Inspector, Result>>
  {
    Result
    accept (Inspector& v) const override
    {
      return v.visit (static_cast<const Derived&> (*this));
    }
  };

  template <typename Derived, typename Mutator, typename Result>
  struct acceptor<Derived, mutator<Mutator, Result>>
    : virtual abstract_acceptor<mutator<Mutator, Result>>
  {
    Result
    accept (Mutator& v) override
    {
      return v.visit (static_cast<Derived&> (*this));
    }
  };

  template <typename Derived, typename VisitorTypes>
  struct visitable;

  template <typename Derived, typename Inspector, typename Result>
  struct visitable<Derived, inspector<Inspector, Result>>
    : acceptor<Derived, inspector<Inspector, Result>>
  {
    using acceptor<Derived, inspector<Inspector, Result>>::accept;
  };

  template <typename Derived, typename Mutator, typename Result>
  struct visitable<Derived, mutator<Mutator, Result>>
    : acceptor<Derived, mutator<Mutator, Result>>
  {
    using acceptor<Derived, mutator<Mutator, Result>>::accept;
  };

  template <typename Derived, typename ...VisitorSubTypes>
  struct visitable<Derived, visitor_types<VisitorSubTypes...>>
    : visitable<Derived, VisitorSubTypes>...
  {
    using visitable<Derived, VisitorSubTypes>::accept...;
  };

}

#endif // OCTAVE_IR_IR_VISITOR_HPP

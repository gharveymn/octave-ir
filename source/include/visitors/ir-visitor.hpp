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

  template <typename ...Mutators>
  struct mutator_types;

  template <typename ...Inspectors>
  struct inspector_types;

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
    virtual
    void
    visit (T&) = 0;
  };

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
    virtual
    void
    visit (const T&) = 0;
  };

  //
  // abstract_visitable
  //

  template <typename Visitor>
  struct abstract_acceptor;

  template <typename Mutator>
  struct abstract_acceptor<mutator_types<Mutator>>
  {
    virtual
    void
    accept (Mutator&) = 0;
  };

  template <typename Inspector>
  struct abstract_acceptor<inspector_types<Inspector>>
  {
    virtual
    void
    accept (Inspector&) const = 0;
  };

  template <typename VisitorTypes>
  struct abstract_visitable;

  template <typename ...Mutators>
  struct abstract_visitable<mutator_types<Mutators...>>
    : virtual abstract_acceptor<mutator_types<Mutators>>...
  {
    using abstract_acceptor<mutator_types<Mutators>>::accept...;
  };

  template <typename ...Inspectors>
  struct abstract_visitable<inspector_types<Inspectors...>>
    : virtual abstract_acceptor<inspector_types<Inspectors>>...
  {
    using abstract_acceptor<inspector_types<Inspectors>>::accept...;
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

  template <typename Derived, typename Mutator>
  struct acceptor<Derived, mutator_types<Mutator>>
    : virtual abstract_acceptor<mutator_types<Mutator>>
  {
    void
    accept (Mutator& v) override
    {
      v.visit (static_cast<Derived&> (*this));
    }
  };

  template <typename Derived, typename Inspector>
  struct acceptor<Derived, inspector_types<Inspector>>
    : virtual abstract_acceptor<inspector_types<Inspector>>
  {
    void
    accept (Inspector& v) const override
    {
      v.visit (static_cast<const Derived&> (*this));
    }
  };

  template <typename Derived, typename VisitorTypes>
  struct visitable;

  template <typename Derived, typename ...Mutators>
  struct visitable<Derived, mutator_types<Mutators...>>
    : acceptor<Derived, mutator_types<Mutators>>...
  {
    using acceptor<Derived, mutator_types<Mutators>>::accept...;
  };

  template <typename Derived, typename ...Inspectors>
  struct visitable<Derived, inspector_types<Inspectors...>>
    : acceptor<Derived, inspector_types<Inspectors>>...
  {
    using acceptor<Derived, inspector_types<Inspectors>>::accept...;
  };

  template <typename Derived, typename ...VisitorSubTypes>
  struct visitable<Derived, visitor_types<VisitorSubTypes...>>
    : visitable<Derived, VisitorSubTypes>...
  {
    using visitable<Derived, VisitorSubTypes>::accept...;
  };

  template <typename T, typename Visitor>
  inline
  void
  dispatch (T& t, Visitor& v)
  {
    t.accept (v);
  }

}

#endif // OCTAVE_IR_IR_VISITOR_HPP

/** ir-visitor.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_VISITOR_HPP
#define OCTAVE_IR_IR_VISITOR_HPP

#include "ir-visitor-fwd.hpp"

namespace gch
{

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

  template <typename Inspector>
  struct abstract_acceptor<inspector_type<Inspector>>
  {
  protected:
    abstract_acceptor            (void)                         = default;
    abstract_acceptor            (const abstract_acceptor&)     = default;
    abstract_acceptor            (abstract_acceptor&&) noexcept = default;
    abstract_acceptor& operator= (const abstract_acceptor&)     = default;
    abstract_acceptor& operator= (abstract_acceptor&&) noexcept = default;
    virtual ~abstract_acceptor   (void)                         = 0;

  public:
    using category_wrapper  = inspector_type<Inspector>;
    using visitor_reference = visitor_reference_t<Inspector>;
    using result_type       = visitor_result_t<Inspector>;

    virtual
    result_type
    accept (visitor_reference) const = 0;
  };

  template <typename Inspector>
  inline
  abstract_acceptor<inspector_type<Inspector>>::
  ~abstract_acceptor (void) = default;

  template <typename Mutator>
  struct abstract_acceptor<mutator_type<Mutator>>
  {
  protected:
    abstract_acceptor            (void)                         = default;
    abstract_acceptor            (const abstract_acceptor&)     = default;
    abstract_acceptor            (abstract_acceptor&&) noexcept = default;
    abstract_acceptor& operator= (const abstract_acceptor&)     = default;
    abstract_acceptor& operator= (abstract_acceptor&&) noexcept = default;
    virtual ~abstract_acceptor   (void)                         = 0;

  public:
    using category_wrapper  = mutator_type<Mutator>;
    using visitor_reference = visitor_reference_t<Mutator>;
    using result_type       = visitor_result_t<Mutator>;

    virtual
    result_type
    accept (visitor_reference) = 0;
  };

  template <typename Mutator>
  inline
  abstract_acceptor<mutator_type<Mutator>>::
  ~abstract_acceptor (void) = default;

  template <>
  struct abstract_acceptor<void>
  {
  protected:
    abstract_acceptor            (void)                         = default;
    abstract_acceptor            (const abstract_acceptor&)     = default;
    abstract_acceptor            (abstract_acceptor&&) noexcept = default;
    abstract_acceptor& operator= (const abstract_acceptor&)     = default;
    abstract_acceptor& operator= (abstract_acceptor&&) noexcept = default;
    virtual ~abstract_acceptor   (void);

  public:
    static
    void
    accept (void) = delete;
  };

  template <typename Visitor>
  struct abstract_visitable
    : virtual abstract_acceptor<visitor_category_wrapper_t<Visitor>>
  { };

  template <>
  struct abstract_visitable<visitor_types<>>
    : virtual abstract_acceptor<void>
  {
  protected:
    abstract_visitable            (void)                          = default;
    abstract_visitable            (const abstract_visitable&)     = default;
    abstract_visitable            (abstract_visitable&&) noexcept = default;
    abstract_visitable& operator= (const abstract_visitable&)     = default;
    abstract_visitable& operator= (abstract_visitable&&) noexcept = default;
    ~abstract_visitable           (void) override;
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

  template <typename Derived, typename Inspector>
  struct acceptor<Derived, inspector_type<Inspector>>
    : virtual abstract_acceptor<inspector_type<Inspector>>
  {
    using category_wrapper         = inspector_type<Inspector>;
    using concrete_reference = const Derived&;
    using visitor_reference  = typename abstract_acceptor<category_wrapper>::visitor_reference;
    using result_type        = typename abstract_acceptor<category_wrapper>::result_type;

    result_type
    accept (visitor_reference) const override;
  };

  template <typename Derived, typename Mutator>
  struct acceptor<Derived, mutator_type<Mutator>>
    : virtual abstract_acceptor<mutator_type<Mutator>>
  {
    using category_wrapper         = mutator_type<Mutator>;
    using concrete_reference = Derived&;
    using visitor_reference  = typename abstract_acceptor<category_wrapper>::visitor_reference;
    using result_type        = typename abstract_acceptor<category_wrapper>::result_type;

    result_type
    accept (visitor_reference) override;
  };

  template <typename Derived, typename Visitor>
  struct visitable
    : acceptor<Derived, visitor_category_wrapper_t<Visitor>>
  { };

  template <typename Derived>
  struct visitable<Derived, visitor_types<>>
    : virtual abstract_acceptor<void>
  { };

  template <typename Derived, typename ...VisitorSubTypes>
  struct visitable<Derived, visitor_types<VisitorSubTypes...>>
    : visitable<Derived, VisitorSubTypes>...
  {
    using visitable<Derived, VisitorSubTypes>::accept...;
  };

}

#endif // OCTAVE_IR_IR_VISITOR_HPP
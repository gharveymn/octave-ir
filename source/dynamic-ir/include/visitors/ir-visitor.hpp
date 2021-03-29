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
  // abstract_visitable
  //

  template <typename Visitor>
  class abstract_acceptor;

  template <typename Inspector>
  class abstract_acceptor<inspector_type<Inspector>>
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
  class abstract_acceptor<mutator_type<Mutator>>
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
  class abstract_acceptor<void>
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
  class abstract_visitable
    : public virtual abstract_acceptor<visitor_category_wrapper_t<Visitor>>
  { };

  template <>
  class abstract_visitable<visitor_types<>>
    : public virtual abstract_acceptor<void>
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
  class abstract_visitable<visitor_types<VisitorSubTypes...>>
    : public abstract_visitable<VisitorSubTypes>...
  {
  public:
    using abstract_visitable<VisitorSubTypes>::accept...;
  };

  //
  // visitable
  //

  template <typename Derived, typename Visitor>
  class acceptor;

  template <typename Derived, typename Inspector>
  class acceptor<Derived, inspector_type<Inspector>>
    : public virtual abstract_acceptor<inspector_type<Inspector>>
  {
  public:
    using category_wrapper         = inspector_type<Inspector>;
    using concrete_reference = const Derived&;
    using visitor_reference  = typename abstract_acceptor<category_wrapper>::visitor_reference;
    using result_type        = typename abstract_acceptor<category_wrapper>::result_type;

    result_type
    accept (visitor_reference) const override;
  };

  template <typename Derived, typename Mutator>
  class acceptor<Derived, mutator_type<Mutator>>
    : public virtual abstract_acceptor<mutator_type<Mutator>>
  {
  public:
    using category_wrapper         = mutator_type<Mutator>;
    using concrete_reference = Derived&;
    using visitor_reference  = typename abstract_acceptor<category_wrapper>::visitor_reference;
    using result_type        = typename abstract_acceptor<category_wrapper>::result_type;

    result_type
    accept (visitor_reference) override;
  };

  template <typename Derived, typename Visitor>
  class visitable
    : public acceptor<Derived, visitor_category_wrapper_t<Visitor>>
  { };

  template <typename Derived>
  class visitable<Derived, visitor_types<>>
    : public virtual abstract_acceptor<void>
  { };

  template <typename Derived, typename ...VisitorSubTypes>
  class visitable<Derived, visitor_types<VisitorSubTypes...>>
    : public visitable<Derived, VisitorSubTypes>...
  {
  public:
    using visitable<Derived, VisitorSubTypes>::accept...;
  };

}

#endif // OCTAVE_IR_IR_VISITOR_HPP

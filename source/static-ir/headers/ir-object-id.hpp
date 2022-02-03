/** ir-static-id.hpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_OBJECT_ID_HPP
#define OCTAVE_IR_IR_OBJECT_ID_HPP

#include "ir-utility.hpp"

#include <cstddef>
#include <cstdint>

namespace gch
{

  template <typename Derived, typename T>
  class ir_iterable_id
    : public named_type<T>
  {
  public:
    using named_type<T>::named_type;

    ir_iterable_id (void)
      : named_type<T> { T () }
    { }

    Derived&
    operator++ (void) noexcept
    {
      T val { static_cast<T> (*this) };
      *this = ir_iterable_id { ++val };
      return static_cast<Derived&> (*this);
    }

    Derived
    operator++ (int) noexcept
    {
      Derived ret { static_cast<T> (*this) };
      ++(*this);
      return ret;
    }
  };

  class ir_variable_id
    : public ir_iterable_id<ir_variable_id, std::size_t>
  {
  public:
    using base_type = ir_iterable_id<ir_variable_id, std::size_t>;
    using base_type::ir_iterable_id;
    using base_type::operator++;
  };

  class ir_def_id
    : public ir_iterable_id<ir_def_id, std::size_t>
  {
  public:
    using base_type = ir_iterable_id<ir_def_id, std::size_t>;
    using base_type::ir_iterable_id;
    using base_type::operator++;
  };

  class ir_block_id
    : public ir_iterable_id<ir_block_id, std::size_t>
  {
  public:
    using base_type = ir_iterable_id<ir_block_id, std::size_t>;
    using base_type::ir_iterable_id;
    using base_type::operator++;
  };

}

#endif // OCTAVE_IR_IR_OBJECT_ID_HPP

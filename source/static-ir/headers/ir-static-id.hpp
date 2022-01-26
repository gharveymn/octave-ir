/** ir-static-id.hpp
 * Copyright © 2022 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_STATIC_ID_HPP
#define OCTAVE_IR_IR_STATIC_ID_HPP

#include "ir-utility.hpp"

#include <cstddef>
#include <cstdint>

namespace gch
{

  template <typename T>
  class ir_iterable_id
    : public named_type<T>
  {
  public:
    using named_type<T>::named_type;

    ir_iterable_id&
    operator++ (void) noexcept
    {
      return *this = ir_iterable_id { static_cast<T> (*this) + 1U };
    }

    ir_iterable_id
    operator++ (int) noexcept
    {
      ir_iterable_id ret { *this };
      ++(*this);
      return ret;
    }
  };

  class ir_static_variable_id
    : public ir_iterable_id<std::size_t>
  {
  public:
    using ir_iterable_id<std::size_t>::ir_iterable_id;
  };

  class ir_static_def_id
    : public ir_iterable_id<std::size_t>
  {
  public:
    using ir_iterable_id<std::size_t>::ir_iterable_id;
  };

  class ir_static_block_id
    : public ir_iterable_id<std::size_t>
  {
  public:
    using ir_iterable_id<std::size_t>::ir_iterable_id;
  };

  class ir_processed_id
  {
  public:
    using id_type = std::uint64_t;

    ir_processed_id            (void)                       = default;
    ir_processed_id            (const ir_processed_id&)     = default;
    ir_processed_id            (ir_processed_id&&) noexcept = default;
    ir_processed_id& operator= (const ir_processed_id&)     = default;
    ir_processed_id& operator= (ir_processed_id&&) noexcept = default;
    ~ir_processed_id           (void)                       = default;

    explicit
    ir_processed_id (id_type id);

  private:
    id_type m_id;
  };

}

#endif // OCTAVE_IR_IR_STATIC_ID_HPP

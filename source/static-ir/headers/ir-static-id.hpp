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

  class ir_static_variable_id
    : public named_type<std::size_t>
  {
  public:
    using named_type<std::size_t>::named_type;

    ir_static_variable_id&
    operator++ (void) noexcept
    {
      return *this = ir_static_variable_id { static_cast<value_type> (*this) + 1 };
    }

    ir_static_variable_id
    operator++ (int) noexcept
    {
      ir_static_variable_id ret { *this };
      ++(*this);
      return ret;
    }
  };

  class ir_static_def_id
    : public named_type<std::size_t>
  {
  public:
    using named_type<std::size_t>::named_type;

    ir_static_def_id&
    operator++ (void) noexcept
    {
      return *this = ir_static_def_id { static_cast<value_type> (*this) + 1 };
    }

    ir_static_def_id
    operator++ (int) noexcept
    {
      ir_static_def_id ret { *this };
      ++(*this);
      return ret;
    }
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

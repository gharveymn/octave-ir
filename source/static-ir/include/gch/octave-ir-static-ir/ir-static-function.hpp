/** ir-static-function.hpp
 * A container holding an unordered set of static blocks.
 *
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_STATIC_IR_IR_STATIC_FUNCTION_HPP
#define OCTAVE_IR_STATIC_IR_IR_STATIC_FUNCTION_HPP

#include "gch/octave-ir-utilities/ir-common.hpp"

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

namespace gch
{

  class ir_static_block;
  class ir_static_variable;

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

  class ir_static_function
  {
  public:
    using container_type          = std::vector<ir_static_block>;
    using value_type              = container_type::value_type;
    using allocator_type          = container_type::allocator_type;
    using size_type               = container_type::size_type;
    using difference_type         = container_type::difference_type;
    using reference               = container_type::reference;
    using const_reference         = container_type::const_reference;
    using pointer                 = container_type::pointer;
    using const_pointer           = container_type::const_pointer;

    using iterator                = container_type::iterator;
    using const_iterator          = container_type::const_iterator;
    using reverse_iterator        = container_type::reverse_iterator;
    using const_reverse_iterator  = container_type::const_reverse_iterator;

    using val_t   = value_type;
    using alloc_t = allocator_type;
    using size_ty = size_type;
    using diff_ty = difference_type;
    using ref     = reference;
    using cref    = const_reference;
    using ptr     = pointer;
    using cptr    = const_pointer;

    using iter    = iterator;
    using citer   = const_iterator;
    using riter   = reverse_iterator;
    using criter  = const_reverse_iterator;

    using variables_container_type          = std::vector<ir_static_variable>;
    using variables_value_type              = variables_container_type::value_type;
    using variables_allocator_type          = variables_container_type::allocator_type;
    using variables_size_type               = variables_container_type::size_type;
    using variables_difference_type         = variables_container_type::difference_type;
    using variables_reference               = variables_container_type::reference;
    using variables_const_reference         = variables_container_type::const_reference;
    using variables_pointer                 = variables_container_type::pointer;
    using variables_const_pointer           = variables_container_type::const_pointer;

    using variables_iterator                = variables_container_type::iterator;
    using variables_const_iterator          = variables_container_type::const_iterator;
    using variables_reverse_iterator        = variables_container_type::reverse_iterator;
    using variables_const_reverse_iterator  = variables_container_type::const_reverse_iterator;

    using variables_val_t   = variables_value_type;
    using variables_alloc_t = variables_allocator_type;
    using variables_size_ty = variables_size_type;
    using variables_diff_ty = variables_difference_type;
    using variables_ref     = variables_reference;
    using variables_cref    = variables_const_reference;
    using variables_ptr     = variables_pointer;
    using variables_cptr    = variables_const_pointer;

    using variables_iter    = variables_iterator;
    using variables_citer   = variables_const_iterator;
    using variables_riter   = variables_reverse_iterator;
    using variables_criter  = variables_const_reverse_iterator;

    ir_static_function (void)                        = delete;
    ir_static_function (const ir_static_function&)     = default;
    ir_static_function (ir_static_function&&) noexcept = default;
    ir_static_function& operator= (const ir_static_function&)     = default;
    ir_static_function& operator= (ir_static_function&&) noexcept = default;
    ~ir_static_function (void)                        = default;

    ir_static_function (std::string_view name, ir_processed_id id, container_type&& blocks,
                        std::vector<ir_static_variable>&& vars);

    [[nodiscard]]
    const_iterator
    begin (void) const noexcept;

    [[nodiscard]]
    const_iterator
    end (void) const noexcept;

    [[nodiscard]]
    const_reverse_iterator
    rbegin (void) const noexcept;

    [[nodiscard]]
    const_reverse_iterator
    rend (void) const noexcept;

    [[nodiscard]]
    const_reference
    front (void) const;

    [[nodiscard]]
    const_reference
    back (void) const;

    [[nodiscard]]
    size_type
    size (void) const noexcept;

    [[nodiscard]]
    bool
    empty (void) const noexcept;

    [[nodiscard]]
    size_type
    num_blocks (void) const noexcept;

    [[nodiscard]]
    const_reference
    operator[] (size_type pos) const;

    [[nodiscard]]
    variables_const_iterator
    variables_begin (void) const noexcept;

    [[nodiscard]]
    variables_const_iterator
    variables_end (void) const noexcept;

    [[nodiscard]]
    std::string_view
    get_name (void) const noexcept;

    [[nodiscard]]
    ir_processed_id
    get_id (void) const noexcept;

    [[nodiscard]]
    std::string
    get_block_name (const ir_static_block& block) const;

  private:
    std::string              m_name;
    ir_processed_id          m_id;
    container_type           m_blocks;
    variables_container_type m_variables;
  };

  std::ostream&
  operator<< (std::ostream& out, const ir_static_function& func);

}

#endif // OCTAVE_IR_STATIC_IR_IR_STATIC_FUNCTION_HPP

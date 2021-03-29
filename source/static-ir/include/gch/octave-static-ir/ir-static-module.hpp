/** ir-static-module.hpp
 * A container holding an unordered set of static blocks.
 *
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_STATIC_MODULE_HPP
#define OCTAVE_IR_IR_STATIC_MODULE_HPP

#include <cstdint>
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

  class ir_static_module
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

    ir_static_module            (void)                        = delete;
    ir_static_module            (const ir_static_module&)     = default;
    ir_static_module            (ir_static_module&&) noexcept = default;
    ir_static_module& operator= (const ir_static_module&)     = default;
    ir_static_module& operator= (ir_static_module&&) noexcept = default;
    ~ir_static_module           (void)                        = default;

    ir_static_module (std::string_view name, ir_processed_id id, container_type&& blocks,
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
    const_reference
    operator[] (size_type pos) const;

  private:
    std::string                     m_name;
    ir_processed_id                 m_id;
    container_type                  m_blocks;
    std::vector<ir_static_variable> m_variables;
  };

}

#endif // OCTAVE_IR_IR_STATIC_MODULE_HPP

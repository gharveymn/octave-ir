/** ir-function.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_FUNCTION_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_FUNCTION_HPP

#include "components/ir-structure.hpp"
#include "values/ir-variable.hpp"

#include <unordered_map>

namespace gch
{

  class ir_static_function;

  class ir_function final
    : public ir_component,
      public ir_structure,
      public visitable<ir_function, consolidated_visitors_t<ir_component, ir_structure>>
  {
  public:
    using args_container_type          = small_vector<nonnull_ptr<ir_variable>>;
    using args_value_type              = args_container_type::value_type;
    using args_allocator_type          = args_container_type::allocator_type;
    using args_size_type               = args_container_type::size_type;
    using args_difference_type         = args_container_type::difference_type;
    using args_reference               = args_container_type::reference;
    using args_const_reference         = args_container_type::const_reference;
    using args_pointer                 = args_container_type::pointer;
    using args_const_pointer           = args_container_type::const_pointer;

    using args_iterator                = args_container_type::iterator;
    using args_const_iterator          = args_container_type::const_iterator;
    using args_reverse_iterator        = args_container_type::reverse_iterator;
    using args_const_reverse_iterator  = args_container_type::const_reverse_iterator;

    using args_val_t   = args_value_type;
    using args_alloc_t = args_allocator_type;
    using args_size_ty = args_size_type;
    using args_diff_ty = args_difference_type;
    using args_ref     = args_reference;
    using args_cref    = args_const_reference;
    using args_ptr     = args_pointer;
    using args_cptr    = args_const_pointer;

    using args_iter    = args_iterator;
    using args_citer   = args_const_iterator;
    using args_riter   = args_reverse_iterator;
    using args_criter  = args_const_reverse_iterator;

    ir_function            (void);
    ir_function            (const ir_function&)     = delete;
    ir_function            (ir_function&&) noexcept = default;
    ir_function& operator= (const ir_function&)     = delete;
    ir_function& operator= (ir_function&&) noexcept = delete;
    ~ir_function           (void) override;

    [[nodiscard]]
    ir_subcomponent&
    get_body (void) noexcept;

    [[nodiscard]]
    const ir_subcomponent&
    get_body (void) const noexcept;

    [[nodiscard]]
    bool
    is_body (const ir_subcomponent& sub) const noexcept;

    ir_variable&
    get_variable (const std::string& identifier);

    ir_variable&
    get_variable (std::string&& identifier);

    ir_variable&
    get_variable (std::string_view identifier);

    void
    set_args (args_container_type&& args);

    [[nodiscard]]
    args_iterator
    args_begin (void) noexcept;

    [[nodiscard]]
    args_const_iterator
    args_begin (void) const noexcept;

    [[nodiscard]]
    args_const_iterator
    args_cbegin (void) const noexcept;

    [[nodiscard]]
    args_iterator
    args_end (void) noexcept;

    [[nodiscard]]
    args_const_iterator
    args_end (void) const noexcept;

    [[nodiscard]]
    args_const_iterator
    args_cend (void) const noexcept;

    [[nodiscard]]
    args_reverse_iterator
    args_rbegin (void) noexcept;

    [[nodiscard]]
    args_const_reverse_iterator
    args_rbegin (void) const noexcept;

    [[nodiscard]]
    args_const_reverse_iterator
    args_crbegin (void) const noexcept;

    [[nodiscard]]
    args_reverse_iterator
    args_rend (void) noexcept;

    [[nodiscard]]
    args_const_reverse_iterator
    args_rend (void) const noexcept;

    [[nodiscard]]
    args_const_reverse_iterator
    args_crend (void) const noexcept;

    [[nodiscard]]
    args_reference
    args_front (void);

    [[nodiscard]]
    args_const_reference
    args_front (void) const;

    [[nodiscard]]
    args_reference
    args_back (void);

    [[nodiscard]]
    args_const_reference
    args_back (void) const;

    [[nodiscard]]
    args_size_type
    args_size (void) const noexcept;

    [[nodiscard]]
    bool
    args_empty (void) const noexcept;

    [[nodiscard]]
    args_size_type
    num_args (void) const noexcept;

    [[nodiscard]]
    bool
    has_args (void) const noexcept;

  private:
    std::unordered_map<std::string, ir_variable> m_variable_map;
    small_vector<nonnull_ptr<ir_variable>>       m_args;
    ir_component_storage                         m_body;
  };

  [[nodiscard]]
  ir_block&
  get_entry_block (ir_function& func);

  [[nodiscard]]
  const ir_block&
  get_entry_block (const ir_function& func);

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_FUNCTION_HPP

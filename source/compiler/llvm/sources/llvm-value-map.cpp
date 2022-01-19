/** llvm-value-map.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "llvm-constant.hpp"
#include "llvm-value-map.hpp"

#include "llvm-interface.hpp"

#include "ir-static-block.hpp"
#include "ir-static-def.hpp"
#include "ir-static-function.hpp"
#include "ir-static-operand.hpp"
#include "ir-static-use.hpp"
#include "ir-static-variable.hpp"

namespace gch
{

  //
  // llvm_def_map
  //

  llvm_def_map::
  llvm_def_map (llvm::AllocaInst& alloc, std::size_t num_defs)
    : m_llvm_alloc (alloc),
      m_llvm_defs  (num_defs)
  { }

  llvm::Value&
  llvm_def_map::
  register_def (ir_static_def_id id, llvm::Value& llvm_value)
  {
    assert (! m_llvm_defs[id].has_value ());
    return m_llvm_defs[id].emplace (llvm_value);
  }

  llvm::AllocaInst&
  llvm_def_map::
  get_allocation (void) const noexcept
  {
    return *m_llvm_alloc;
  }

  llvm::Value&
  llvm_def_map::
  operator[] (ir_static_def_id id) const
  {
    assert (ir_undefined_def_id != id);
    assert (m_llvm_defs[id].has_value ());
    return *m_llvm_defs[id];
  }

  llvm_module_interface::
  llvm_module_interface (llvm_module_type& llvm_module)
    : m_llvm_module (llvm_module),
      m_type_map    (generate_ir_type_map<llvm_type_getter_map> (*this)),
      m_true_value  (*invoke_with_context (&llvm::ConstantInt::getTrue)),
      m_false_value (*invoke_with_context (&llvm::ConstantInt::getFalse))
  { }

  llvm::Type&
  llvm_module_interface::
  get_llvm_type (ir_type ty) const
  {
    return *m_type_map[ty];
  }

  auto
  llvm_module_interface::
  get_module (void) noexcept
    -> llvm_module_type&
  {
    return *m_llvm_module;
  }

  auto
  llvm_module_interface::
  get_module (void) const noexcept
    -> const llvm_module_type&
  {
    return *m_llvm_module;
  }

  template <>
  llvm::ConstantInt&
  llvm_module_interface::
  get_bool_constant<true> (void)
  {
    return *m_true_value;
  }

  template <>
  llvm::ConstantInt&
  llvm_module_interface::
  get_bool_constant<false> (void)
  {
    return *m_false_value;
  }

  llvm::ConstantInt&
  llvm_module_interface::
  get_bool_constant (bool b)
  {
    return b ? get_bool_constant<true> () : get_bool_constant<false> ();
  }

  optional_ref<llvm::Function>
  llvm_module_interface::
  get_function (std::string_view name)
  {
    return invoke_with_module (&llvm::Module::getFunction, name);
  }

  //
  // llvm_value_map
  //

  llvm_value_map::
  llvm_value_map (llvm_module_interface module_interface, llvm::Function& llvm_func,
                const ir_static_function& func)
    : llvm_module_interface (module_interface),
      m_llvm_function (llvm_func),
      m_function      (func)
  {
    llvm_ir_builder_type builder { invoke_with_context (
      [](auto& context) { return llvm_ir_builder_type { context }; }) };

    m_blocks.reserve (func.num_blocks ());

    // init entry block
    llvm::BasicBlock& first_block = create_block ("entry");
    m_blocks.emplace_back (&first_block);
    builder.SetInsertPoint (&first_block);

    // init variables
    std::for_each (func.variables_begin (), func.variables_end (),
                   [&](const ir_static_variable& var) {
      llvm::Type& ty   = get_llvm_type (var.get_type ());
      llvm::Twine name = create_twine (var.get_name ());
      m_var_map.try_emplace (nonnull_ptr { var },
                             *builder.CreateAlloca (&ty, nullptr, name),
                             var.get_num_defs ());
    });

    // init blocks
    std::transform (std::next (func.begin ()), func.end (), std::back_inserter (m_blocks),
                    [&](const ir_static_block& block) {
      return &create_block (m_function.get_block_name (block));
    });
  }

  llvm::BasicBlock&
  llvm_value_map::
  operator[] (ir_static_block_id block_id) const
  {
    return *m_blocks[block_id];
  }

  llvm::BasicBlock&
  llvm_value_map::
  operator[] (const ir_static_block& block) const
  {
    using size_ty = block_vector_type::size_type;

    auto off = std::distance (&m_function.front (), &block);

    assert (0 <= off);
    assert (static_cast<std::size_t> (off) < (std::numeric_limits<size_ty>::max) ());

    return (*this)[ir_static_block_id (static_cast<size_ty> (off))];
  }

  llvm::AllocaInst&
  llvm_value_map::
  operator[] (const ir_static_variable& var) const
  {
    auto found = m_var_map.find (nonnull_ptr { var });
    assert (found != m_var_map.end ());
    return found->second.get_allocation ();
  }

  llvm::Value&
  llvm_value_map::
  operator[] (ir_static_use use)
  {
    auto found = m_var_map.find (nonnull_ptr { use.get_variable () });
    assert (found != m_var_map.end ());

    ir_static_def_id id = use.get_def_id ();
    if (ir_undefined_def_id == id)
      return *llvm::UndefValue::get (&get_llvm_type (found->first->get_type ()));

    return found->second[id];
  }

  llvm::Value&
  llvm_value_map::
  operator[] (ir_static_use use) const
  {
    return as_mutable (*this).operator[] (use);
  }

  llvm::Value&
  llvm_value_map::
  operator[] (const ir_static_operand& op)
  {
    if (optional_ref c { maybe_as_constant (op) })
      return get_constant (*this, *c);
    return operator[] (as_use (op));
  }

  llvm::Value&
  llvm_value_map::
  register_def (ir_static_def def, llvm::Value& llvm_value)
  {
    auto found = m_var_map.find (nonnull_ptr { def.get_variable () });
    assert (found != m_var_map.end ());
    return found->second.register_def (def.get_id (), llvm_value);
  }

  llvm::BasicBlock&
  llvm_value_map::
  create_block_before (std::string_view name, llvm::BasicBlock *pos)
  {
    return *invoke_with_context ([&](llvm::LLVMContext& context) {
      return llvm::BasicBlock::Create (context, create_twine (name), &m_llvm_function, pos);
    });
  }

  llvm::BasicBlock&
  llvm_value_map::
  create_block (std::string_view name)
  {
    return create_block_before (name, nullptr);
  }

}

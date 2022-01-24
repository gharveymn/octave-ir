/** llvm-value-map.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_LLVM_VALUE_MAP_HPP
#define OCTAVE_IR_COMPILER_LLVM_LLVM_VALUE_MAP_HPP

#include "llvm-common.hpp"
#include "llvm-type.hpp"
#include "ir-type-util.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/NoFolder.h>

GCH_ENABLE_WARNINGS_MSVC

#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace llvm
{

  class AllocaInst;
  class BasicBlock;
  class ConstantInt;
  class Function;
  class LLVMContext;
  class NoFolder;
  class Type;
  class Value;

}

namespace gch
{

  using llvm_ir_builder_type = llvm::IRBuilder<llvm::NoFolder>;

  class ir_static_block;
  class ir_static_block_id;
  class ir_static_def;
  class ir_static_def_id;
  class ir_static_function;
  class ir_static_operand;
  class ir_static_use;
  class ir_static_variable;

  class llvm_def_map
  {
  public:
    explicit
    llvm_def_map (llvm::AllocaInst& alloc, std::size_t num_defs);

    llvm::Value&
    register_def (ir_static_def_id id, llvm::Value& llvm_value);

    [[nodiscard]]
    llvm::AllocaInst&
    get_allocation (void) const noexcept;

    [[nodiscard]]
    llvm::Value&
    operator[] (ir_static_def_id id) const;

  private:
    nonnull_ptr<llvm::AllocaInst>          m_llvm_alloc;
    std::vector<optional_ref<llvm::Value>> m_llvm_defs;
  };

  class llvm_module_interface
  {
  public:
    using llvm_module_type = llvm::orc::ThreadSafeModule;

    llvm_module_interface (llvm_module_type& llvm_module);

    [[nodiscard]]
    llvm::Type&
    get_llvm_type (ir_type ty) const;

    [[nodiscard]]
    llvm_module_type&
    get_module (void) noexcept;

    [[nodiscard]]
    const llvm_module_type&
    get_module (void) const noexcept;

    template <bool B>
    [[nodiscard]]
    llvm::ConstantInt&
    get_bool_constant (void)
    {
      if constexpr (B)
        return *m_true_value;
      else
        return *m_false_value;
    }

    [[nodiscard]]
    llvm::ConstantInt&
    get_bool_constant (bool b);

    optional_ref<llvm::Function>
    get_function (std::string_view name);

    template <typename Functor, typename ...Args>
    decltype (auto)
    invoke_with_module (Functor&& functor, Args&&... args)
    {
      return get_module ().withModuleDo ([&](llvm::Module& module) -> decltype (auto) {
        return std::invoke (std::forward<Functor> (functor),
                            module,
                            std::forward<Args> (args)...);
      });
    }

    template <typename Functor, typename ...Args>
    decltype (auto)
    invoke_with_module (Functor&& functor, Args&&... args) const
    {
      return get_module ().withModuleDo ([&](const llvm::Module& module) -> decltype (auto) {
        return std::invoke (std::forward<Functor> (functor),
                            module,
                            std::forward<Args> (args)...);
      });
    }

    template <typename Return, typename ...Args>
    decltype (auto)
    invoke_with_context (Return (* f) (llvm::LLVMContext&, Args&&...), Args&&... args) const
    {
      return get_module ().withModuleDo ([&](const llvm::Module& module) -> decltype (auto) {
        return std::invoke (f, module.getContext (), std::forward<Args> (args)...);
      });
    }

    template <typename Functor, typename ...Args>
    decltype (auto)
    invoke_with_context (Functor&& functor, Args&&... args) const
    {
      return get_module ().withModuleDo ([&](const llvm::Module& module) -> decltype (auto) {
        return std::invoke (std::forward<Functor> (functor),
                            module.getContext (),
                            std::forward<Args> (args)...);
      });
    }

    template <typename T>
    [[nodiscard]]
    llvm::Type&
    get_llvm_type (void) const
    {
      return get_llvm_type (ir_type_v<T>);
    }

  private:
    template <typename T>
    struct llvm_type_getter_map
    {
      constexpr
      llvm::Type *
      operator() (const llvm_module_interface& v) const noexcept
      {
        return v.invoke_with_context (llvm_type_function_v<T>);
      }
    };

    nonnull_ptr<llvm_module_type>  m_llvm_module;
    ir_type_map<llvm::Type *>      m_type_map;
    nonnull_ptr<llvm::ConstantInt> m_true_value;
    nonnull_ptr<llvm::ConstantInt> m_false_value;
  };

  class llvm_value_map
      : public llvm_module_interface
  {
  public:
    using block_vector_type = std::vector<llvm::BasicBlock *>;
    using variable_map_type = std::unordered_map<nonnull_cptr<ir_static_variable>, llvm_def_map>;

    llvm_value_map            (void)                      = delete;
    llvm_value_map            (const llvm_value_map&)     = delete;
    llvm_value_map            (llvm_value_map&&) noexcept = delete;
    llvm_value_map& operator= (const llvm_value_map&)     = delete;
    llvm_value_map& operator= (llvm_value_map&&) noexcept = delete;
    ~llvm_value_map           (void)                      = default;

    llvm_value_map (llvm_module_interface module_interface, llvm::Function& llvm_func,
                    const ir_static_function& func);

    llvm::BasicBlock&
    operator[] (ir_static_block_id block_id) const;

    llvm::BasicBlock&
    operator[] (const ir_static_block& block) const;

    llvm::AllocaInst&
    operator[] (const ir_static_variable& var) const;

    llvm::AllocaInst&
    operator[] (ir_static_def def) const;

    llvm::Value&
    operator[] (ir_static_use use);

    llvm::Value&
    operator[] (ir_static_use use) const;

    llvm::Value&
    operator[] (const ir_static_operand& op);

    llvm::Value&
    register_def (ir_static_def def, llvm::Value& llvm_value);

    llvm::BasicBlock&
    create_block_before (std::string_view name, llvm::BasicBlock *pos);

    llvm::BasicBlock&
    create_block (std::string_view name = "");

    using llvm_module_interface::get_llvm_type;

    llvm::Type&
    get_llvm_type (ir_static_def def) const;

    llvm::Twine
    get_variable_name (ir_static_def def) const;

    ir_type
    get_type (ir_static_def def) const;

    ir_type
    get_type (const ir_static_operand& op) const;

  private:
    llvm::Function&           m_llvm_function;
    const ir_static_function& m_function;
    block_vector_type         m_blocks;
    variable_map_type         m_var_map;
  };

}

#endif // OCTAVE_IR_COMPILER_LLVM_LLVM_VALUE_MAP_HPP

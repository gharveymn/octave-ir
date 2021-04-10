/** llvm-value-map.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_LLVM_VALUE_MAP_HPP
#define OCTAVE_IR_COMPILER_LLVM_LLVM_VALUE_MAP_HPP

#include "llvm-common.hpp"
#include "llvm-type.hpp"

#include "gch/octave-ir-static-ir/ir-static-def.hpp"
#include "gch/octave-ir-static-ir/ir-type-util.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/IR/Instructions.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>

GCH_ENABLE_WARNINGS_MSVC

#include <unordered_map>

namespace gch
{

  class ir_constant;
  class ir_static_block_id;
  class ir_static_block;
  class ir_static_def;
  class ir_static_def_id;
  class ir_static_function;
  class ir_static_operand;
  class ir_static_use;

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

  class llvm_value_map
  {
  public:
    using llvm_module_type  = llvm::orc::ThreadSafeModule;

    using type_map_type     = ir_type_map<llvm::Type *>;
    using block_vector_type = std::vector<llvm::BasicBlock *>;
    using variable_map_type = std::unordered_map<nonnull_cptr<ir_static_variable>, llvm_def_map>;

    template <typename Functor, typename ...Args>
    decltype (auto)
    invoke_with_context (Functor&& functor, Args&&... args)
    {
      return m_llvm_module.withModuleDo (
        [&](llvm::Module& module) -> decltype (auto)
        {
          return std::invoke (std::forward<Functor> (functor),
                              module.getContext (),
                              std::forward<Args> (args)...);
        });
    }

    template <typename Functor, typename ...Args>
    decltype (auto)
    invoke_with_context (Functor&& functor, Args&&... args) const
    {
      return m_llvm_module.withModuleDo (
        [&](const llvm::Module& module) -> decltype (auto)
        {
          return std::invoke (std::forward<Functor> (functor),
                              module.getContext (),
                              std::forward<Args> (args)...);
        });
    }

    llvm_value_map            (void)                      = delete;
    llvm_value_map            (const llvm_value_map&)     = delete;
    llvm_value_map            (llvm_value_map&&) noexcept = delete;
    llvm_value_map& operator= (const llvm_value_map&)     = delete;
    llvm_value_map& operator= (llvm_value_map&&) noexcept = delete;
    ~llvm_value_map           (void)                      = default;

    llvm_value_map (llvm_module_type& llvm_module, llvm::Function& llvm_func,
                    const ir_static_function& func);

    llvm::Type&
    operator[] (ir_type ty) const;

    llvm::BasicBlock&
    operator[] (ir_static_block_id block_id) const;

    llvm::BasicBlock&
    operator[] (const ir_static_block& block) const;

    llvm::AllocaInst&
    operator[] (const ir_static_variable& var) const;

    llvm::Value&
    operator[] (const ir_constant& c) const;

    llvm::Value&
    operator[] (ir_static_use use) const;

    llvm::Value&
    operator[] (const ir_static_operand& op) const;

    llvm::Value&
    register_def (ir_static_def def, llvm::Value& llvm_value);

    template <typename T>
    llvm::Type *
    get_llvm_type (void) const
    {
      return llvm_type_function_v<T> (*m_llvm_module.getContext ().getContext ());
    }

  private:
    template <typename T>
    struct llvm_type_getter_map
    {
      constexpr
      llvm::Type *
      operator() (const llvm_value_map& v) const noexcept
      {
        return v.get_llvm_type<T> ();
      }
    };

    llvm_module_type&         m_llvm_module;
    llvm::Function&           m_llvm_function;
    const ir_static_function& m_function;
    type_map_type             m_type_map;
    block_vector_type         m_blocks;
    variable_map_type         m_var_map;
  };

}

#endif // OCTAVE_IR_COMPILER_LLVM_LLVM_VALUE_MAP_HPP

/** llvm-interface.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_LLVM_INTERFACE_HPP
#define OCTAVE_IR_COMPILER_LLVM_LLVM_INTERFACE_HPP

#include "llvm-common.hpp"
#include "llvm-version.hpp"

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/EPCIndirectionUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/IRTransformLayer.h>
#include <llvm/ExecutionEngine/Orc/Layer.h>
#include <llvm/ExecutionEngine/Orc/Mangling.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Mangler.h>
#include <llvm/IR/Module.h>

#if GCH_LLVM_HAS_DEFAULT_ORCV2
#  include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#endif

GCH_ENABLE_WARNINGS_MSVC

namespace gch
{

  class ir_static_function;
  class llvm_interface;

  llvm::orc::ThreadSafeModule
  create_llvm_module (const llvm::DataLayout& data_layout, const ir_static_function& func,
                      const std::string& suffix);

  class llvm_interface
  {
    using resource_tracker_type = llvm::orc::JITDylib;

    class ast_layer
    {
      class materialization_unit : public llvm::orc::MaterializationUnit
      {
      public:
        materialization_unit (ast_layer& ast_layer, const ir_static_function& func);

        [[nodiscard]]
        llvm::StringRef
        getName (void) const override
        {
          return "gch::llvm_interface::ast_layer::materialization_unit";
        }

        void
        materialize (std::unique_ptr<llvm::orc::MaterializationResponsibility> resp) override;

      private:
        void
        discard (const resource_tracker_type&, const llvm::orc::SymbolStringPtr&) override
        {
          llvm_unreachable("octave-ir functions are not overridable");
        }

        ast_layer&                m_ast_layer;
        const ir_static_function& m_function;
      };

    public:
      ast_layer (llvm::orc::IRLayer& base_layer, const llvm::DataLayout& data_layout);

      llvm::Error
      add (llvm::orc::ResourceTrackerSP res_tracker, const ir_static_function& func);

      void
      emit (std::unique_ptr<llvm::orc::MaterializationResponsibility> resp,
            const ir_static_function& func);

      llvm::orc::SymbolFlagsMap
      get_interface (const ir_static_function& func);

    private:
      llvm::orc::IRLayer&     m_base_layer;
      const llvm::DataLayout& m_data_layout;
    };

  public:
    using object_layer_type   = llvm::orc::RTDyldObjectLinkingLayer;
    using compile_layer_type  = llvm::orc::IRCompileLayer;

    llvm_interface (std::unique_ptr<llvm::orc::ExecutionSession> execution_session,
                    std::unique_ptr<llvm::orc::EPCIndirectionUtils> epc_indirection_utils,
                    llvm::orc::JITTargetMachineBuilder&& jit_builder,
                    const llvm::DataLayout& data_layout);

    llvm_interface            (void)                      = delete;
    llvm_interface            (const llvm_interface&)     = delete;
    llvm_interface            (llvm_interface&&) noexcept = delete;
    llvm_interface& operator= (const llvm_interface&)     = delete;
    llvm_interface& operator= (llvm_interface&&) noexcept = delete;
    ~llvm_interface (void);

    const llvm::DataLayout&
    get_data_layout (void) const noexcept;

    llvm::orc::JITDylib&
    get_jit_dylib (void) const noexcept;

    llvm::Error
    add_module (llvm::orc::ThreadSafeModule&& module,
                llvm::orc::ResourceTrackerSP res_tracker = nullptr);

    llvm::Error
    add_ast (const ir_static_function& func, llvm::orc::ResourceTrackerSP res_tracker = nullptr);

    llvm::Expected<llvm::JITEvaluatedSymbol>
    find_symbol (std::string_view name);

    static
    llvm::Expected<std::unique_ptr<llvm_interface>>
    create (void);

  private:
    static
    std::unique_ptr<llvm::SectionMemoryManager>
    create_memory_manager (void);

    static
    std::unique_ptr<llvm::orc::ConcurrentIRCompiler>
    create_compiler (llvm::orc::JITTargetMachineBuilder&& jit_builder);

    static
    llvm::Expected<llvm::orc::ThreadSafeModule>
    optimize_module (llvm::orc::ThreadSafeModule module,
                     const llvm::orc::MaterializationResponsibility& resp);

    static
    void
    handle_lazy_call_through_error (void);

    std::unique_ptr<llvm::orc::ExecutionSession>    m_execution_session;
    std::unique_ptr<llvm::orc::EPCIndirectionUtils> m_epc_indirection_utils;
    const llvm::DataLayout                          m_data_layout;
    llvm::orc::MangleAndInterner                    m_mangler;
    object_layer_type                               m_object_layer;
    compile_layer_type                              m_compile_layer;
    llvm::orc::IRTransformLayer                     m_optimization_layer;
    ast_layer                                       m_ast_layer;

    llvm::orc::JITDylib&                            m_jit_dylib;
  };

}

#endif // OCTAVE_IR_COMPILER_LLVM_LLVM_INTERFACE_HPP

// #if LLVM_ENABLE_ABI_BREAKING_CHECKS
// #  error blah
// #endif

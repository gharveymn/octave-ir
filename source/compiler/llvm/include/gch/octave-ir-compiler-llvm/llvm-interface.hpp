/** llvm-interface.hpp.h
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_LLVM_LLVM_INTERFACE_HPP
#define OCTAVE_IR_COMPILER_LLVM_LLVM_INTERFACE_HPP

#include "llvm-version.hpp"

#ifdef _MSC_VER
#  pragma warning (push, 0)
#endif

#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
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

#ifdef _MSC_VER
#  pragma warning (pop)
#endif

namespace gch
{

  class llvm_interface
  {
    using resource_tracker_type = llvm::orc::JITDylib;

  public:
    using object_layer_type   = llvm::orc::RTDyldObjectLinkingLayer;
    using compile_layer_type  = llvm::orc::IRCompileLayer;

    llvm_interface            (void);
    llvm_interface            (const llvm_interface&)     = delete;
    llvm_interface            (llvm_interface&&) noexcept = delete;
    llvm_interface& operator= (const llvm_interface&)     = delete;
    llvm_interface& operator= (llvm_interface&&) noexcept = delete;
    ~llvm_interface           (void)                      = default;

    llvm_interface (llvm::orc::JITTargetMachineBuilder&& jit_builder,
                    std::shared_ptr<llvm::orc::SymbolStringPool>&& symbol_pool);

    llvm::TargetMachine&
    get_target_machine (void) const;

    const llvm::DataLayout&
    get_data_layout (void) const noexcept;

    llvm::orc::JITDylib&
    get_jit_dylib (void) const noexcept;

    llvm::Error
    add_module (llvm::orc::ThreadSafeModule&& module);

    llvm::Expected<llvm::JITEvaluatedSymbol>
    find_symbol (std::string_view name);

  private:
    static
    std::unique_ptr<llvm::SectionMemoryManager>
    create_memory_manager (void);

    static
    std::unique_ptr<llvm::orc::ConcurrentIRCompiler>
    create_compiler (llvm::orc::JITTargetMachineBuilder&& jit_builder);

    resource_tracker_type&
    get_resource_tracker (void);

    llvm::orc::ExecutionSession          m_execution_session;
    std::unique_ptr<llvm::TargetMachine> m_target_machine;
    const llvm::DataLayout               m_data_layout;
    llvm::orc::MangleAndInterner         m_mangler;
    llvm::orc::JITDylib&                 m_jit_dylib;
    object_layer_type                    m_object_layer;
    compile_layer_type                   m_compile_layer;
  };

}

#endif // OCTAVE_IR_COMPILER_LLVM_LLVM_INTERFACE_HPP

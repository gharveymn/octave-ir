/** llvm-interface.cpp.c
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-ir-compiler-llvm/llvm-interface.hpp"

namespace gch
{

  llvm_interface::
  llvm_interface (void)
    : llvm_interface (cantFail (llvm::orc::JITTargetMachineBuilder::detectHost ()),
                      std::make_shared<llvm::orc::SymbolStringPool> ())
  { }

  llvm_interface::
  llvm_interface (llvm::orc::JITTargetMachineBuilder&& jit_builder,
                  std::shared_ptr<llvm::orc::SymbolStringPool>&& symbol_pool)
    : m_execution_session (std::move (symbol_pool)),
      m_target_machine    (llvm::cantFail (jit_builder.createTargetMachine ())),
      m_data_layout       (m_target_machine->createDataLayout ()),
      m_mangler           (m_execution_session, m_data_layout),
      m_jit_dylib         (m_execution_session.createBareJITDylib ("<main>")),
      m_object_layer      (m_execution_session, create_memory_manager),
      m_compile_layer     (m_execution_session, m_object_layer,
                           create_compiler (std::move (jit_builder)))
  {
    m_jit_dylib.addGenerator (llvm::cantFail (
      llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess (
        m_data_layout.getGlobalPrefix ()))
    );
  }

  llvm::TargetMachine&
  llvm_interface::
  get_target_machine (void) const
  {
    return *m_target_machine;
  }

  const llvm::DataLayout&
  llvm_interface::
  get_data_layout (void) const noexcept
  {
    return m_data_layout;
  }

  llvm::orc::JITDylib&
  llvm_interface::
  get_jit_dylib (void) const noexcept
  {
    return m_jit_dylib;
  }

  llvm::Error
  llvm_interface::
  add_module (llvm::orc::ThreadSafeModule&& module)
  {
    return m_compile_layer.add (get_resource_tracker (), std::move (module));
  }

  llvm::Expected<llvm::JITEvaluatedSymbol>
  llvm_interface::
  find_symbol (std::string_view name)
  {
    return m_execution_session.lookup ({ &get_jit_dylib () }, m_mangler (name.data ()));
  }

  std::unique_ptr<llvm::SectionMemoryManager>
  llvm_interface::
  create_memory_manager (void)
  {
    return std::make_unique<llvm::SectionMemoryManager> ();
  }

  std::unique_ptr<llvm::orc::ConcurrentIRCompiler>
  llvm_interface::
  create_compiler (llvm::orc::JITTargetMachineBuilder&& jit_builder)
  {
    return std::make_unique<llvm::orc::ConcurrentIRCompiler> (std::move (jit_builder));
  }

  auto
  llvm_interface::
  get_resource_tracker (void)
    -> resource_tracker_type&
  {
    return m_jit_dylib;
  }

}

/** llvm-interface.cpp.c
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "llvm-interface.hpp"
#include "ir-static-function.hpp"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar.h>

#include <iostream>

namespace gch
{

  llvm_interface::ast_layer::materialization_unit::
  materialization_unit (ast_layer& ast_layer, const ir_static_function& func)
    : MaterializationUnit (ast_layer.get_interface (func), nullptr),
      m_ast_layer (ast_layer),
      m_function (func)
  { }

  llvm::StringRef
  llvm_interface::ast_layer::materialization_unit::
  getName (void) const
  {
    return "gch::llvm_interface::ast_layer::materialization_unit";
  }

  void
  llvm_interface::ast_layer::materialization_unit::
  materialize (std::unique_ptr<llvm::orc::MaterializationResponsibility> resp)
  {
    m_ast_layer.emit (std::move (resp), m_function);
  }

  void
  llvm_interface::ast_layer::materialization_unit::
  discard (const resource_tracker_type&, const llvm::orc::SymbolStringPtr&)
  {
    llvm_unreachable ("octave-ir functions are not overridable");
  }

  llvm_interface::ast_layer::
  ast_layer (llvm::orc::IRLayer& base_layer, const llvm::DataLayout& data_layout, bool printing)
    : m_base_layer (base_layer),
      m_data_layout (data_layout),
      m_printing_enabled (printing)
  { }

  llvm::Error
  llvm_interface::ast_layer::
  add (const ir_static_function& func, llvm::orc::ResourceTrackerSP res_tracker)
  {
    return res_tracker->getJITDylib ().define (
      std::make_unique<materialization_unit> (*this, func),
      res_tracker);
  }

  void
  llvm_interface::ast_layer::
  emit (std::unique_ptr<llvm::orc::MaterializationResponsibility> resp,
        const ir_static_function& func)
  {
    llvm::orc::ThreadSafeModule tsm = create_llvm_module (m_data_layout, func);
    if (m_printing_enabled)
    {
      tsm.withModuleDo ([&](llvm::Module& module) { module.print (llvm::outs (), nullptr); });
      std::cout << std::endl;
    }
    m_base_layer.emit (std::move (resp), std::move (tsm));
  }

  llvm::orc::SymbolFlagsMap
  llvm_interface::ast_layer::
  get_interface (const ir_static_function& func)
  {
    llvm::orc::MangleAndInterner mangler (m_base_layer.getExecutionSession (), m_data_layout);
    llvm::orc::SymbolFlagsMap syms;
    syms[mangler (func.get_name ().data ())] =
      llvm::JITSymbolFlags (llvm::JITSymbolFlags::Exported | llvm::JITSymbolFlags::Callable);
    return syms;
  }

  void
  llvm_interface::ast_layer::
  enable_printing (bool printing)
  {
    m_printing_enabled = printing;
  }

  llvm_interface::
  llvm_interface (std::unique_ptr<llvm::orc::ExecutionSession> execution_session,
                  std::unique_ptr<llvm::orc::EPCIndirectionUtils> epc_indirection_utils,
                  llvm::orc::JITTargetMachineBuilder&& jit_builder,
                  const llvm::DataLayout& data_layout)
    : m_execution_session     (std::move (execution_session)),
      m_epc_indirection_utils (std::move (epc_indirection_utils)),
      m_data_layout           (data_layout),
      m_mangler               (*m_execution_session, m_data_layout),
      m_object_layer          (*m_execution_session, create_memory_manager),
      m_compile_layer         (*m_execution_session, m_object_layer,
                               create_compiler (std::move (jit_builder))),
      m_optimization_layer    (*m_execution_session, m_compile_layer, optimize_module),
      m_ast_layer             (m_optimization_layer, m_data_layout),
      m_jit_dylib             (m_execution_session->createBareJITDylib ("<main>"))
  {
    m_jit_dylib.addGenerator (llvm::cantFail (
      llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess (
        m_data_layout.getGlobalPrefix ()))
    );
  }

  llvm_interface::
  ~llvm_interface (void)
  {
    // FIXME: Is this out of order?
    if (auto err = m_execution_session->endSession ())
      m_execution_session->reportError (std::move(err));
    if (auto err = m_epc_indirection_utils->cleanup ())
      m_execution_session->reportError(std::move (err));
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
  add_module (llvm::orc::ThreadSafeModule&& module, llvm::orc::ResourceTrackerSP res_tracker)
  {
    if (! res_tracker)
      res_tracker = m_jit_dylib.getDefaultResourceTracker ();

    return m_optimization_layer.add(res_tracker, std::move (module));
  }

  llvm::Error
  llvm_interface::
  add_ast (const ir_static_function& func, llvm::orc::ResourceTrackerSP res_tracker)
  {
    if (! res_tracker)
      res_tracker = m_jit_dylib.getDefaultResourceTracker ();

    return m_ast_layer.add (func, res_tracker);
  }

  llvm::Expected<llvm::JITEvaluatedSymbol>
  llvm_interface::
  find_symbol (std::string_view name)
  {
    return m_execution_session->lookup ({ &get_jit_dylib () }, m_mangler (name.data ()));
  }

  llvm::Expected<std::unique_ptr<llvm_interface>>
  llvm_interface::
  create (void)
  {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    auto executor_process_control = llvm::orc::SelfExecutorProcessControl::Create ();
    if (! executor_process_control)
      return executor_process_control.takeError ();

    auto execution_session = std::make_unique<llvm::orc::ExecutionSession> (
      std::move (*executor_process_control));

    if (! execution_session)
      return nullptr;

    auto epc_indirection_utils = llvm::orc::EPCIndirectionUtils::Create (
      execution_session->getExecutorProcessControl ());

    if (! epc_indirection_utils)
      return epc_indirection_utils.takeError();

    (*epc_indirection_utils)->createLazyCallThroughManager(
      *execution_session, llvm::pointerToJITTargetAddress (&handle_lazy_call_through_error));

    if (llvm::Error err = setUpInProcessLCTMReentryViaEPCIU (**epc_indirection_utils))
      return err;

    llvm::orc::JITTargetMachineBuilder jit_builder (
      execution_session->getExecutorProcessControl ().getTargetTriple ());

    auto data_layout = jit_builder.getDefaultDataLayoutForTarget();
    if (! data_layout)
      return data_layout.takeError ();

    auto interface = std::make_unique<llvm_interface> (
      std::move (execution_session),
      std::move (*epc_indirection_utils),
      std::move (jit_builder),
      *data_layout);

    return interface;
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

  llvm::Expected<llvm::orc::ThreadSafeModule>
  llvm_interface::
  optimize_module (llvm::orc::ThreadSafeModule module,
                   const llvm::orc::MaterializationResponsibility&)
  {
    module.withModuleDo ([](llvm::Module& mod) {
      // Create a function pass manager.
      llvm::legacy::FunctionPassManager function_pass_manager (&mod);

      // Add some optimizations.
      function_pass_manager.add (llvm::createInstructionCombiningPass ());
      function_pass_manager.add (llvm::createReassociatePass ());
      function_pass_manager.add (llvm::createGVNPass ());
      function_pass_manager.add (llvm::createCFGSimplificationPass ());
      function_pass_manager.doInitialization ();

      // Run the optimizations over all functions in the module being added to
      // the JIT.
      std::for_each (mod.begin (), mod.end (), [&](llvm::Function& func) {
        function_pass_manager.run (func);
      });
    });

    return module;
  }

  void
  llvm_interface::
  handle_lazy_call_through_error (void)
  {
    std::cerr << "LazyCallThrough error: Could not find function body" << std::endl;
    exit (1);
  }

  void
  llvm_interface::
  enable_printing (bool printing)
  {
    m_ast_layer.enable_printing (printing);
  }

}

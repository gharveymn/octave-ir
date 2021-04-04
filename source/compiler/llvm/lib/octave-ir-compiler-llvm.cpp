/** octave-ir-compiler-llvm.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-ir-compiler.hpp"
#include "gch/octave-ir-static-ir/ir-static-block.hpp"
#include "gch/octave-ir-static-ir/ir-static-instruction.hpp"
#include "gch/octave-ir-static-ir/ir-static-variable.hpp"
#include "gch/octave-ir-static-ir/ir-type-util.hpp"

#include <gch/nonnull_ptr.hpp>

#include <llvm/Config/llvm-config.h>

#ifndef LLVM_VERSION_PATCH
#  define LLVM_VERSION_PATCH 0
#endif

#define GCH_LLVM_VERSION_MAJOR_NUM(MAJOR) ((MAJOR) * 010000L)
#define GCH_LLVM_VERSION_MINOR_NUM(MINOR) ((MINOR) * 000100L)
#define GCH_LLVM_VERSION_PATCH_NUM(PATCH) ((PATCH) * 000001L)

#define GCH_LLVM_VERSION_NUM(MAJOR, MINOR, PATCH) \
  GCH_LLVM_VERSION_MAJOR_NUM (MAJOR)              \
+ GCH_LLVM_VERSION_MINOR_NUM (MINOR)              \
+ GCH_LLVM_VERSION_PATCH_NUM (PATCH)

#ifndef GCH_LLVM_VERSION
#  define GCH_LLVM_VERSION GCH_LLVM_VERSION_NUM (LLVM_VERSION_MAJOR, \
                                                 LLVM_VERSION_MINOR, \
                                                 LLVM_VERSION_PATCH)
#endif

#define GCH_LLVM_VERSION_EQUAL(MAJOR, MINOR, PATCH) \
(GCH_LLVM_VERSION == GCH_LLVM_VERSION_NUM (MAJOR, MINOR, PATCH))

#define GCH_LLVM_VERSION_LESS(MAJOR, MINOR, PATCH) \
(GCH_LLVM_VERSION < GCH_LLVM_VERSION_NUM (MAJOR, MINOR, PATCH))

#define GCH_LLVM_VERSION_GREATER_EQUAL(MAJOR, MINOR, PATCH) \
(! GCH_LLVM_VERSION_LESS (MAJOR, MINOR, PATCH))

#define GCH_LLVM_VERSION_GREATER(MAJOR, MINOR, PATCH) \
(GCH_LLVM_VERSION_NUM (MAJOR, MINOR, PATCH) < GCH_LLVM_VERSION)

#define GCH_LLVM_VERSION_LESS_EQUAL(MAJOR, MINOR, PATCH) \
(! GCH_LLVM_VERSION_GREATER (MAJOR, MINOR, PATCH))

#define GCH_LLVM_VERSION_MAJOR_EQUAL(MAJOR) \
GCH_LLVM_VERSION_EQUAL (MAJOR, 00, 00)

#define GCH_LLVM_VERSION_MAJOR_LESS(MAJOR) \
GCH_LLVM_VERSION_LESS (MAJOR, 00, 00)

#define GCH_LLVM_VERSION_MAJOR_GREATER_EQUAL(MAJOR) \
GCH_LLVM_VERSION_GREATER_EQUAL (MAJOR, 00, 00)

#define GCH_LLVM_VERSION_MAJOR_GREATER(MAJOR) \
GCH_LLVM_VERSION_GREATER (MAJOR, 00, 00) < GCH_LLVM_VERSION

#define GCH_LLVM_VERSION_MAJOR_LESS_EQUAL(MAJOR) \
GCH_LLVM_VERSION_LESS_EQUAL (MAJOR, 00, 00)

#define GCH_LLVM_HAS_ORC           GCH_LLVM_VERSION_GREATER_EQUAL (3, 7, 0)
#define GCH_LLVM_HAS_ORCV2         GCH_LLVM_VERSION_MAJOR_GREATER_EQUAL (7)
#define GCH_LLVM_HAS_DEFAULT_ORCV2 GCH_LLVM_VERSION_MAJOR_GREATER_EQUAL (8)

#ifdef _MSC_VER
#  pragma warning (push, 0)
#endif

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

#include <unordered_map>
#include <vector>

namespace gch
{

  class llvm_interface
  {
    static
    auto
    create_memory_manager (void)
    {
      return std::make_unique<llvm::SectionMemoryManager> ();
    }

    static
    auto
    create_compiler (llvm::orc::JITTargetMachineBuilder&& jit_builder)
    {
      return std::make_unique<llvm::orc::ConcurrentIRCompiler> (std::move (jit_builder));
    }

    decltype (auto)
    get_resource_tracker (void)
    {
      return m_jit_dylib;
    }

  public:
    using object_layer_type   = llvm::orc::RTDyldObjectLinkingLayer;
    using compile_layer_type  = llvm::orc::IRCompileLayer;

//  llvm_interface            (void)                      = impl;
    llvm_interface            (const llvm_interface&)     = delete;
    llvm_interface            (llvm_interface&&) noexcept = delete;
    llvm_interface& operator= (const llvm_interface&)     = delete;
    llvm_interface& operator= (llvm_interface&&) noexcept = delete;
//  ~llvm_interface           (void)                      = impl;

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
    get_target_machine (void) const
    {
      return *m_target_machine;
    }

    const llvm::DataLayout&
    get_data_layout (void) const noexcept
    {
      return m_data_layout;
    }

    llvm::orc::JITDylib&
    get_jit_dylib (void) const noexcept
    {
      return m_jit_dylib;
    }

    llvm::Error
    add_module (llvm::orc::ThreadSafeModule&& module)
    {
      return m_compile_layer.add (get_resource_tracker (), std::move (module));
    }

    llvm::Expected<llvm::JITEvaluatedSymbol>
    find_symbol (std::string_view name)
    {
      return m_execution_session.lookup ({ &get_jit_dylib () }, m_mangler (name));
    }

  private:
    llvm::orc::ExecutionSession          m_execution_session;
    std::unique_ptr<llvm::TargetMachine> m_target_machine;
    const llvm::DataLayout               m_data_layout;
    llvm::orc::MangleAndInterner         m_mangler;
    llvm::orc::JITDylib&                 m_jit_dylib;
    object_layer_type                    m_object_layer;
    compile_layer_type                   m_compile_layer;
  };

  static
  llvm::Twine
  create_twine (std::string_view view)
  {
    return { view.data () };
  }

  template <typename T, typename Enable = void>
  struct llvm_type_function
  {
    static constexpr
    auto
    value = [](llvm::LLVMContext&) { return nullptr; };
  };

  template <>
  struct llvm_type_function<void>
  {
    static constexpr
    auto
    value = &llvm::Type::getVoidTy;
  };

  template <>
  struct llvm_type_function<double>
  {
    static constexpr
    auto
    value = &llvm::Type::getDoubleTy;
  };

  template <>
  struct llvm_type_function<single>
  {
    static constexpr
    auto
    value = &llvm::Type::getFloatTy;
  };

  template <>
  struct llvm_type_function<int64_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt64Ty;
  };

  template <>
  struct llvm_type_function<int32_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt32Ty;
  };

  template <>
  struct llvm_type_function<int16_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt16Ty;
  };

  template <>
  struct llvm_type_function<int8_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt8Ty;
  };

  template <>
  struct llvm_type_function<uint64_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt64Ty;
  };

  template <>
  struct llvm_type_function<uint32_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt32Ty;
  };

  template <>
  struct llvm_type_function<uint16_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt16Ty;
  };

  template <>
  struct llvm_type_function<uint8_t>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt8Ty;
  };

  template <>
  struct llvm_type_function<char>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt8Ty;
  };

  template <>
  struct llvm_type_function<bool>
  {
    static constexpr
    auto
    value = &llvm::Type::getInt1Ty;
  };

  template <typename Scalar>
  struct llvm_type_function<Scalar, std::enable_if_t<std::is_arithmetic_v<Scalar>>>
  {
    static constexpr
    auto
    value = &llvm::Type::getScalarTy<Scalar>;
  };

  template <typename T>
  static constexpr
  auto
  llvm_type_function_v = llvm_type_function<T>::value;

  class llvm_def_map
  {
  public:
    explicit
    llvm_def_map (llvm::AllocaInst& alloc, std::size_t num_defs)
      : m_llvm_alloc (alloc),
        m_llvm_defs  (num_defs)
    { }

    llvm::AllocaInst&
    get_allocation (void) const noexcept
    {
      return *m_llvm_alloc;
    }

    llvm::Value&
    register_def (ir_static_def_id id, llvm::Value& llvm_value)
    {
      assert (! m_llvm_defs[id].has_value ());
      return m_llvm_defs[id].emplace (llvm_value);
    }

    llvm::Value&
    operator[] (ir_static_def_id id) const
    {
      assert (m_llvm_defs[id].has_value ());
      return *m_llvm_defs[id];
    }

  private:
    nonnull_ptr<llvm::AllocaInst>          m_llvm_alloc;
    std::vector<optional_ref<llvm::Value>> m_llvm_defs;
  };

  class llvm_value_map
  {
  public:
    using llvm_module_type  = llvm::orc::ThreadSafeModule;

    using type_map_type     = ir_type_map<llvm::Type *>;
    using block_vector_type = std::vector<nonnull_ptr<llvm::BasicBlock>>;
    using variable_map_type = std::unordered_map<nonnull_cptr<ir_static_variable>, llvm_def_map>;

    llvm_value_map            (void)                      = delete;
    llvm_value_map            (const llvm_value_map&)     = delete;
    llvm_value_map            (llvm_value_map&&) noexcept = delete;
    llvm_value_map& operator= (const llvm_value_map&)     = delete;
    llvm_value_map& operator= (llvm_value_map&&) noexcept = delete;
    ~llvm_value_map           (void)                      = default;

    llvm_value_map (llvm_module_type& llvm_module, const ir_static_unit& unit,
                    llvm::Function& function)
      : m_llvm_module (llvm_module),
        m_module      (unit),
        m_function    (function),
        m_type_map    (template_generate_ir_type_map<llvm_type_getter_map> (*this))
    {
      using namespace llvm;

      IRBuilder<> builder (get_llvm_context ());

      m_blocks.reserve (unit.num_blocks ());

      // init entry block
      BasicBlock& first_block = *BasicBlock::Create (get_llvm_context (), "entry", &m_function);
      m_blocks.emplace_back (first_block);
      builder.SetInsertPoint (&first_block);

      // init variables
      std::for_each (unit.variables_begin (), unit.variables_end (),
                      [&](const ir_static_variable& var)
                      {
                        Type& ty   = (*this)[var.get_type ()];
                        const char *name = var.get_name ().data ();

                        m_var_map.try_emplace (nonnull_ptr { var },
                                               *builder.CreateAlloca (&ty, nullptr, name),
                                               var.get_num_defs ());
                      });

      // init blocks
      std::transform (std::next (unit.begin ()), unit.end (), std::back_inserter (m_blocks),
                      [&](const ir_static_block& block)
                      {
                        return nonnull_ptr { *BasicBlock::Create (get_llvm_context (),
                                                                  m_module.get_block_name (block),
                                                                  &m_function) };
                      });
    }

    llvm::LLVMContext&
    get_llvm_context (void) const
    {
      return *m_llvm_module.getContext ().getContext ();
    }

    llvm::Type&
    operator[] (ir_type ty) const
    {
      return *m_type_map[ty];
    }

    llvm::BasicBlock&
    operator[] (ir_static_block_id block_id) const
    {
      return *m_blocks[block_id];
    }

    llvm::BasicBlock&
    operator[] (const ir_static_block& block) const
    {
      using size_ty = block_vector_type::size_type;

      auto off = std::distance (&m_module.front (), &block);

      assert (0 <= off);
      assert (static_cast<std::size_t> (off) < (std::numeric_limits<size_ty>::max) ());

      return (*this)[static_cast<ir_static_block_id> (off)];
    }

    llvm::AllocaInst&
    operator[] (const ir_static_variable& var) const
    {
      auto found = m_var_map.find (nonnull_ptr { var });
      assert (found != m_var_map.end ());
      return found->second.get_allocation ();
    }

    llvm::Value&
    operator[] (ir_static_use use) const
    {
      auto found = m_var_map.find (nonnull_ptr { use.get_variable () });
      assert (found != m_var_map.end ());
      return found->second[use.get_def_id ()];
    }

    template <typename T>
    llvm::Type *
    get_llvm_type (void) const
    {
      return llvm_type_function_v<T> (get_llvm_context ());
    }

    llvm::Value&
    register_def (ir_static_def def, llvm::Value& llvm_value)
    {
      auto found = m_var_map.find (nonnull_ptr { def.get_variable () });
      assert (found != m_var_map.end ());
      return found->second.register_def (def.get_id (), llvm_value);
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


    llvm_module_type&       m_llvm_module;
    const ir_static_unit& m_module;
    llvm::Function&         m_function;
    type_map_type           m_type_map;
    block_vector_type       m_blocks;
    variable_map_type       m_var_map;
  };

  template <ir_opcode Op>
  struct instruction_translator
  {
    static constexpr ir_opcode opcode = Op;

    static
    llvm::Instruction&
    translate (const ir_static_instruction& instr, llvm::IRBuilder<>& builder,
               llvm_value_map& value_map)
    {
      throw std::exception { };
    }

    constexpr
    auto
    operator() (void) const noexcept
    {
      return &translate;
    }
  };

  template <>
  llvm::Instruction&
  instruction_translator<ir_opcode::phi>::
  translate (const ir_static_instruction& instr, llvm::IRBuilder<>& builder,
             llvm_value_map& value_map)
  {
    const ir_static_def& def           = instr.get_def ();
    unsigned             num_incoming  = static_cast<unsigned> (instr.num_args ()) / 2;

    llvm::Type& llvm_def_ty   = value_map[def.get_type ()];
    const char *llvm_def_name = def.get_variable_name ().data ();

    llvm::PHINode& llvm_phi = *builder.CreatePHI (&llvm_def_ty, num_incoming, llvm_def_name);
    return llvm_phi;
  }

  static constexpr
  auto
  translator_map { ir_metadata::template_generate_map<instruction_translator> () };

  static
  llvm::BasicBlock&
  translate_block (const ir_static_block& block, llvm_value_map& value_map)
  {
    llvm::BasicBlock& llvm_block = value_map[block];
    llvm::IRBuilder<> block_builder (&llvm_block);

    std::for_each (block.begin (), block.end (),
                   [&](const ir_static_instruction& instr)
                   {
                     translator_map[instr.get_metadata ()] (instr, block_builder, value_map);
                   });

    return llvm_block;
  }

  static
  llvm::Function&
  translate_unit (const ir_static_unit& unit, llvm::orc::ThreadSafeModule& llvm_module)
  {
    llvm::FunctionType *llvm_function_ty = nullptr;
    llvm::Function& out_func = *llvm_module.withModuleDo (
      [&] (llvm::Module& module)
      {
        return llvm::Function::Create (llvm_function_ty,
                                       llvm::Function::ExternalLinkage,
                                       create_twine (unit.get_name ()),
                                       module);
      });

    llvm_value_map value_map { llvm_module, unit, out_func };
    std::for_each (unit.begin (), unit.end (),
                   [&](const ir_static_block& block) { translate_block (block, value_map); });

    // resolve the phi nodes
    std::for_each (unit.begin (), unit.end (),
      [&](const ir_static_block& block)
      {
        auto instr_it  = block.begin ();
        auto phi_range = value_map[block].phis ();
        std::for_each (phi_range.begin (), phi_range.end (),
          [&](llvm::PHINode& phi_node)
          {
            assert (is_a<ir_opcode::phi> (*instr_it));
            assert (instr_it->num_args () % 2 == 0);

            for (auto op_it = instr_it->begin (); op_it != instr_it->end ();)
            {
              const ir_static_operand& value_op = *op_it++;
              const ir_static_operand& block_op = *op_it++;

              assert (is_use (value_op));
              const ir_static_use& value = as_use (value_op);

              assert (is_constant (block_op));
              const ir_constant& block_id_c = as_constant (block_op);

              assert (is_a<ir_static_block_id::value_type> (block_id_c));
              ir_static_block_id block_id { as<ir_static_block_id::value_type> (block_id_c) };

              phi_node.addIncoming (&value_map[value], &value_map[block_id]);
            }
          });
      });
    return out_func;
  }

  std::filesystem::path
  compile (const ir_static_unit& unit)
  {

    std::vector<int> x;
    llvm::SmallVector<std::string, 2> v;
    v.emplace_back ("hi");
    return v[0];
  }

}

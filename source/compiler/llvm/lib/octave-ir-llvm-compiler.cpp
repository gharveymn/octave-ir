/** octave-ir-llvm-compiler.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gch/octave-ir-compiler.hpp"
#include "gch/octave-ir-static-ir/ir-static-block.hpp"
#include "gch/octave-ir-static-ir/ir-static-instruction.hpp"
#include "gch/octave-ir-static-ir/ir-static-variable.hpp"
#include "gch/octave-ir-static-ir/ir-type-base.hpp"
#include "gch/octave-ir-static-ir/ir-type-std.hpp"

#include <gch/nonnull_ptr.hpp>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/BasicBlock.h>

#include <unordered_map>
#include <vector>

namespace gch
{

  static
  llvm::Twine
  create_twine (std::string_view view)
  {
    return { view.data () };
  }

  using llvm_type_function_type = llvm::Type * (*) (llvm::LLVMContext &);

  template <typename T, typename Enable = void>
  struct llvm_type_function
  { };

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
  static constexpr auto llvm_type_function_v = llvm_type_function<T>::value;

  class llvm_value_map
  {
  public:
    using type_map_t     = std::unordered_map<ir_type, nonnull_ptr<llvm::Type>>;
    using block_vector_t = std::vector<nonnull_ptr<llvm::BasicBlock>>;

    using var_map_t = std::unordered_map<nonnull_cptr<ir_static_variable>,
                                         nonnull_ptr<llvm::AllocaInst>>;

    using def_map_t = std::unordered_map<ir_static_def, nonnull_ptr<llvm::Value>>;

    llvm_value_map            (void)                      = delete;
    llvm_value_map            (const llvm_value_map&)     = delete;
    llvm_value_map            (llvm_value_map&&) noexcept = delete;
    llvm_value_map& operator= (const llvm_value_map&)     = delete;
    llvm_value_map& operator= (llvm_value_map&&) noexcept = delete;
    ~llvm_value_map           (void)                      = default;

    llvm_value_map (const ir_static_module& module, llvm::Function& function)
      : m_module   (module),
        m_context  (function.getContext ()),
        m_function (function)
    {
      initialize_type_map ();

      llvm::IRBuilder<> builder (m_context);

      // init entry block
      llvm::BasicBlock& first_block = *llvm::BasicBlock::Create (m_context, "entry", &m_function);
      m_block_map.try_emplace (nonnull_ptr { module.front () }, first_block);
      builder.SetInsertPoint (&first_block);

      // init variables
      std::transform (module.variables_begin (), module.variables_end (),
                      std::back_inserter (m_var_map),
                      [&](const ir_static_variable& var)
                      {
                        llvm::Type& ty   = (*this)[var.get_type ()];
                        const char *name = var.get_name ().data ();

                        return var_map_t::value_type {
                          var,
                          *builder.CreateAlloca (&ty, nullptr, name)
                        };
                       });

      // init blocks
      std::transform (std::next (module.begin ()), module.end (), std::back_inserter (m_blocks),
                      [&](const ir_static_block& block)
                      {
                        return nonnull_ptr {
                          *llvm::BasicBlock::Create (m_context, m_module.get_block_name (block),
                                                     &m_function)
                        };
                      });
    }

    llvm::Type&
    operator[] (ir_type ty) const
    {
      auto found = m_type_map.find (ty);
      assert (found != m_type_map.end ());
      return *found->second;
    }

    llvm::BasicBlock&
    operator[] (const ir_static_block& block) const
    {
      using size_ty = block_vector_t::size_type;

      auto off = std::distance (&m_module.front (), &block);

      assert (0 <= off);
      assert (static_cast<std::size_t> (off) < (std::numeric_limits<size_ty>::max) ());

      return *m_blocks[static_cast<size_ty> (off)];
    }

    llvm::AllocaInst&
    operator[] (const ir_static_variable& var) const
    {
      auto found = m_var_map.find (nonnull_ptr { var });
      assert (found != m_var_map.end ());
      return *found->second;
    }

    optional_ref<llvm::Value>
    operator[] (ir_static_def def) const
    {
      if (auto found = m_def_map.find (def) ; found != m_def_map.end ())
        return optional_ref { *found->second };
      return nullopt;
    }

    template <typename T>
    llvm::Type *
    get_llvm_type (void)
    {
      return llvm_type_function_v<T> (m_context);
    }

    llvm::Value&
    register_def (ir_static_def def, llvm::Value& llvm_value)
    {
      m_def_map.try_emplace (def, llvm_value);
      return llvm_value;
    }

  private:
    void
    initialize_type_map (void)
    {
      add_type_mapping<void> ();

      add_type_mapping<double> ();
      add_type_mapping<single> ();

      add_type_mapping<int64_t> ();
      add_type_mapping<int32_t> ();
      add_type_mapping<int16_t> ();
      add_type_mapping<int8_t> ();

      add_type_mapping<uint64_t> ();
      add_type_mapping<uint32_t> ();
      add_type_mapping<uint16_t> ();
      add_type_mapping<uint8_t> ();

      add_type_mapping<char> ();
      add_type_mapping<bool> ();
    }

    template <typename T>
    void
    add_type_mapping (void)
    {
      m_type_map.try_emplace (ir_type_v<T>, get_llvm_type<T> ());
    }

    const ir_static_module& m_module;
    llvm::LLVMContext&      m_context;
    llvm::Function&         m_function;
    type_map_t              m_type_map;
    block_vector_t          m_blocks;
    var_map_t               m_var_map;
    def_map_t               m_def_map;
  };

  template <ir_opcode Op>
  struct instruction_translator
  {
    static constexpr ir_opcode opcode = Op;

    static
    llvm::Instruction&
    translate (const ir_static_instruction& instr, llvm_module& llvm_module);

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
  translate (llvm_module& module, const ir_static_instruction& instr)
  {
    const ir_static_def& def           = instr.get_def ();
    unsigned             num_incoming  = static_cast<unsigned> (instr.num_args ()) / 2;

    llvm::Type *llvm_def_ty   = module.type_map[def.get_type ()];
    const char *llvm_def_name = def.get_variable_name ().data ();

    llvm::PHINode& llvm_phi = *module.builder.CreatePHI (llvm_def_ty, num_incoming, llvm_def_name);
    llvm_phi.addIncoming ()

  }

  static constexpr
  auto
  translation_map { ir_metadata::template_generate_map<instruction_translator> () };

  static
  llvm::BasicBlock&
  compile_block (const ir_static_block& block, llvm::Twine&& name, llvm::Function& llvm_function,
                 llvm::LLVMContext& llvm_context)
  {
    auto& llvm_block = *llvm::BasicBlock::Create (llvm_context, std::move (name), &llvm_function);
    llvm::IRBuilder<> block_builder (&llvm_block);

    std::for_each (block.begin (), block.end (),
                   [](const ir_static_instruction& instr)
                   {

                   });

    return llvm_block;
  }

  std::filesystem::path
  compile (const ir_static_module& module)
  {

    std::vector<int> x;
    llvm::SmallVector<std::string, 2> v;
    v.emplace_back ("hi");
    return v[0];
  }

}

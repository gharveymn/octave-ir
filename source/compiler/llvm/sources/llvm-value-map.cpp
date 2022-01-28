/** llvm-value-map.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "llvm-common.hpp"
#include "llvm-constant.hpp"
#include "llvm-value-map.hpp"

#include "ir-static-block.hpp"
#include "ir-static-def.hpp"
#include "ir-static-function.hpp"
#include "ir-static-operand.hpp"
#include "ir-static-use.hpp"
#include "ir-static-variable.hpp"
#include "ir-type-util.hpp"
#include "ir-type.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/optional_ref.hpp>

GCH_DISABLE_WARNINGS_MSVC

#include <llvm/ADT/Twine.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Module.h>

GCH_ENABLE_WARNINGS_MSVC

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace llvm
{
  class AllocaInst;
  class Function;
  class LLVMContext;
  class Type;
  class Value;
}

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

  llvm::ConstantInt&
  llvm_module_interface::
  get_bool_constant (bool b)
  {
    return b ? get_bool_constant<true> () : get_bool_constant<false> ();
  }

  optional_ref<llvm::Function>
  llvm_module_interface::
  get_external_function (std::string_view name, llvm::FunctionType& prototype)
  {
    struct key_pair
    {
      std::string                     name;
      nonnull_ptr<llvm::FunctionType> prototype;
    };

    struct key_hash
    {
      std::size_t
      operator() (const key_pair& pair) const
      {
        std::size_t name_hash = std::hash<std::string> {} (pair.name);
        std::size_t proto_hash = std::hash<nonnull_ptr<llvm::FunctionType>> { } (pair.prototype);
        return name_hash ^ (proto_hash + 0x9E3779B9 + (name_hash << 6) + (name_hash >> 2));
      }
    };

    struct key_equal
    {
      bool
      operator() (const key_pair& lhs, const key_pair& rhs) const
      {
        return lhs.name == rhs.name && lhs.prototype == rhs.prototype;
      }
    };

    using map_type = std::unordered_map<key_pair,
                                        optional_ref<llvm::Function>,
                                        key_hash,
                                        key_equal>;

    // The function name/prototype map is a singleton since LLVM loads each external function
    // once per thread.
    static std::unique_ptr<map_type> map;
    if (! map)
      map = std::make_unique<map_type> ();

    auto [it, ins] = map->try_emplace (key_pair { std::string (name), nonnull_ptr { prototype } });

    optional_ref<llvm::Function>& ret = it->second;

    if (ins)
    {
      ret.emplace (invoke_with_module ([&](llvm::Module& module) {
        return llvm::Function::Create (
          &prototype,
          llvm::Function::ExternalLinkage,
          create_twine (name),
          module);
      }));
    }

    return ret;
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

  llvm::AllocaInst&
  llvm_value_map::
  operator[] (ir_static_def def) const
  {
    return operator[] (m_function.get_variable (def));
  }

  llvm::Value&
  llvm_value_map::
  operator[] (ir_static_use use)
  {
    return std::as_const (*this).operator[] (use);
  }

  llvm::Value&
  llvm_value_map::
  operator[] (ir_static_use use) const
  {
    auto found = m_var_map.find (nonnull_ptr { m_function.get_variable (use) });
    assert (found != m_var_map.end ());

    if (std::optional def_id { use.maybe_get_def_id () })
      return found->second[*def_id];
    return *llvm::UndefValue::get (&get_llvm_type (found->first->get_type ()));
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
    auto found = m_var_map.find (nonnull_ptr { m_function.get_variable (def) });
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

  llvm::Type&
  llvm_value_map::
  get_llvm_type (ir_static_def def) const
  {
    return get_llvm_type (get_type (def));
  }

  llvm::Twine
  llvm_value_map::
  get_variable_name (ir_static_def def) const
  {
    return create_twine (m_function.get_variable_name (def));
  }

  ir_type
  llvm_value_map::
  get_type (ir_static_def def) const
  {
    return m_function.get_type (def);
  }

  ir_type
  llvm_value_map::
  get_type (const ir_static_operand& op) const
  {
    return visit (overloaded {
      std::mem_fn (&ir_constant::get_type),
      [&](ir_static_use use) { return m_function.get_type (use); }
    }, op);
  }

  llvm::Argument&
  llvm_value_map::
  get_llvm_argument (ir_static_variable_id var_id)
  {
    // FIXME: We shouldn't need to do a search here.
    auto found = std::find (m_function.args_begin (), m_function.args_end (), var_id);
    assert (found != m_function.args_end ()
        &&  "Could not find the variable in the function arguments");

    unsigned arg_idx = static_cast<unsigned> (std::distance (m_function.args_begin (), found));
    llvm::Argument *llvm_arg = m_llvm_function.getArg (arg_idx);
    return *llvm_arg;
  }

  llvm::Constant&
  llvm_value_map::
  get_zero (ir_type type)
  {
    if (type.is_integral ())
      return *llvm::ConstantInt::get (&get_llvm_type (type), 0);
    else
      return *llvm::ConstantFP::get (&get_llvm_type (type), 0.0);
  }

}

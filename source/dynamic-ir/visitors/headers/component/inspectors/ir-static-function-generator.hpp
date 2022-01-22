/** ir-static-generator.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_STATIC_FUNCTION_GENERATOR_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_STATIC_FUNCTION_GENERATOR_HPP

#include "ir-abstract-component-inspector.hpp"
#include "ir-index-sequence-map.hpp"
#include "ir-instruction.hpp"
#include "ir-static-block.hpp"
#include "ir-static-def.hpp"
#include "ir-static-function.hpp"
#include "ir-static-instruction.hpp"
#include "ir-static-variable.hpp"
#include "ir-visitor.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/small_vector.hpp>

#include <unordered_map>
#include <vector>

namespace gch
{

  class determinator_manager;
  class ir_use_timeline;
  class ir_static_instruction;
  class ir_static_use;
  class ir_static_variable_map;
  class ir_variable;

  class ir_def_reference
  {
  public:
    ir_def_reference            (void)                        = delete;
    ir_def_reference            (const ir_def_reference&)     = default;
    ir_def_reference            (ir_def_reference&&) noexcept = default;
    ir_def_reference& operator= (const ir_def_reference&)     = default;
    ir_def_reference& operator= (ir_def_reference&&) noexcept = default;
    ~ir_def_reference           (void)                        = default;

    ir_def_reference (const ir_def& d, bool is_indeterminate);

    [[nodiscard]]
    const ir_def&
    operator* (void) const noexcept;

    [[nodiscard]]
    bool
    is_indeterminate (void) const noexcept;

    [[nodiscard]]
    bool
    is_determinate (void) const noexcept;

    void
    set_def (const ir_def& d) noexcept;

    void
    set_indeterminate_state (bool b) noexcept;

  private:
    nonnull_cptr<ir_def> m_def;
    bool                 m_is_indeterminate;
  };

  class ir_static_variable_map
  {
    using static_variable_map = std::unordered_map<nonnull_cptr<ir_variable>,
                                                   nonnull_cptr<ir_static_variable>>;

  public:
    ir_static_variable_map            (void)                              = delete;
    ir_static_variable_map            (const ir_static_variable_map&)     = default;
    ir_static_variable_map            (ir_static_variable_map&&) noexcept = default;
    ir_static_variable_map& operator= (const ir_static_variable_map&)     = delete;
    ir_static_variable_map& operator= (ir_static_variable_map&&) noexcept = delete;
    ~ir_static_variable_map           (void)                              = default;

    explicit
    ir_static_variable_map (const ir_function& func);

    const ir_static_variable&
    operator[] (const ir_variable& var) const;

    ir_static_def
    create_static_def (const ir_def& def) const;

    ir_static_use
    create_static_use (const ir_use& use) const;

    ir_static_def_id
    get_def_id (const ir_def& def) const;

    ir_static_def_id
    get_def_id (const ir_def_reference& dr) const;

    ir_static_def_id
    origin_id (const ir_use_timeline& ut) const;

    ir_static_def_id
    origin_id (const ir_use& use) const;

    std::optional<ir_def_reference>&
    map_origin (const ir_use_timeline& ut, const std::optional<ir_def_reference>& origin);

    std::optional<ir_def_reference>&
    map_origin (const ir_use_timeline& ut);

    optional_ref<std::optional<ir_def_reference>>
    maybe_get (const ir_use_timeline& ut);

    optional_cref<std::optional<ir_def_reference>>
    maybe_get (const ir_use_timeline& ut) const;

  private:
    ir_static_def_id
    register_def (const ir_def& d);

    std::unordered_map<nonnull_cptr<ir_use_timeline>,
                       std::optional<ir_def_reference>>               m_origin_map;
    std::unordered_map<nonnull_cptr<ir_variable>, ir_static_variable> m_variable_map;
    std::unordered_map<nonnull_cptr<ir_def>,      ir_static_def_id>   m_id_map;
  };

  class ir_static_incoming_pair
  {
  public:
    ir_static_incoming_pair            (void)                               = delete;
    ir_static_incoming_pair            (const ir_static_incoming_pair&)     = default;
    ir_static_incoming_pair            (ir_static_incoming_pair&&) noexcept = default;
    ir_static_incoming_pair& operator= (const ir_static_incoming_pair&)     = default;
    ir_static_incoming_pair& operator= (ir_static_incoming_pair&&) noexcept = default;
    ~ir_static_incoming_pair           (void)                               = default;

    ir_static_incoming_pair (ir_static_block_id block_id, ir_static_def_id def_id) noexcept;

    [[nodiscard]]
    ir_static_block_id
    get_block_id (void) const noexcept;

    [[nodiscard]]
    ir_static_def_id
    get_def_id (void) const noexcept;

  private:
    ir_static_block_id m_block_id;
    ir_static_def_id   m_def_id;
  };

  class ir_resolved_phi_node
  {
  public:
    using incoming_container_type          = small_vector<ir_static_incoming_pair>;
    using incoming_value_type              = incoming_container_type::value_type;
    using incoming_allocator_type          = incoming_container_type::allocator_type;
    using incoming_size_type               = incoming_container_type::size_type;
    using incoming_difference_type         = incoming_container_type::difference_type;
    using incoming_reference               = incoming_container_type::reference;
    using incoming_const_reference         = incoming_container_type::const_reference;
    using incoming_pointer                 = incoming_container_type::pointer;
    using incoming_const_pointer           = incoming_container_type::const_pointer;

    using incoming_iterator                = incoming_container_type::iterator;
    using incoming_const_iterator          = incoming_container_type::const_iterator;
    using incoming_reverse_iterator        = incoming_container_type::reverse_iterator;
    using incoming_const_reverse_iterator  = incoming_container_type::const_reverse_iterator;

    using incoming_val_t   = incoming_value_type;
    using incoming_alloc_t = incoming_allocator_type;
    using incoming_size_ty = incoming_size_type;
    using incoming_diff_ty = incoming_difference_type;
    using incoming_ref     = incoming_reference;
    using incoming_cref    = incoming_const_reference;
    using incoming_ptr     = incoming_pointer;
    using incoming_cptr    = incoming_const_pointer;

    using incoming_iter    = incoming_iterator;
    using incoming_citer   = incoming_const_iterator;
    using incoming_riter   = incoming_reverse_iterator;
    using incoming_criter  = incoming_const_reverse_iterator;

    ir_resolved_phi_node            (void)                            = delete;
    ir_resolved_phi_node            (const ir_resolved_phi_node&)     = default;
    ir_resolved_phi_node            (ir_resolved_phi_node&&) noexcept = default;
    ir_resolved_phi_node& operator= (const ir_resolved_phi_node&)     = default;
    ir_resolved_phi_node& operator= (ir_resolved_phi_node&&) noexcept = default;
    ~ir_resolved_phi_node           (void)                            = default;

    ir_resolved_phi_node (ir_static_def_id id, small_vector<ir_static_incoming_pair>&& incoming);

    [[nodiscard]]
    ir_static_def_id
    get_id (void) const noexcept;

    [[nodiscard]]
    incoming_const_iterator
    begin (void) const noexcept;

    [[nodiscard]]
    incoming_const_iterator
    end (void) const noexcept;

  private:
    ir_static_def_id        m_id;
    incoming_container_type m_incoming;
  };

  class ir_injection
  {
  public:
    class assign
    {
    public:
      assign (const ir_variable& assign_var, ir_static_def_id assign_def_id, bool assign_value);

      [[nodiscard]]
      const ir_variable&
      get_assign_variable (void) const noexcept;

      [[nodiscard]]
      ir_static_def_id
      get_assign_def_id (void) const noexcept;

      [[nodiscard]]
      bool
      get_assign_value (void) const noexcept;

    private:
      nonnull_cptr<ir_variable> m_assign_var;
      ir_static_def_id          m_assign_def_id;
      bool                      m_assign_value;
    };

    class terminator
    {
    public:
      terminator (const ir_variable& uninit_var);

      [[nodiscard]]
      const ir_variable&
      get_uninit_variable (void) const noexcept;

    private:
      nonnull_cptr<ir_variable> m_uninit_variable;
    };

    class branch : public terminator
    {
    public:
      branch (const ir_variable& uninit_var,
              const ir_variable& determinator_var,
              ir_static_def_id   determinator_def_id,
              ir_static_block_id continue_block_id,
              ir_static_block_id terminal_block_id);

      [[nodiscard]]
      const ir_variable&
      get_determinator_variable (void) const noexcept;

      [[nodiscard]]
      ir_static_def_id
      get_determinator_def_id (void) const noexcept;

      [[nodiscard]]
      ir_static_block_id
      get_continue_block_id (void) const noexcept;

      [[nodiscard]]
      ir_static_block_id
      get_terminal_block_id (void) const noexcept;

    private:
      nonnull_cptr<ir_variable> m_determinator_var;
      ir_static_def_id          m_determinator_def_id;
      ir_static_block_id        m_continue_block_id;
      ir_static_block_id        m_terminal_block_id;
    };

    ir_injection            (void)                    = delete;
    ir_injection            (const ir_injection&)     = delete;
    ir_injection            (ir_injection&&) noexcept = default;
    ir_injection& operator= (const ir_injection&)     = delete;
    ir_injection& operator= (ir_injection&&) noexcept = default;
    ~ir_injection           (void)                    = default;

    template <typename T, typename ...Args>
    ir_injection (std::in_place_type_t<T>, ir_instruction_citer pos, Args&&... args)
      : m_pos (pos),
        m_data (std::in_place_type<T>, std::forward<Args> (args)...)
    { }

    template <typename Visitor>
    friend constexpr
    auto
    visit (Visitor&& visitor, const ir_injection& det)
    {
      return std::visit (std::forward<Visitor> (visitor), det.m_data);
    }

    template <typename Alt, typename Visitor>
    friend constexpr
    auto
    visit_alternative (Visitor&& visitor, const ir_injection& det)
    {
      return std::visit ([&](auto&& val) {
        if constexpr (std::is_same_v<Alt, remove_cvref_t<decltype (val)>>)
          return std::invoke (std::forward<Visitor> (visitor), *std::get_if<Alt> (&det.m_data));
        else
          return std::invoke_result_t<Visitor, const Alt&> ();
      }, det.m_data);
    }

    template <typename T>
    friend constexpr
    bool
    is_a (const ir_injection& inj) noexcept
    {
      return std::holds_alternative<T> (inj.m_data);
    }

    template <typename T>
    friend constexpr
    optional_cref<T>
    maybe_as (const ir_injection& inj) noexcept
    {
      return std::get_if<T> (&inj.m_data);
    }

    [[nodiscard]]
    ir_instruction_citer
    get_injection_pos (void) const noexcept;

  private:
    ir_instruction_citer                     m_pos;
    std::variant<assign, terminator, branch> m_data;
  };

  constexpr
  optional_cref<ir_injection::assign>
  maybe_as_assign (const ir_injection& inj) noexcept
  {
    return maybe_as<ir_injection::assign> (inj);
  }

  constexpr
  const ir_injection::assign&
  as_assign (const ir_injection& inj) noexcept
  {
    return *maybe_as_assign (inj);
  }

  constexpr
  optional_cref<ir_injection::terminator>
  maybe_as_terminator (const ir_injection& inj) noexcept
  {
    return maybe_as<ir_injection::terminator> (inj);
  }

  constexpr
  const ir_injection::terminator&
  as_terminator (const ir_injection& inj) noexcept
  {
    return *maybe_as_terminator (inj);
  }

  constexpr
  optional_cref<ir_injection::branch>
  maybe_as_branch (const ir_injection& inj) noexcept
  {
    return maybe_as<ir_injection::branch> (inj);
  }

  constexpr
  const ir_injection::branch&
  as_branch (const ir_injection& inj) noexcept
  {
    return *maybe_as_branch (inj);
  }

  class ir_block_descriptor
  {
  public:
    using phi_map_type            = std::unordered_map<nonnull_cptr<ir_variable>,
                                                       ir_resolved_phi_node>;
    using phi_map_value_type      = phi_map_type::value_type;
    using phi_map_allocator_type  = phi_map_type::allocator_type;
    using phi_map_size_type       = phi_map_type::size_type;
    using phi_map_difference_type = phi_map_type::difference_type;
    using phi_map_reference       = phi_map_type::reference;
    using phi_map_const_reference = phi_map_type::const_reference;
    using phi_map_pointer         = phi_map_type::pointer;
    using phi_map_const_pointer   = phi_map_type::const_pointer;

    using phi_map_iterator        = phi_map_type::iterator;
    using phi_map_const_iterator  = phi_map_type::const_iterator;

    using phi_map_val_t   = phi_map_value_type;
    using phi_map_alloc_t = phi_map_allocator_type;
    using phi_map_size_ty = phi_map_size_type;
    using phi_map_diff_ty = phi_map_difference_type;
    using phi_map_ref     = phi_map_reference;
    using phi_map_cref    = phi_map_const_reference;
    using phi_map_ptr     = phi_map_pointer;
    using phi_map_cptr    = phi_map_const_pointer;

    using phi_map_iter    = phi_map_iterator;
    using phi_map_citer   = phi_map_const_iterator;

    using injections_container_type          = std::vector<ir_injection>;
    using injections_value_type              = injections_container_type::value_type;
    using injections_allocator_type          = injections_container_type::allocator_type;
    using injections_size_type               = injections_container_type::size_type;
    using injections_difference_type         = injections_container_type::difference_type;
    using injections_reference               = injections_container_type::reference;
    using injections_const_reference         = injections_container_type::const_reference;
    using injections_pointer                 = injections_container_type::pointer;
    using injections_const_pointer           = injections_container_type::const_pointer;

    using injections_iterator                = injections_container_type::iterator;
    using injections_const_iterator          = injections_container_type::const_iterator;
    using injections_reverse_iterator        = injections_container_type::reverse_iterator;
    using injections_const_reverse_iterator  = injections_container_type::const_reverse_iterator;

    using injections_val_t   = injections_value_type;
    using injections_alloc_t = injections_allocator_type;
    using injections_size_ty = injections_size_type;
    using injections_diff_ty = injections_difference_type;
    using injections_ref     = injections_reference;
    using injections_cref    = injections_const_reference;
    using injections_ptr     = injections_pointer;
    using injections_cptr    = injections_const_pointer;

    using injections_iter    = injections_iterator;
    using injections_citer   = injections_const_iterator;
    using injections_riter   = injections_reverse_iterator;
    using injections_criter  = injections_const_reverse_iterator;

    using successors_container_type          = small_vector<ir_static_block_id, 2>;
    using successors_value_type              = successors_container_type::value_type;
    using successors_allocator_type          = successors_container_type::allocator_type;
    using successors_size_type               = successors_container_type::size_type;
    using successors_difference_type         = successors_container_type::difference_type;
    using successors_reference               = successors_container_type::reference;
    using successors_const_reference         = successors_container_type::const_reference;
    using successors_pointer                 = successors_container_type::pointer;
    using successors_const_pointer           = successors_container_type::const_pointer;

    using successors_iterator                = successors_container_type::iterator;
    using successors_const_iterator          = successors_container_type::const_iterator;
    using successors_reverse_iterator        = successors_container_type::reverse_iterator;
    using successors_const_reverse_iterator  = successors_container_type::const_reverse_iterator;

    using successors_val_t   = successors_value_type;
    using successors_alloc_t = successors_allocator_type;
    using successors_size_ty = successors_size_type;
    using successors_diff_ty = successors_difference_type;
    using successors_ref     = successors_reference;
    using successors_cref    = successors_const_reference;
    using successors_ptr     = successors_pointer;
    using successors_cptr    = successors_const_pointer;

    using successors_iter    = successors_iterator;
    using successors_citer   = successors_const_iterator;
    using successors_riter   = successors_reverse_iterator;
    using successors_criter  = successors_const_reverse_iterator;

    ir_block_descriptor            (void)                           = delete;
    ir_block_descriptor            (const ir_block_descriptor&)     = default;
    ir_block_descriptor            (ir_block_descriptor&&) noexcept = default;
    ir_block_descriptor& operator= (const ir_block_descriptor&)     = default;
    ir_block_descriptor& operator= (ir_block_descriptor&&) noexcept = default;
    ~ir_block_descriptor           (void)                           = default;

    explicit
    ir_block_descriptor (ir_static_block_id id) noexcept;

    ir_block_descriptor (ir_static_block_id id,
                         small_vector<ir_static_block_id>&& successor_ids) noexcept;

    [[nodiscard]]
    ir_static_block_id
    get_id (void) const noexcept;

    [[nodiscard]]
    phi_map_const_iterator
    phi_map_begin (void) const noexcept;

    [[nodiscard]]
    phi_map_const_iterator
    phi_map_end (void) const noexcept;

    [[nodiscard]]
    phi_map_size_type
    num_phi_nodes (void) const noexcept;

    [[nodiscard]]
    bool
    has_phi_nodes (void) const noexcept;

    [[nodiscard]]
    injections_const_iterator
    injections_begin (void) const noexcept;

    [[nodiscard]]
    injections_const_iterator
    injections_end (void) const noexcept;

    [[nodiscard]]
    injections_const_reverse_iterator
    injections_rbegin (void) const noexcept;

    [[nodiscard]]
    injections_const_reverse_iterator
    injections_rend (void) const noexcept;

    [[nodiscard]]
    injections_size_type
    num_injections (void) const noexcept;

    [[nodiscard]]
    bool
    has_injections (void) const noexcept;

    [[nodiscard]]
    successors_const_iterator
    successors_begin (void) const noexcept;

    [[nodiscard]]
    successors_const_iterator
    successors_end (void) const noexcept;

    [[nodiscard]]
    successors_const_reference
    successors_front (void) const;

    [[nodiscard]]
    successors_const_reference
    successors_back (void) const;

    [[nodiscard]]
    successors_size_type
    num_successors (void) const noexcept;

    [[nodiscard]]
    bool
    has_successors (void) const noexcept;

    void
    add_successor (ir_static_block_id id);

    void
    add_phi_node (const ir_variable& var, ir_static_def_id id,
                  small_vector<ir_static_incoming_pair>&& incoming);

    injections_iterator
    emplace_assign_injection (injections_const_iterator pos,
                                 ir_instruction_citer      instr_pos,
                                 const ir_variable&        det_var,
                                 ir_static_def_id          def_id,
                                 bool                      assign_value);

    injections_iterator
    emplace_terminator_injection (injections_const_iterator pos,
                                     ir_instruction_citer      instr_pos,
                                     const ir_variable&        uninit_var);

    injections_iterator
    emplace_branch_injection (injections_const_iterator pos,
                              ir_instruction_citer      instr_pos,
                              const ir_variable&        det_var,
                              const ir_variable&        uninit_var,
                              ir_static_def_id          def_id,
                              ir_static_block_id        continue_block_id,
                              ir_static_block_id        terminal_block_id);

    ir_injection&
    prepend_assign_injection (const ir_block&    block,
                              const ir_variable& det_var,
                              ir_static_def_id   def_id,
                              bool               assign_value);

    optional_cref<ir_resolved_phi_node>
    maybe_get_phi (const ir_variable& var) const;

    template <typename ...Args>
    const ir_static_instruction&
    emplace_terminal_instruction (Args&&... args)
    {
      return m_terminal_instr.emplace (std::forward<Args> (args)...);
    }

    const ir_static_instruction&
    emplace_terminal_instruction (ir_opcode op,
                                  ir_static_def def,
                                  std::initializer_list<ir_static_operand> init);

    const ir_static_instruction&
    emplace_terminal_instruction (ir_opcode op, std::initializer_list<ir_static_operand> init);

    bool
    has_terminal_instruction (void) const noexcept;

    const ir_static_instruction&
    get_terminal_instruction (void) const noexcept;

  private:
    ir_static_block_id                   m_id;
    phi_map_type                         m_phi_nodes;
    injections_container_type            m_injections;
    successors_container_type            m_successors;
    std::optional<ir_static_instruction> m_terminal_instr;
  };

  class ir_dynamic_block_manager
  {
  public:
    using map_type                = std::unordered_map<nonnull_cptr<ir_block>, ir_block_descriptor>;
    using key_type                = map_type::key_type;
    using mapped_type             = map_type::mapped_type;
    using value_type              = map_type::value_type;
    using allocator_type          = map_type::allocator_type;
    using size_type               = map_type::size_type;
    using difference_type         = map_type::difference_type;
    using reference               = map_type::reference;
    using const_reference         = map_type::const_reference;
    using pointer                 = map_type::pointer;
    using const_pointer           = map_type::const_pointer;

    using iterator                = map_type::iterator;
    using const_iterator          = map_type::const_iterator;

    using val_t   = value_type;
    using alloc_t = allocator_type;
    using size_ty = size_type;
    using diff_ty = difference_type;
    using ref     = reference;
    using cref    = const_reference;
    using ptr     = pointer;
    using cptr    = const_pointer;

    using iter    = iterator;
    using citer   = const_iterator;

    ir_dynamic_block_manager            (void)                                = delete;
    ir_dynamic_block_manager            (const ir_dynamic_block_manager&)     = delete;
    ir_dynamic_block_manager            (ir_dynamic_block_manager&&) noexcept = default;
    ir_dynamic_block_manager& operator= (const ir_dynamic_block_manager&)     = delete;
    ir_dynamic_block_manager& operator= (ir_dynamic_block_manager&&) noexcept = delete;
    ~ir_dynamic_block_manager           (void)                                = default;

    explicit
    ir_dynamic_block_manager (const ir_function& func);

    ir_block_descriptor&
    register_block (const ir_block& block);

    ir_block_descriptor&
    register_block (const ir_block& block, const small_vector<nonnull_cptr<ir_block>>& successors);

    [[nodiscard]]
    ir_block_descriptor&
    operator[] (const ir_block& block);

    [[nodiscard]]
    const ir_block_descriptor&
    operator[] (const ir_block& block) const;

    [[nodiscard]]
    ir_static_block_id
    create_injected_block (void) noexcept;

    [[nodiscard]]
    std::size_t
    num_mapped_blocks (void) const noexcept;

    [[nodiscard]]
    std::size_t
    num_injected_blocks (void) const noexcept;

    [[nodiscard]]
    std::size_t
    total_num_blocks (void) const noexcept;

    [[nodiscard]]
    bool
    contains (const ir_block& block) const;

    [[nodiscard]]
    ir_variable&
    create_temp_variable (const ir_component& c, std::string_view name, ir_type ty);

    [[nodiscard]]
    iterator
    begin (void) noexcept;

    [[nodiscard]]
    const_iterator
    begin (void) const noexcept;

    [[nodiscard]]
    const_iterator
    cbegin (void) const noexcept;

    [[nodiscard]]
    iterator
    end (void) noexcept;

    [[nodiscard]]
    const_iterator
    end (void) const noexcept;

    [[nodiscard]]
    const_iterator
    cend (void) const noexcept;

    const ir_function&
    get_function (void) const noexcept;

  private:
    ir_block_descriptor&
    find_or_emplace (const ir_block& block);

    nonnull_cptr<ir_function>                                       m_function;
    std::unordered_map<nonnull_cptr<ir_block>, ir_block_descriptor> m_descriptor_map;
    std::list<ir_variable>                                          m_temp_variables;
    std::size_t                                                     m_num_injected_blocks = 0;
  };



  class ir_dynamic_block_manager_builder final
    : public ir_abstract_component_inspector
  {
  public:
    ir_dynamic_block_manager_builder (const ir_function& func);

    ir_dynamic_block_manager
    operator() (void);

    void
    visit (const ir_block& block) override;

    void
    visit (const ir_component_fork& fork) override;

    void
    visit (const ir_component_loop& loop) override;

    void
    visit (const ir_component_sequence& seq) override;

    void
    visit (const ir_function& func) override;

  private:
    void
    dispatch (const ir_component& c);

    ir_dynamic_block_manager m_block_manager;
  };

  ir_static_function
  generate_static_function (const ir_function& c);

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_STATIC_FUNCTION_GENERATOR_HPP

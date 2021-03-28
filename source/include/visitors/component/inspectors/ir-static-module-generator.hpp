/** ir-static-generator.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_STATIC_MODULE_GENERATOR_HPP
#define OCTAVE_IR_IR_STATIC_MODULE_GENERATOR_HPP

#include "components/static/ir-static-block.hpp"
#include "components/static/ir-static-module.hpp"
#include "utilities/ir-index-sequence-map.hpp"
#include "values/static/ir-static-def.hpp"
#include "values/static/ir-static-variable.hpp"
#include "values/ir-instruction.hpp"
#include "visitors/component/inspectors/ir-abstract-component-inspector.hpp"
#include "visitors/ir-visitor.hpp"

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

  // injective
  class ir_def_id_map
  {
  public:
    using count_map_type            = std::unordered_map<nonnull_cptr<ir_variable>, std::size_t>;
    using count_map_value_type      = count_map_type::value_type;
    using count_map_allocator_type  = count_map_type::allocator_type;
    using count_map_size_type       = count_map_type::size_type;
    using count_map_difference_type = count_map_type::difference_type;
    using count_map_reference       = count_map_type::reference;
    using count_map_const_reference = count_map_type::const_reference;
    using count_map_pointer         = count_map_type::pointer;
    using count_map_const_pointer   = count_map_type::const_pointer;

    using count_map_iterator        = count_map_type::iterator;
    using count_map_const_iterator  = count_map_type::const_iterator;

    using count_map_val_t   = count_map_value_type;
    using count_map_alloc_t = count_map_allocator_type;
    using count_map_size_ty = count_map_size_type;
    using count_map_diff_ty = count_map_difference_type;
    using count_map_ref     = count_map_reference;
    using count_map_cref    = count_map_const_reference;
    using count_map_ptr     = count_map_pointer;
    using count_map_cptr    = count_map_const_pointer;

    using count_map_iter    = count_map_iterator;
    using count_map_citer   = count_map_const_iterator;

    ir_static_def_id
    register_def (const ir_def& d);

    ir_static_def_id
    operator[] (const ir_def& d) const;

    std::size_t
    num_defs (const ir_variable& v) const;

    bool
    contains (const ir_def& d) const;

    std::size_t
    increment (const ir_variable& var);

    [[nodiscard]]
    count_map_size_type
    num_variables (void) const noexcept;

    [[nodiscard]]
    count_map_iterator
    count_map_begin (void) noexcept;

    [[nodiscard]]
    count_map_const_iterator
    count_map_begin (void) const noexcept;

    [[nodiscard]]
    count_map_const_iterator
    count_map_cbegin (void) const noexcept;

    [[nodiscard]]
    count_map_iterator
    count_map_end (void) noexcept;

    [[nodiscard]]
    count_map_const_iterator
    count_map_end (void) const noexcept;

    [[nodiscard]]
    count_map_const_iterator
    count_map_cend (void) const noexcept;

  private:
    std::unordered_map<nonnull_cptr<ir_def>,      ir_static_def_id> m_id_map;
    std::unordered_map<nonnull_cptr<ir_variable>, std::size_t>      m_def_count_map;
  };

  // surjective
  class ir_timeline_origin_map
  {
  public:
    using key_type    = nonnull_cptr<ir_use_timeline>;
    using mapped_type = std::optional<ir_def_reference>;

    using map_type        = std::unordered_map<key_type, mapped_type>;
    using value_type      = map_type::value_type;
    using allocator_type  = map_type::allocator_type;
    using size_type       = map_type::size_type;
    using difference_type = map_type::difference_type;
    using reference       = map_type::reference;
    using const_reference = map_type::const_reference;
    using pointer         = map_type::pointer;
    using const_pointer   = map_type::const_pointer;

    using iterator        = map_type::iterator;
    using const_iterator  = map_type::const_iterator;

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

    std::optional<ir_def_reference>&
    map_origin (const ir_use_timeline& ut, const std::optional<ir_def_reference>& origin);

    std::optional<ir_def_reference>&
    map_origin (const ir_use_timeline& ut, const ir_def& d, bool is_indet);

    ir_static_def_id
    def_id (const ir_def& d) const;

    ir_static_def_id
    origin_id (const ir_use_timeline& ut) const;

    ir_static_def_id
    origin_id (const ir_use& use) const;

    const ir_def_id_map&
    get_id_map (void) const noexcept;

    ir_def_id_map&&
    release_id_map (void) noexcept;

    optional_ref<std::optional<ir_def_reference>>
    maybe_get (const ir_use_timeline& ut);

    optional_cref<std::optional<ir_def_reference>>
    maybe_get (const ir_use_timeline& ut) const;

  private:
    map_type      m_origin_map;
    ir_def_id_map m_id_map;
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

    ir_resolved_phi_node            (void)                            = default;
    ir_resolved_phi_node            (const ir_resolved_phi_node&)     = default;
    ir_resolved_phi_node            (ir_resolved_phi_node&&) noexcept = default;
    ir_resolved_phi_node& operator= (const ir_resolved_phi_node&)     = default;
    ir_resolved_phi_node& operator= (ir_resolved_phi_node&&) noexcept = default;
    ~ir_resolved_phi_node           (void)                            = default;

    ir_resolved_phi_node (ir_static_def_id id, small_vector<ir_static_incoming_pair>&& incoming);

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

  class ir_determinator_injection
  {
  public:
    enum class injection_type
    {
      determinator,
      branch      ,
    };

    ir_determinator_injection            (void)                                 = delete;
    ir_determinator_injection            (const ir_determinator_injection&)     = delete;
    ir_determinator_injection            (ir_determinator_injection&&) noexcept = default;
    ir_determinator_injection& operator= (const ir_determinator_injection&)     = delete;
    ir_determinator_injection& operator= (ir_determinator_injection&&) noexcept = default;
    ~ir_determinator_injection           (void)                                 = default;

    ir_determinator_injection (ir_instruction_citer    pos,
                               ir_instruction_metadata metadata,
                               ir_static_def_id        def_id,
                               bool                    assign_value);

    ir_determinator_injection (ir_instruction_citer    pos,
                               ir_instruction_metadata metadata,
                               ir_static_def_id        def_id,
                               ir_static_block_id      continue_block_id,
                               ir_static_block_id      terminal_block_id);

    [[nodiscard]]
    ir_instruction_citer
    get_pos (void) const noexcept;

    [[nodiscard]]
    const ir_instruction&
    get_instruction (void) const noexcept;

    [[nodiscard]]
    const ir_variable&
    get_variable (void) const noexcept;

    [[nodiscard]]
    ir_static_def_id
    get_def_id (void) const noexcept;

    [[nodiscard]]
    bool
    is_branch (void) const noexcept;

    [[nodiscard]]
    bool
    is_assign (void) const noexcept;

    [[nodiscard]]
    bool
    get_assign_value (void) const noexcept;

    [[nodiscard]]
    ir_static_block_id
    get_continue_path_id (void) const noexcept;

    [[nodiscard]]
    ir_static_block_id
    get_terminal_path_id (void) const noexcept;

    [[nodiscard]]
    ir_static_instruction
    generate_instruction (void) const noexcept;

  private:
    ir_instruction_citer      m_pos;
    nonnull_cptr<ir_variable> m_variable;
    ir_static_def_id          m_def_id;
    injection_type            m_type;

    union
    {
      bool assign_value;
      struct
      {
        ir_static_block_id continue_path;
        ir_static_block_id terminal_path;
      } branches;
    } m_args;
  };

  class ir_block_descriptor
  {
  public:
    using injections_container_type          = std::vector<ir_determinator_injection>;
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

    explicit
    ir_block_descriptor (ir_static_block_id id, small_vector<ir_static_block_id>&& successor_ids);

    ir_static_block_id
    get_id (void) const noexcept;

    [[nodiscard]]
    phi_map_iterator
    phi_map_begin (void) noexcept;

    [[nodiscard]]
    phi_map_const_iterator
    phi_map_begin (void) const noexcept;

    [[nodiscard]]
    phi_map_const_iterator
    phi_map_cbegin (void) const noexcept;

    [[nodiscard]]
    phi_map_iterator
    phi_map_end (void) noexcept;

    [[nodiscard]]
    phi_map_const_iterator
    phi_map_end (void) const noexcept;

    [[nodiscard]]
    phi_map_const_iterator
    phi_map_cend (void) const noexcept;

    [[nodiscard]]
    injections_iterator
    injections_begin (void) noexcept;

    [[nodiscard]]
    injections_const_iterator
    injections_begin (void) const noexcept;

    [[nodiscard]]
    injections_const_iterator
    injections_cbegin (void) const noexcept;

    [[nodiscard]]
    injections_iterator
    injections_end (void) noexcept;

    [[nodiscard]]
    injections_const_iterator
    injections_end (void) const noexcept;

    [[nodiscard]]
    injections_const_iterator
    injections_cend (void) const noexcept;

    [[nodiscard]]
    injections_reverse_iterator
    injections_rbegin (void) noexcept;

    [[nodiscard]]
    injections_const_reverse_iterator
    injections_rbegin (void) const noexcept;

    [[nodiscard]]
    injections_const_reverse_iterator
    injections_crbegin (void) const noexcept;

    [[nodiscard]]
    injections_reverse_iterator
    injections_rend (void) noexcept;

    [[nodiscard]]
    injections_const_reverse_iterator
    injections_rend (void) const noexcept;

    [[nodiscard]]
    injections_const_reverse_iterator
    injections_crend (void) const noexcept;

    [[nodiscard]]
    injections_reference
    injections_front (void);

    [[nodiscard]]
    injections_const_reference
    injections_front (void) const;

    [[nodiscard]]
    injections_reference
    injections_back (void);

    [[nodiscard]]
    injections_const_reference
    injections_back (void) const;

    [[nodiscard]]
    injections_size_type
    num_injections (void) const noexcept;

    [[nodiscard]]
    bool
    has_injections (void) const noexcept;

    [[nodiscard]]
    successors_iterator
    successors_begin (void) noexcept;

    [[nodiscard]]
    successors_const_iterator
    successors_begin (void) const noexcept;

    [[nodiscard]]
    successors_const_iterator
    successors_cbegin (void) const noexcept;

    [[nodiscard]]
    successors_iterator
    successors_end (void) noexcept;

    [[nodiscard]]
    successors_const_iterator
    successors_end (void) const noexcept;

    [[nodiscard]]
    successors_const_iterator
    successors_cend (void) const noexcept;

    [[nodiscard]]
    successors_reverse_iterator
    successors_rbegin (void) noexcept;

    [[nodiscard]]
    successors_const_reverse_iterator
    successors_rbegin (void) const noexcept;

    [[nodiscard]]
    successors_const_reverse_iterator
    successors_crbegin (void) const noexcept;

    [[nodiscard]]
    successors_reverse_iterator
    successors_rend (void) noexcept;

    [[nodiscard]]
    successors_const_reverse_iterator
    successors_rend (void) const noexcept;

    [[nodiscard]]
    successors_const_reverse_iterator
    successors_crend (void) const noexcept;

    [[nodiscard]]
    successors_reference
    successors_front (void);

    [[nodiscard]]
    successors_const_reference
    successors_front (void) const;

    [[nodiscard]]
    successors_reference
    successors_back (void);

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
    emplace_determinator (injections_const_iterator pos,
                          ir_instruction_citer      instr_pos,
                          ir_static_def_id          def_id,
                          bool                      assign_value);

    injections_iterator
    emplace_branch (injections_const_iterator pos,
                    ir_instruction_citer      instr_pos,
                    ir_static_def_id          def_id,
                    ir_static_block_id        continue_block_id,
                    ir_static_block_id        terminal_block_id);

    injections_iterator
    prepend_determinator (const ir_block& block, ir_static_def_id def_id, bool assign_value);

    optional_cref<ir_resolved_phi_node>
    maybe_get_phi (const ir_variable& var) const;

  private:
    ir_static_block_id        m_id;
    phi_map_type              m_phi_nodes;
    injections_container_type m_injections;
    successors_container_type m_successors;
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

    ir_dynamic_block_manager            (void)                                = default;
    ir_dynamic_block_manager            (const ir_dynamic_block_manager&)     = delete;
    ir_dynamic_block_manager            (ir_dynamic_block_manager&&) noexcept = default;
    ir_dynamic_block_manager& operator= (const ir_dynamic_block_manager&)     = delete;
    ir_dynamic_block_manager& operator= (ir_dynamic_block_manager&&) noexcept = delete;
    ~ir_dynamic_block_manager           (void)                                = default;

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

  private:
    ir_block_descriptor&
    find_or_emplace (const ir_block& block);

    std::unordered_map<nonnull_cptr<ir_block>, ir_block_descriptor> m_descriptor_map;
    std::size_t                                                     m_num_injected_blocks;
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

    ir_static_variable_map (const determinator_manager& dm, ir_timeline_origin_map&& origin_map);

    std::vector<ir_static_variable>&&
    release_variables (void) noexcept;

    const ir_static_variable&
    operator[] (const ir_variable& var) const;

    ir_static_def
    create_static_def (const ir_def& def) const;

    ir_static_use
    create_static_use (const ir_use& use) const;

    ir_static_def_id
    get_def_id (const ir_def& def) const;

  private:
    std::vector<ir_static_variable> m_variables;
    static_variable_map             m_variable_map;
    ir_timeline_origin_map          m_origin_map;
  };

  class ir_dynamic_block_manager_builder final
    : public ir_abstract_component_inspector
  {
  public:
    ir_dynamic_block_manager
    operator() (const ir_component& c);

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

  ir_static_module
  generate_static_module (const ir_component& c);

}

#endif // OCTAVE_IR_IR_STATIC_MODULE_GENERATOR_HPP

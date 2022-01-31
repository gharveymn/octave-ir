/** ir-static-function-generator.cpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "component/inspectors/ir-static-function-generator.hpp"

#include "ir-static-block.hpp"
#include "ir-static-function.hpp"
#include "ir-all-components.hpp"

#include "ir-error.hpp"
#include "ir-index-sequence-map.hpp"

#include "ir-constant.hpp"
#include "ir-static-def.hpp"
#include "ir-static-instruction.hpp"
#include "ir-static-operand.hpp"
#include "ir-static-use.hpp"
#include "ir-static-variable.hpp"

#include "ir-def-resolution.hpp"

#include "structure/inspectors/utility/ir-subcomponent-inspector.hpp"
#include "structure/inspectors/ir-abstract-structure-inspector.hpp"

#include "ir-optional-util.hpp"

#include <gch/nonnull_ptr.hpp>
#include <gch/small_vector.hpp>
#include <gch/select-iterator.hpp>

#include <cassert>
#include <unordered_map>
#include <vector>

namespace gch
{
  class indirect_def;

  class resolved_def
    : public intrusive_tracker<resolved_def, remote::intrusive_reporter<indirect_def>>
  {
    using base = intrusive_tracker<resolved_def, remote::intrusive_reporter<indirect_def>>;

  public:
    explicit
    resolved_def (nullopt_t)
      : m_resolution (nullopt)
    { }

    explicit
    resolved_def (const ir_def& def)
      : m_resolution (def)
    { }

    explicit
    resolved_def (optional_cref<ir_def> def)
      : m_resolution (def)
    { }

    optional_cref<ir_def>
    maybe_get_def (void) const noexcept
    {
      return m_resolution;
    }
  private:
    optional_cref<ir_def> m_resolution;
  };

  class indirect_def
    : public intrusive_reporter<indirect_def, remote::intrusive_tracker<resolved_def>>
  {
    using base = intrusive_reporter<indirect_def, remote::intrusive_tracker<resolved_def>>;
    using base::reporter;
  };

  class def_reference
  {
  public:
    explicit
    def_reference (nullopt_t)
      : m_data (std::in_place_type<resolved_def>, nullopt)
    { }

    explicit
    def_reference (const ir_def& def)
      : m_data (std::in_place_type<resolved_def>, def)
    { }

    explicit
    def_reference (optional_cref<ir_def> def)
      : m_data (std::in_place_type<resolved_def>, def)
    { }

    explicit
    def_reference (resolved_def& remote)
      : m_data (std::in_place_type<indirect_def>, tag::bind, remote)
    { }

    void
    repoint (resolved_def& remote)
    {
      std::visit (overloaded {
        [&](resolved_def& curr_resolved) {
          remote.splice_back (curr_resolved);
          m_data.emplace<indirect_def> (tag::bind, remote);
        },
        [&](indirect_def& curr_indirect) {
          curr_indirect.rebind (remote);
        },
      }, m_data);
    }

    resolved_def&
    get_resolution (void) noexcept
    {
      if (optional_ref resolved { std::get_if<resolved_def> (&m_data) })
        return *resolved;
      return std::get_if<indirect_def> (&m_data)->get_remote ();
    }

    const resolved_def&
    get_resolution (void) const noexcept
    {
      return as_mutable (*this).get_resolution ();
    }

    optional_cref<ir_def>
    maybe_get_def (void) const noexcept
    {
      return get_resolution ().maybe_get_def ();
    }

  private:
    std::variant<resolved_def, indirect_def> m_data;
  };

  template <typename Container>
  static
  void
  append_determined_uninit_terminator (Container& container, const ir_static_variable& var)
  {
    std::string err;
    err.append ("The variable `")
       .append (var.get_name ())
       .append ("` was uninitialized at this time.");

    container.template emplace_back<ir_opcode::call> (
      ir_constant ("throw_error"),
      ir_constant (std::move (err))
    );

    container.template emplace_back<ir_opcode::unreachable> ();
  }

  static
  bool
  operator== (const ir_def_reference& lhs, const ir_def_reference& rhs)
  {
    // assert (&*lhs != &*rhs || lhs.is_indeterminate () == rhs.is_indeterminate ());
    return &*lhs == &*rhs;
  }

  static
  bool
  operator!= (const ir_def_reference& lhs, const ir_def_reference& rhs)
  {
    return ! (lhs == rhs);
  }

  static
  ir_block_descriptor::injections_citer
  find_first_injection_after (const ir_instruction&                      instr,
                              ir_block_descriptor::injections_citer       first,
                              const ir_block_descriptor::injections_citer last,
                              ir_instruction_citer                       instrs_first)
  {
    if (first == last)
      return last;

    auto check_subrange =
      [&](ir_instruction_citer subrange_first, ir_block_descriptor::injections_citer inj_it)
        -> std::optional<ir_block_descriptor::injections_citer>
      {
        const auto subrange_last = inj_it->get_injection_pos ();
        for (; subrange_first != subrange_last; ++subrange_first)
        {
          if (&*subrange_first == &instr)
            return inj_it;
        }
        return std::nullopt;
      };

    if (std::optional res { check_subrange (instrs_first, first) })
      return *res;

    while (++first != last)
    {
      if (std::optional res { check_subrange (std::prev (first)->get_injection_pos (), first) })
        return *res;
    }
    return first;
  }

  static
  ir_block_descriptor::injections_citer
  find_first_injection_after (const ir_instruction& instr, const ir_block_descriptor& d,
                              ir_instruction_citer instrs_first)
  {
    return find_first_injection_after (instr, d.injections_begin (), d.injections_end (),
                                       instrs_first);
  }

  static
  ir_block_descriptor::injections_criter
  find_latest_injection_before (const ir_instruction&                        instr,
                                ir_block_descriptor::injections_criter       rfirst,
                                const ir_block_descriptor::injections_criter rlast,
                                ir_instruction_criter                        instrs_rfirst)
  {
    if (rfirst == rlast)
      return rlast;

    auto check_subrange =
      [&](ir_instruction_criter subrange_rfirst, ir_block_descriptor::injections_criter inj_rit)
        -> std::optional<ir_block_descriptor::injections_criter>
      {
        const auto subrange_rlast = std::make_reverse_iterator (inj_rit->get_injection_pos ());
        for (; subrange_rfirst != subrange_rlast; ++subrange_rfirst)
        {
          if (&*(subrange_rfirst.base ()) == &instr)
            return inj_rit;
        }
        return std::nullopt;
      };

    if (std::optional res { check_subrange (instrs_rfirst, rfirst) })
      return *res;

    while (++rfirst != rlast)
    {
      const auto subrange_rfirst = std::make_reverse_iterator (
        std::prev (rfirst)->get_injection_pos ());

      if (std::optional res { check_subrange (subrange_rfirst, rfirst) })
        return *res;
    }
    return rfirst;
  }

  static
  ir_block_descriptor::injections_criter
  find_latest_injection_before (const ir_instruction& instr, const ir_block_descriptor& d,
                                ir_instruction_criter instrs_rfirst)
  {
    return find_latest_injection_before (instr, d.injections_rbegin (), d.injections_rend (),
                                         instrs_rfirst);
  }

  class determinator_propagator final
    : public ir_abstract_component_inspector
  {
  public:
    struct incoming_pair
    {
      nonnull_cptr<ir_block> block;
      ir_static_def_id       def_id;
    };

    struct result_type
    {
      small_vector<incoming_pair> pairs;
      bool                        stopped;
    };

    determinator_propagator (ir_dynamic_block_manager& block_manager,
                             const ir_variable& var,
                             ir_static_variable& det_var,
                             ir_static_variable_id det_var_id,
                             ir_static_def_id dominator)
      : m_block_manager (block_manager),
        m_variable      (var),
        m_det_var       (det_var),
        m_det_var_id    (det_var_id),
        m_dominator     (dominator),
        m_result        { { }, false }
    { }

    result_type
    operator() (const ir_component& c)
    {
      c.accept (*this);
      return std::move (m_result);
    }

    void
    visit (const ir_block& block) override
    {
      if (optional_ref dt { block.maybe_get_def_timeline (m_variable) })
      {
        if (dt->has_local_timelines ())
        {
          ir_block_descriptor& desc      = m_block_manager[block];
          ir_instruction_citer instr_pos = std::next (dt->local_front ().get_def_pos ());
          ir_instruction_citer first     = block.begin<ir_block::range::body> ();

          auto pos = find_first_injection_after (*instr_pos, desc, first);
          ir_static_def_id det_id = m_det_var.create_id ();

          desc.emplace_injection (
            pos,
            instr_pos,
            ir_static_instruction::create<ir_opcode::assign> (
              { m_det_var_id, det_id },
              ir_static_operand { 1 }
            ));

          m_dominator      = det_id;
          m_result.stopped = true;
        }
      }
      m_result.pairs.assign ({ incoming_pair { nonnull_ptr { block }, m_dominator } });
    }

    void
    visit (const ir_component_fork& fork) override
    {
      visit (fork.get_condition ());
      if (m_result.stopped)
      {
        assert (! m_result.pairs.empty ());

        m_dominator = m_result.pairs.front ().def_id;
        m_result.pairs = pair_leaves_with_dominator (fork, m_dominator);
        return;
      }

      // assert (m_result.pairs.empty ());
      m_result.pairs.clear ();
      m_result.stopped = true;
      std::for_each (fork.cases_begin (), fork.cases_end (), [&](const ir_subcomponent& sub) {
        auto [pairs, stopped] = dispatch (sub);
        m_result.pairs.append (pairs);
        m_result.stopped &= stopped;
      });
    }

    void
    visit (const ir_component_loop& loop) override
    {
      result_type start_res { dispatch (loop.get_start ()) };

      if (needs_phi (start_res.pairs))
        m_dominator = create_phi (loop.get_condition (), start_res.pairs);

      m_result.stopped = start_res.stopped;

      if (! m_result.stopped)
      {
        visit (loop.get_condition ());
        if (m_result.stopped)
          return;

        ir_static_def_id body_dominator = m_dominator;
        result_type res { dispatch (loop.get_body ()) };
        if (needs_phi (res.pairs))
          body_dominator = create_phi (get_entry_block (loop.get_update ()), m_result.pairs);

        if (! res.stopped)
        {
          res = dispatch_with (loop.get_update (), body_dominator);

          if (body_dominator != m_dominator || needs_phi (res.pairs))
          {
            res.pairs.append (start_res.pairs);
            m_dominator = create_phi (loop.get_condition (), res.pairs);
          }

        }
        else if (body_dominator != m_dominator)
        {
          start_res.pairs.append (pair_leaves_with_dominator (loop.get_update (), body_dominator));
          m_dominator = create_phi (loop.get_condition (), start_res.pairs);
        }
      }

      m_result.pairs.assign ({
        incoming_pair { nonnull_ptr { loop.get_condition () }, m_dominator }
      });
    }

    void
    visit (const ir_component_sequence& seq) override
    {
      for (const ir_subcomponent& sub : seq)
      {
        if (needs_phi (m_result.pairs))
          m_dominator = create_phi (get_entry_block (sub), m_result.pairs);
        m_result.pairs.clear ();

        if (m_result.stopped)
        {
          m_result.pairs = pair_leaves_with_dominator (seq.back (), m_dominator);
          return;
        }

        in_place_dispatch (sub);
      }
    }

    void
    visit (const ir_function& func) override
    {
      in_place_dispatch (func.get_body ());
    }

  private:
    result_type
    dispatch_with (const ir_subcomponent& c, ir_static_def_id dominator)
    {
      return determinator_propagator {
        m_block_manager,
        m_variable,
        m_det_var,
        m_det_var_id,
        dominator
      } (c);
    }

    result_type
    dispatch (const ir_subcomponent& c)
    {
      return dispatch_with (c, m_dominator);
    }

    void
    in_place_dispatch (const ir_subcomponent& sub)
    {
      sub.accept (*this);
    }

    [[nodiscard]]
    bool
    needs_phi (const small_vector<incoming_pair>& pairs) const noexcept
    {
      return std::any_of (pairs.begin (), pairs.end (), [&](incoming_pair p) {
        return p.def_id != m_dominator;
      });
    }

    ir_static_def_id
    create_phi (const ir_block& block, const small_vector<incoming_pair>& pairs)
    {
      ir_block_descriptor& desc = m_block_manager[block];

      small_vector<ir_static_operand, 2> phi_args;
      std::for_each (pairs.begin (), pairs.end (), [&](incoming_pair p){
        phi_args.emplace_back (std::in_place_type<ir_static_block_id>,
                               m_block_manager[*p.block].get_id ());
        phi_args.emplace_back (m_det_var_id, p.def_id);
      });

      ir_static_def_id phi_id { m_det_var.create_id () };
      desc.emplace_front_injection (
        block.end<ir_block::range::phi> (),
        ir_static_instruction::create<ir_opcode::phi> (
          { m_det_var_id, phi_id },
          std::move (phi_args)
        ));

      return phi_id;
    }

    static
    small_vector<incoming_pair>
    pair_leaves_with_dominator (const ir_component& c, ir_static_def_id dom)
    {
      if (optional_ref s { maybe_cast<ir_structure> (c) })
      {
        small_vector<incoming_pair> pairs;
        std::transform (s->leaves_begin (), s->leaves_end (), std::back_inserter (pairs),
                        [dom](nonnull_cptr<ir_block> block) -> incoming_pair {
                          return { block, dom };
                        });
        return pairs;
      }

      assert (is_a<ir_block> (c));
      return { incoming_pair { nonnull_ptr { static_cast<const ir_block&> (c) }, dom } };

    }

    ir_dynamic_block_manager& m_block_manager;
    const ir_variable&        m_variable;
    ir_static_variable&       m_det_var;
    ir_static_variable_id     m_det_var_id;
    ir_static_def_id          m_dominator;
    result_type               m_result;
  };

  class determinator_manager
  {
  public:
    using map_type        = std::unordered_map<nonnull_cptr<ir_variable>, ir_static_variable_id>;
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

    determinator_manager            (void)                            = delete;
    determinator_manager            (const determinator_manager&)     = default;
    determinator_manager            (determinator_manager&&) noexcept = default;
    determinator_manager& operator= (const determinator_manager&)     = delete;
    determinator_manager& operator= (determinator_manager&&) noexcept = delete;
    ~determinator_manager           (void)                            = default;

    determinator_manager (const ir_component& c, ir_dynamic_block_manager& block_manager)
      : m_super_component  (c),
        m_entry_block_desc (block_manager[get_entry_block (c)]),
        m_block_manager    (block_manager)
    { }

    ir_static_variable&
    get_determinator (const ir_variable& var, ir_static_variable_map& var_map)
    {
      nonnull_ptr var_ptr { var };
      if (auto found = m_det_map.find (nonnull_ptr { var }) ; found != m_det_map.end ())
        return var_map[found->second];
      auto [det, id] = var_map.add_orphaned_static_variable (create_name (var), ir_type_v<bool>);
      m_det_map.try_emplace (nonnull_ptr { var }, id);
      return propagate (var, *det, id);
    }

  private:
    ir_static_variable&
    propagate (const ir_variable& var, ir_static_variable& det, ir_static_variable_id det_var_id)
    {
      const ir_block&  entry = get_entry_block (m_super_component);
      ir_static_def_id dom_id   = det.create_id ();

      m_entry_block_desc.emplace_front_injection (
        entry.begin<ir_block::range::body> (),
        ir_static_instruction::create<ir_opcode::assign> (
          { det_var_id, dom_id },
          ir_static_operand { false }
        ));

      determinator_propagator { m_block_manager, var, det, det_var_id, dom_id } (m_super_component);

      return det;
    }

    static
    std::string
    create_name (const ir_variable& var)
    {
      return std::string (".det.").append (var.get_name ());
    }

    const ir_component&       m_super_component;
    ir_block_descriptor&      m_entry_block_desc;
    ir_dynamic_block_manager& m_block_manager;
    map_type                  m_det_map;
  };

  template <typename It>
  [[nodiscard]]
  static
  std::optional<ir_static_def_id>
  find_determinator_phi_def (It first, const It last, ir_static_variable_id det_id)
  {
    for (; first != last; ++first)
    {
      const ir_injection& inj = *first;
      if (1 != inj.size ())
        continue;

      const ir_static_instruction& instr = inj.front ();
      if (! is_a<ir_opcode::phi> (instr) || ! instr.has_def ())
        continue;

      const ir_static_def& def = instr.get_def ();
      if (det_id == def.get_variable_id ())
        return def.get_id ();
    }

    return std::nullopt;
  }

  [[nodiscard]]
  static
  std::optional<ir_static_def_id>
  find_determinator_phi_def (ir_static_variable_id det_id, const ir_block_descriptor& desc)
  {
    return find_determinator_phi_def (desc.injections_begin (), desc.injections_end (), det_id);
  }

  [[nodiscard]]
  static
  std::optional<ir_static_def_id>
  find_determinator_def_in_block_before (ir_static_variable_id det_id,
                                         const ir_block_descriptor& desc,
                                         const ir_block& block,
                                         const ir_instruction& instr)
  {
    auto rfirst = find_latest_injection_before (instr, desc, block.rbegin<ir_block::range::all> ());
    return find_determinator_phi_def (rfirst, desc.injections_rend (), det_id);
  }

  [[nodiscard]]
  static
  std::optional<ir_static_def_id>
  find_determinator_def_in_block (ir_static_variable_id det_id,
                                  const ir_block_descriptor& desc,
                                  const ir_block& block)
  {
    return find_determinator_def_in_block_before (det_id, desc, block,
                                                  block.back<ir_block::range::body> ());
  }

  class determination_def_finder final
    : public ir_abstract_structure_inspector
  {
  public:
    determination_def_finder            (void)                                = delete;
    determination_def_finder            (const determination_def_finder&)     = default;
    determination_def_finder            (determination_def_finder&&) noexcept = default;
    determination_def_finder& operator= (const determination_def_finder&)     = delete;
    determination_def_finder& operator= (determination_def_finder&&) noexcept = delete;
    ~determination_def_finder           (void) override                       = default;

    determination_def_finder (const ir_subcomponent& sub,
                              const ir_dynamic_block_manager& bm,
                              ir_static_variable_id det_id)
      : m_subcomponent  (sub),
        m_block_manager (bm),
        m_det_id        (det_id)
    { }

    ir_static_def_id
    operator() (void)
    {
      get_subcomponent ().get_parent ().accept (*this);
      assert (m_result);
      return *m_result;
    }

    void
    visit (const ir_component_fork& fork) override
    {
      if (fork.is_condition (get_subcomponent ()) || ! check_block (fork.get_condition ()))
        in_place_dispatch (fork);
    }

    void
    visit (const ir_component_loop& loop) override
    {
      using id = ir_component_loop::subcomponent_id;

      switch (loop.get_id (get_subcomponent ()))
      {
        case id::update:
        case id::body:
          if (check_block (loop.get_condition ()))
            return;
          [[fallthrough]];
        case id::condition:
          if (check_block (get_entry_block (loop.get_start ())))
            return;
          [[fallthrough]];
        case id::start:
          in_place_dispatch (loop);
          return;
        default:
          abort<reason::impossible> ();
      }
    }

    void
    visit (const ir_component_sequence& seq) override
    {
      auto pos = seq.find (get_subcomponent ());
      assert (pos != seq.end ());

      if (std::none_of (std::make_reverse_iterator (pos), seq.rend (),
                        [&](const ir_subcomponent& sub) {
                          return check_block (get_entry_block (sub));
                        }))
      {
        in_place_dispatch (seq);
      }
    }

    void
    visit (const ir_function&) override
    {
      m_result = ir_static_def_id { 0U };
    }

  private:
    bool
    check_block (const ir_block& b)
    {
      const ir_block_descriptor& desc = m_block_manager[b];
      return bool (m_result = find_determinator_def_in_block (m_det_id, desc, b));
    }

    [[nodiscard]]
    const ir_subcomponent&
    get_subcomponent (void) const noexcept
    {
      return *m_subcomponent;
    }

    const ir_subcomponent&
    set_subcomponent (const ir_subcomponent& sub) noexcept
    {
      return m_subcomponent.emplace (sub);
    }

    void
    in_place_dispatch (const ir_substructure& sub)
    {
      set_subcomponent (sub);
      sub.get_parent ().accept (*this);
    }

    nonnull_cptr<ir_subcomponent>   m_subcomponent;
    const ir_dynamic_block_manager& m_block_manager;
    ir_static_variable_id           m_det_id;
    std::optional<ir_static_def_id> m_result;
  };

  [[nodiscard]]
  static
  ir_static_def_id
  find_determinator_def (ir_static_variable_id det_id,
                         const ir_block_descriptor& descriptor,
                         const ir_block& start,
                         const ir_instruction& instr,
                         const ir_dynamic_block_manager& block_manager)
  {
    if (auto found { find_determinator_def_in_block_before (det_id, descriptor, start, instr) })
      return *found;
    return determination_def_finder { start, block_manager, det_id } ();
  }

  [[nodiscard]]
  static
  ir_static_operand
  create_block_operand (ir_static_block_id id)
  {
    return ir_static_operand { std::in_place_type<ir_static_block_id>, id };
  }

  //
  // ir_def_reference
  //

  ir_def_reference::
  ir_def_reference (const ir_def& d, bool is_indeterminate)
    : m_def              (d),
      m_is_indeterminate (is_indeterminate)
  { }

  const ir_def&
  ir_def_reference::
  operator* (void) const noexcept
  {
    return *m_def;
  }

  bool
  ir_def_reference::
  is_indeterminate (void) const noexcept
  {
    return m_is_indeterminate;
  }

  bool
  ir_def_reference::
  is_determinate (void) const noexcept
  {
    return ! is_indeterminate ();
  }

  void
  ir_def_reference::
  set_def (const ir_def& d) noexcept
  {
    m_def.emplace (d);
  }

  void
  ir_def_reference::
  set_indeterminate_state (bool b) noexcept
  {
    m_is_indeterminate = b;
  }

  //
  // ir_static_variable_map
  //

  ir_static_variable_map::
  ir_static_variable_map (const ir_function& func)
  {
    auto num_variables = func.num_variables ();
    m_variables.reserve (num_variables);
    m_var_id_map.reserve (num_variables);

    std::for_each (func.variables_begin (), func.variables_end (), applied {
      [&](const std::string_view& name, const ir_variable& var)
      {
        m_var_id_map.try_emplace (nonnull_ptr { var }, m_variables.size ());
        m_variables.emplace_back (name, var.get_type ());
      }
    });
  }

  ir_static_variable&
  ir_static_variable_map::
  operator[] (ir_static_variable_id var_id)
  {
    return m_variables[var_id];
  }

  const ir_static_variable&
  ir_static_variable_map::
  operator[] (ir_static_variable_id var_id) const
  {
    return as_mutable (*this).operator[] (var_id);
  }

  ir_static_variable&
  ir_static_variable_map::
  operator[] (const ir_variable& var)
  {
    return operator[] (get_variable_id (var));
  }

  const ir_static_variable&
  ir_static_variable_map::
  operator[] (const ir_variable& var) const
  {
    return as_mutable (*this).operator[] (var);
  }

  ir_static_def
  ir_static_variable_map::
  create_static_def (const ir_def& def) const
  {
    return { get_variable_id (def.get_variable ()), get_def_id (def) };
  }

  ir_static_use
  ir_static_variable_map::
  create_static_use (const ir_use& use) const
  {
    return { get_variable_id (use.get_variable ()), origin_id (use) };
  }

  ir_static_variable_id
  ir_static_variable_map::
  get_variable_id (const ir_variable& var) const
  {
    auto found = m_var_id_map.find (nonnull_ptr { var });
    assert (found != m_var_id_map.end ());
    return found->second;
  }

  ir_static_variable_id
  ir_static_variable_map::
  get_variable_id (const ir_static_variable& svar) const
  {
    auto idx = std::distance (&m_variables.front (), &svar);
    return ir_static_variable_id { static_cast<ir_static_variable_id::value_type> (idx) };
  }

  ir_static_def_id
  ir_static_variable_map::
  get_def_id (const ir_def& def) const
  {
    auto found = m_def_id_map.find (nonnull_ptr { def });
    assert (found != m_def_id_map.end ());
    return found->second;
  }

  ir_static_def_id
  ir_static_variable_map::
  get_def_id (const ir_def_reference& dr) const
  {
    return get_def_id (*dr);
  }

  std::optional<ir_static_def_id>
  ir_static_variable_map::
  origin_id (const ir_use_timeline& ut) const
  {
    auto found = m_origin_map.find (nonnull_ptr { ut });
    assert (found != m_origin_map.end ());
    if (found->second)
      return get_def_id (**found->second);
    return std::nullopt;
  }

  std::optional<ir_static_def_id>
  ir_static_variable_map::
  origin_id (const ir_use& use) const
  {
    return origin_id (use.get_timeline ());
  }

  std::optional<ir_static_def_id>
  ir_static_variable_map::
  origin_id (optional_cref<ir_use_timeline> use) const
  {
    return use >>= [&](const ir_use_timeline& ut) { return origin_id (ut); };
  }

  ir_static_def_id
  ir_static_variable_map::
  register_def (const ir_def& d)
  {
    auto found = m_def_id_map.find (nonnull_ptr { d });
    if (found == m_def_id_map.end ())
    {
      ir_static_def_id id { operator[] (d.get_variable ()).create_id () };
      found = m_def_id_map.try_emplace (nonnull_ptr { d }, id).first;
    }
    return found->second;
  }

  std::optional<ir_def_reference>&
  ir_static_variable_map::
  map_origin (const ir_use_timeline& ut, const ir_def& def)
  {
    auto [pos, in] = m_origin_map.try_emplace (nonnull_ptr { ut }, std::in_place, def, false);
    if (in)
      register_def (def);
    return pos->second;
  }

  std::optional<ir_def_reference>&
  ir_static_variable_map::
  map_origin (const ir_use_timeline& ut, optional_cref<ir_def> origin)
  {
    if (origin)
      return map_origin (ut, *origin);
    auto [pos, in] = m_origin_map.try_emplace (nonnull_ptr { ut }, std::nullopt);
    return pos->second;
  }

  std::optional<ir_def_reference>&
  ir_static_variable_map::
  map_origin (const ir_use_timeline& ut, const std::optional<ir_def_reference>& origin)
  {
    auto [pos, in] = m_origin_map.try_emplace (nonnull_ptr { ut }, origin);
    if (origin && in)
      register_def (**origin);
    return pos->second;
  }

  std::optional<ir_def_reference>&
  ir_static_variable_map::
  map_origin (const ir_use_timeline& ut)
  {
    if (optional_ref def { ut.maybe_get_def () })
    {
      auto [pos, in] = m_origin_map.try_emplace (nonnull_ptr { ut }, std::in_place, *def, false);
      if (in)
        register_def (*def);
      return pos->second;
    }
    auto [pos, in] = m_origin_map.try_emplace (nonnull_ptr { ut }, std::nullopt);
    return pos->second;
  }

  bool
  ir_static_variable_map::
  has_origin (const ir_use_timeline& ut) const
  {
    return m_origin_map.find (nonnull_ptr { ut }) != m_origin_map.end ();
  }

  std::optional<ir_def_reference>&
  ir_static_variable_map::
  get_origin (const ir_use_timeline& ut)
  {
    return m_origin_map.find (nonnull_ptr { ut })->second;
  }

  const std::optional<ir_def_reference>&
  ir_static_variable_map::
  get_origin (const ir_use_timeline& ut) const
  {
    return as_mutable (*this).get_origin (ut);
  }

  optional_ref<std::optional<ir_def_reference>>
  ir_static_variable_map::
  maybe_get_origin (const ir_use_timeline& ut)
  {
    if (auto found = m_origin_map.find (nonnull_ptr { ut }) ; found != m_origin_map.end ())
      return optional_ref { found->second };
    return nullopt;
  }

  optional_cref<std::optional<ir_def_reference>>
  ir_static_variable_map::
  maybe_get_origin (const ir_use_timeline& ut) const
  {
    return as_mutable (*this).maybe_get_origin (ut);
  }

  std::vector<ir_static_variable>&&
  ir_static_variable_map::
  release_variables (void) noexcept
  {
    return std::move (m_variables);
  }

  std::pair<nonnull_ptr<ir_static_variable>, ir_static_variable_id>
  ir_static_variable_map::
  add_orphaned_static_variable (const std::string& name, ir_type type)
  {
    ir_static_variable_id var_id { m_variables.size () };
    ir_static_variable& var = m_variables.emplace_back (name, type);
    return { nonnull_ptr { var }, var_id };
  }

  //
  // ir_static_incoming_pair
  //

  ir_static_incoming_pair::
  ir_static_incoming_pair (ir_static_block_id block_id,
                           std::optional<ir_static_def_id> def_id) noexcept
    : m_block_id (block_id),
      m_def_id   (def_id)
  { }

  ir_static_block_id
  ir_static_incoming_pair::
  get_block_id () const noexcept
  {
    return m_block_id;
  }

  bool
  ir_static_incoming_pair::
  has_def_id () const noexcept
  {
    return m_def_id.has_value ();
  }

  ir_static_def_id
  ir_static_incoming_pair::
  get_def_id () const
  {
    return *m_def_id;
  }

  std::optional<ir_static_def_id>
  ir_static_incoming_pair::
  maybe_get_def_id () const noexcept
  {
    return m_def_id;
  }

  //
  // ir_resolved_phi_node
  //

  ir_resolved_phi_node::
  ir_resolved_phi_node (ir_static_def_id id, small_vector<ir_static_incoming_pair>&& incoming)
    : m_id       (id),
      m_incoming (std::move (incoming))
  { }

  ir_static_def_id
  ir_resolved_phi_node::
  get_id (void) const noexcept
  {
    return m_id;
  }

  small_vector<ir_static_incoming_pair>::const_iterator
  ir_resolved_phi_node::
  begin (void) const noexcept
  {
    return m_incoming.begin ();
  }

  small_vector<ir_static_incoming_pair>::const_iterator
  ir_resolved_phi_node::
  end (void) const noexcept
  {
    return m_incoming.end ();
  }

  bool
  ir_resolved_phi_node::
  has_incoming () const noexcept
  {
    return ! m_incoming.empty ();
  }

  //
  // ir_injection
  //

  ir_instruction_citer
  ir_injection::
  get_injection_pos (void) const noexcept
  {
    return m_pos;
  }

  bool
  ir_injection::
  has_injection_function (void) const noexcept
  {
    return bool (m_inj_func);
  }

  ir_static_block&
  ir_injection::
  invoke (ir_static_block& block,
          std::vector<ir_static_block>& sblocks,
          const ir_static_variable_map& var_map) const
  {
    return m_inj_func (block, sblocks, var_map);
  }

  optional_ref<ir_static_block>
  ir_injection::
  maybe_invoke (ir_static_block& block,
                std::vector<ir_static_block>& sblocks,
                const ir_static_variable_map& var_map) const
  {
    if (has_injection_function ())
      return optional_ref { m_inj_func (block, sblocks, var_map) };
    return nullopt;
  }

  auto
  ir_injection::
  begin (void) noexcept
    -> iter
  {
    return m_instructions.begin ();
  }

  auto
  ir_injection::
  begin (void) const noexcept
    -> citer
  {
    return as_mutable (*this).begin ();
  }

  auto
  ir_injection::
  cbegin (void) const noexcept
    -> citer
  {
    return begin ();
  }

  auto
  ir_injection::
  end (void) noexcept
    -> iter
  {
    return m_instructions.end ();
  }

  auto
  ir_injection::
  end (void) const noexcept
    -> citer
  {
    return as_mutable (*this).end ();
  }

  auto
  ir_injection::
  cend (void) const noexcept
    -> citer
  {
    return end ();
  }

  auto
  ir_injection::
  rbegin (void) noexcept
    -> riter
  {
    return m_instructions.rbegin ();
  }

  auto
  ir_injection::
  rbegin (void) const noexcept
    -> criter
  {
    return as_mutable (*this).rbegin ();
  }

  auto
  ir_injection::
  crbegin (void) const noexcept
    -> criter
  {
    return rbegin ();
  }

  auto
  ir_injection::
  rend (void) noexcept
    -> riter
  {
    return m_instructions.rend ();
  }

  auto
  ir_injection::
  rend (void) const noexcept
    -> criter
  {
    return as_mutable (*this).rend ();
  }

  auto
  ir_injection::
  crend (void) const noexcept
    -> criter
  {
    return rend ();
  }

  auto
  ir_injection::
  front (void)
    -> ref
  {
    return *begin ();
  }

  auto
  ir_injection::
  front (void) const
    -> cref
  {
    return as_mutable (*this).front ();
  }

  auto
  ir_injection::
  back (void)
    -> ref
  {
    return *rbegin ();
  }

  auto
  ir_injection::
  back (void) const
    -> cref
  {
    return as_mutable (*this).back ();
  }

  auto
  ir_injection::
  size (void) const noexcept
    -> size_ty
  {
    return m_instructions.size ();
  }

  bool
  ir_injection::
  empty (void) const noexcept
  {
    return m_instructions.empty ();
  }

  void
  ir_injection::
  push_back (const ir_static_instruction& instr)
  {
    m_instructions.push_back (instr);
  }

  void
  ir_injection::
  push_back (ir_static_instruction&& instr)
  {
    m_instructions.push_back (std::move (instr));
  }

  //
  // ir_block_descriptor
  //

  ir_block_descriptor::
  ir_block_descriptor (ir_static_block_id id) noexcept
    : m_id (id)
  { }

  ir_block_descriptor::
  ir_block_descriptor (ir_static_block_id id, successors_container_type&& successor_ids) noexcept
    : m_id (id),
      m_successors (std::move (successor_ids))
  { }

  ir_static_block_id
  ir_block_descriptor::
  get_id (void) const noexcept
  {
    return m_id;
  }

  auto
  ir_block_descriptor::
  phi_map_begin (void) const noexcept
    -> phi_map_citer
  {
    return m_phi_nodes.begin ();
  }

  auto
  ir_block_descriptor::
  phi_map_end (void) const noexcept
    -> phi_map_citer
  {
    return m_phi_nodes.end ();
  }

  auto
  ir_block_descriptor::
  num_phi_nodes () const noexcept
    -> phi_map_size_ty
  {
   return m_phi_nodes.size ();
  }

  bool
  ir_block_descriptor::
  has_phi_nodes () const noexcept
  {
    return m_phi_nodes.empty ();
  }

  auto
  ir_block_descriptor::
  injections_begin (void) const noexcept
    -> injections_citer
  {
    return m_injections.begin ();
  }

  auto
  ir_block_descriptor::
  injections_end (void) const noexcept
    -> injections_citer
  {
    return m_injections.end ();
  }

  auto
  ir_block_descriptor::
  injections_rbegin (void) const noexcept
    -> injections_criter
  {
    return m_injections.rbegin ();
  }

  auto
  ir_block_descriptor::
  injections_rend (void) const noexcept
    -> injections_criter
  {
    return m_injections.rend ();
  }

  auto
  ir_block_descriptor::
  num_injections () const noexcept
    -> injections_size_ty
  {
   return m_injections.size ();
  }

  bool
  ir_block_descriptor::
  has_injections () const noexcept
  {
    return m_injections.empty ();
  }

  auto
  ir_block_descriptor::
  successors_begin (void) const noexcept
    -> successors_citer
  {
    return m_successors.begin ();
  }

  auto
  ir_block_descriptor::
  successors_end (void) const noexcept
    -> successors_citer
  {
    return m_successors.end ();
  }

  auto
  ir_block_descriptor::
  successors_front (void) const
    -> successors_cref
  {
    return m_successors.front ();
  }

  auto
  ir_block_descriptor::
  successors_back (void) const
    -> successors_cref
  {
    return m_successors.back ();
  }

  auto
  ir_block_descriptor::
  num_successors () const noexcept
    -> successors_size_ty
  {
   return m_successors.size ();
  }

  bool
  ir_block_descriptor::
  has_successors () const noexcept
  {
    return ! m_successors.empty ();
  }

  void
  ir_block_descriptor::
  add_successor (ir_static_block_id id)
  {
    m_successors.push_back (id);
  }

  void
  ir_block_descriptor::
  add_phi_node (const ir_variable& var, ir_static_def_id id,
                small_vector<ir_static_incoming_pair>&& incoming)
  {
    m_phi_nodes.try_emplace (nonnull_ptr { var }, id, std::move (incoming));
  }

  optional_cref<ir_resolved_phi_node>
  ir_block_descriptor::
  maybe_get_phi (const ir_variable& var) const
  {
    if (auto found = m_phi_nodes.find (nonnull_ptr { var }) ; found != phi_map_end ())
      return optional_ref { found->second };
    return nullopt;
  }

  bool
  ir_block_descriptor::
  has_terminal_instruction (void) const noexcept
  {
    return m_terminal_instr.has_value ();
  }

  const ir_static_instruction&
  ir_block_descriptor::
  get_terminal_instruction (void) const noexcept
  {
    return *m_terminal_instr;
  }

  //
  // ir_dynamic_block_manager
  //

  ir_dynamic_block_manager::
  ir_dynamic_block_manager (const ir_function& func)
    : m_function (func)
  { }

  ir_block_descriptor&
  ir_dynamic_block_manager::
  register_block (const ir_block& block)
  {
    return find_or_emplace (block);
  }

  ir_block_descriptor&
  ir_dynamic_block_manager::
  register_block (const ir_block& block, const small_vector<nonnull_cptr<ir_block>>& successors)
  {
    ir_static_block_id mapped_id { num_mapped_blocks () };
    auto [pos, inserted] = m_descriptor_map.try_emplace (nonnull_ptr { block }, mapped_id);
    ir_block_descriptor& descriptor = pos->second;

    std::for_each (successors.begin (), successors.end (), [&](nonnull_cptr<ir_block> b) {
      descriptor.add_successor (find_or_emplace (*b).get_id ());
    });

    return descriptor;
  }

  ir_block_descriptor&
  ir_dynamic_block_manager::
  operator[] (const ir_block& block)
  {
    assert (contains (block));
    return m_descriptor_map.find (nonnull_ptr { block })->second;
  }

  const ir_block_descriptor&
  ir_dynamic_block_manager::
  operator[] (const ir_block& block) const
  {
    assert (contains (block));
    return m_descriptor_map.find (nonnull_ptr { block })->second;
  }

  ir_static_block_id
  ir_dynamic_block_manager::
  create_injected_block (void) noexcept
  {
    return ir_static_block_id { num_mapped_blocks () + m_num_injected_blocks++ };
  }

  std::size_t
  ir_dynamic_block_manager::
  num_mapped_blocks (void) const noexcept
  {
    return m_descriptor_map.size ();
  }

  std::size_t
  ir_dynamic_block_manager::
  num_injected_blocks (void) const noexcept
  {
    return m_num_injected_blocks;
  }

  std::size_t
  ir_dynamic_block_manager::
  total_num_blocks (void) const noexcept
  {
    return num_mapped_blocks () + num_injected_blocks ();
  }

  bool
  ir_dynamic_block_manager::
  contains (const ir_block& block) const
  {
    return m_descriptor_map.find (nonnull_ptr { block }) != m_descriptor_map.end ();
  }

  ir_variable&
  ir_dynamic_block_manager::
  create_temp_variable (const ir_component& c, std::string_view name, ir_type ty)
  {
    return m_temp_variables.emplace_back (c, name, ty);
  }

  auto
  ir_dynamic_block_manager::
  begin (void) noexcept
    -> iter
  {
    return m_descriptor_map.begin ();
  }

  auto
  ir_dynamic_block_manager::
  begin (void) const noexcept
    -> citer
  {
    return as_mutable (*this).begin ();
  }

  auto
  ir_dynamic_block_manager::
  cbegin (void) const noexcept
    -> citer
  {
    return begin ();
  }

  auto
  ir_dynamic_block_manager::
  end (void) noexcept
    -> iter
  {
    return m_descriptor_map.end ();
  }

  auto
  ir_dynamic_block_manager::
  end (void) const noexcept
    -> citer
  {
    return as_mutable (*this).end ();
  }

  auto
  ir_dynamic_block_manager::
  cend (void) const noexcept
    -> citer
  {
    return end ();
  }

  const ir_function&
  ir_dynamic_block_manager::
  get_function (void) const noexcept
  {
    return *m_function;
  }

  ir_block_descriptor&
  ir_dynamic_block_manager::
  find_or_emplace (const ir_block& block)
  {
    ir_static_block_id mapped_id { num_mapped_blocks () };
    return m_descriptor_map.try_emplace (nonnull_ptr { block }, mapped_id).first->second;
  }

  //
  // ir_dynamic_block_manager_builder
  //

  ir_dynamic_block_manager_builder::
  ir_dynamic_block_manager_builder (const ir_function& func)
    : m_block_manager (func)
  { }

  ir_dynamic_block_manager
  ir_dynamic_block_manager_builder::
  operator() (void)
  {
    dispatch (m_block_manager.get_function ());
    return std::move (m_block_manager);
  }

  void
  ir_dynamic_block_manager_builder::
  visit (const ir_block& block)
  {
    m_block_manager.register_block (block);
  }

  void
  ir_dynamic_block_manager_builder::
  visit (const ir_component_fork& fork)
  {
    dispatch (fork.get_condition ());
    std::for_each (fork.cases_begin (), fork.cases_end (), [&](const auto& c) { dispatch (c); });
  }

  void
  ir_dynamic_block_manager_builder::
  visit (const ir_component_loop& loop)
  {
    dispatch (loop.get_start ());
    dispatch (loop.get_condition ());
    dispatch (loop.get_body ());
    dispatch (loop.get_update ());
  }

  void
  ir_dynamic_block_manager_builder::
  visit (const ir_component_sequence& seq)
  {
    std::for_each (seq.begin (), seq.end (), [&](const auto& c) { dispatch (c); });
  }

  void
  ir_dynamic_block_manager_builder::
  visit (const ir_function& func)
  {
    dispatch (func.get_body ());
  }

  void
  ir_dynamic_block_manager_builder::
  dispatch (const ir_component& c)
  {
    c.accept (*this);
  }

  static
  ir_dynamic_block_manager&
  set_successor_ids (ir_dynamic_block_manager& block_manager)
  {
    // FIXME: this is slow - refactor at some point
    std::for_each (block_manager.begin (), block_manager.end (), applied {
      [&](nonnull_cptr<ir_block> block_ptr, ir_block_descriptor& descriptor)
      {
        small_vector<nonnull_ptr<ir_block>> succs { get_successors (*block_ptr) };
        std::for_each (succs.begin (), succs.end (), [&](nonnull_cptr<ir_block> b) {
          descriptor.add_successor (block_manager[*b].get_id ());
        });
      }
    });
    return block_manager;
  }

  class unresolved_incoming_pair
  {
  public:
    unresolved_incoming_pair (const ir_block& incoming_block, const def_reference& def_ref) noexcept
      : m_incoming_block (incoming_block),
        m_def_ref (def_ref)
    { }

    [[nodiscard]]
    const ir_block&
    get_block (void) const noexcept
    {
      return *m_incoming_block;
    }

    [[nodiscard]]
    optional_cref<ir_def>
    maybe_get_def (void) const noexcept
    {
      return m_def_ref->maybe_get_def ();
    }

  private:
    nonnull_cptr<ir_block>      m_incoming_block;
    nonnull_cptr<def_reference> m_def_ref;
  };

  class unresolved_phi_node
  {
  public:
    unresolved_phi_node (const ir_def& phi_def,
                         small_vector<unresolved_incoming_pair>&& incoming) noexcept
      : m_phi_def (phi_def),
        m_incoming (std::move (incoming))
    { }

    [[nodiscard]]
    const ir_def&
    get_def (void) const noexcept
    {
      return *m_phi_def;
    }

    [[nodiscard]]
    small_vector<unresolved_incoming_pair>::const_iterator
    begin (void) const noexcept
    {
      return m_incoming.begin ();
    }

    [[nodiscard]]
    small_vector<unresolved_incoming_pair>::const_iterator
    end (void) const noexcept
    {
      return m_incoming.end ();
    }

    [[nodiscard]]
    bool
    has_incoming (void) const noexcept
    {
      return ! m_incoming.empty ();
    }

  private:
    nonnull_cptr<ir_def>                   m_phi_def;
    small_vector<unresolved_incoming_pair> m_incoming;
  };

  class def_resolver
  {
  public:
    using def_reference_map = std::unordered_map<nonnull_cptr<ir_use_timeline>, def_reference>;

    struct def_ref_map_insertion_result
    {
      def_reference& value;
      bool           inserted;
    };

    explicit
    def_resolver (ir_dynamic_block_manager& block_manager, const ir_component& c)
      : m_block_manager (block_manager),
        m_det_manager   (c, block_manager)
    { }

    ir_static_variable_map
    operator() (void);

    template <typename ...Args>
    def_ref_map_insertion_result
    map_def_ref (const ir_use_timeline& ut, Args&&... args)
    {
      auto [it, ins] = m_def_map.try_emplace (nonnull_ptr { ut }, std::forward<Args> (args)...);
      assert (it != m_def_map.end ());
      return { it->second, ins };
    }

    void
    add_unresolved_phi (const ir_block& join_block,
                        const ir_def& phi_def,
                        small_vector<unresolved_incoming_pair>&& incoming_pairs)
    {
      m_unresolved_phi.emplace_back (
        std::piecewise_construct,
        std::forward_as_tuple (join_block),
        std::forward_as_tuple (phi_def, std::move (incoming_pairs)));
    }

    void
    add_fetch (const ir_block& block, const ir_variable& var, ir_static_def_id def_id)
    {
      ir_block_descriptor& desc = m_block_manager[block];
      desc.add_phi_node (var, def_id, { });
    }

    void
    add_uninit_phi (const ir_block& block,
                    const ir_def_timeline& dt,
                    const ir_static_variable_map& var_map)
    {
      ir_block_descriptor&  desc        = m_block_manager[block];
      const ir_instruction& first_usage = dt.get_incoming_timeline ().front ().get_instruction ();

      auto pos = find_first_injection_after (first_usage, desc,
                                             block.begin<ir_block::range::body> ());

      auto instr_pos = std::find_if (block.begin<ir_block::range::body> (),
                                     dt.instructions_end (dt.get_incoming_timeline_iter ()),
                                     [&](const ir_instruction& instr) {
                                       return &instr == &first_usage;
                                     });

      ir_injection& inj = *desc.emplace_injection (pos, instr_pos);
      append_determined_uninit_terminator (inj, var_map[dt.get_variable ()]);
    }

    void
    create_determinator (const ir_block& block,
                         const ir_variable& var,
                         ir_instruction_citer pos,
                         ir_static_variable_map& var_map)
    {
      const ir_static_variable& det_var    = m_det_manager.get_determinator (var, var_map);
      ir_block_descriptor&      desc   = m_block_manager[block];
      ir_static_variable_id     det_var_id = var_map.get_variable_id (det_var);

      ir_static_def_id det_def_id = find_determinator_def (det_var_id,
                                                           desc,
                                                           block,
                                                           *pos,
                                                           m_block_manager);

      ir_static_block_id continue_id = m_block_manager.create_injected_block ();
      ir_static_block_id terminal_id = m_block_manager.create_injected_block ();

      auto injection_pos = find_first_injection_after (*pos, desc,
                                                       block.begin<ir_block::range::body> ());

      auto det_branch = ir_static_instruction::create<ir_opcode::cbranch> (
        ir_static_use { det_var_id, det_def_id },
        create_block_operand (continue_id),
        create_block_operand (terminal_id));

      desc.emplace_injection (
        injection_pos,
        pos,
        std::move (det_branch),
        [=, &var](ir_static_block&,
                  std::vector<ir_static_block>& sblocks,
                  const ir_static_variable_map& vmap) -> ir_static_block&
        {
          append_determined_uninit_terminator (sblocks[terminal_id], vmap[var]);
          return sblocks[continue_id];
        });
    }

    void
    create_determinator (const ir_block& block,
                         const ir_def_timeline& dt,
                         const ir_instruction& instr,
                         ir_static_variable_map& var_map)
    {
      auto instr_pos = std::find_if (block.begin<ir_block::range::body> (),
                                     dt.instructions_end (dt.get_incoming_timeline_iter ()),
                                     [&](const ir_instruction& curr) {
                                       return &curr == &instr;
                                     });

      create_determinator (block, dt.get_variable (), instr_pos, var_map);
    }

  private:
    void
    resolve_phi (const ir_static_variable_map& var_map)
    {
      std::for_each (m_unresolved_phi.begin (), m_unresolved_phi.end (), applied {
        [&](nonnull_cptr<ir_block> join_block, const unresolved_phi_node& node)
        {
          ir_block_descriptor& desc = m_block_manager[*join_block];
          const ir_def& phi_def = node.get_def ();
          const ir_variable& var = phi_def.get_variable ();

          ir_static_def_id phi_def_id = var_map.get_def_id (phi_def);
          small_vector<ir_static_incoming_pair> incoming_pairs;
          std::for_each (node.begin (), node.end (), [&](const unresolved_incoming_pair& pair) {
            ir_static_block_id block_id = m_block_manager[pair.get_block ()].get_id ();
            std::optional def_id {
              pair.maybe_get_def () >>= maybe {
                [&](const ir_def& def) { return var_map.get_def_id (def); }
              }
            };
            incoming_pairs.emplace_back (block_id, def_id);
          });

          desc.add_phi_node (var, phi_def_id, std::move (incoming_pairs));
        }
      });
    }

    struct static_def_resolution
    {
      nonnull_cptr<ir_block>          leaf_block;
      std::optional<ir_static_def_id> def_id;
    };

    [[nodiscard]]
    std::optional<ir_static_def_id>
    static_join_at (const ir_block& block,
                    const ir_variable& var,
                    const small_vector<static_def_resolution>& incoming,
                    ir_static_variable_map& var_map)
    {
      if (incoming.empty ())
        return { };

      // Note: In the case of loops we may be appending to a def-timeline which already exists.

      bool needs_determinator = false;
      small_vector<ir_static_incoming_pair> incoming_pairs;
      std::transform (incoming.begin (), incoming.end (), std::back_inserter (incoming_pairs),
                      [&](const static_def_resolution& r) -> ir_static_incoming_pair {
        ir_static_block_id block_id = get_block_id (*r.leaf_block);
        if (r.def_id)
          return { block_id, *r.def_id };
        needs_determinator = true;
        return { block_id, std::nullopt };
      });

      ir_static_def_id phi_id = var_map[var].create_id ();
      m_block_manager[block].add_phi_node (var, phi_id, std::move (incoming_pairs));

      if (needs_determinator)
        create_determinator (block, var, block.end<ir_block::range::body> (), var_map);

      return phi_id;
    }

    small_vector<static_def_resolution>
    static_resolve_with (ir_def_resolution_stack& stack,
                         std::optional<ir_static_def_id> def_id,
                         ir_static_variable_map& var_map)
    {
      if (const auto& block_res = stack.maybe_get_block_resolution ())
      {
        if (const auto& res = block_res->maybe_get_resolution ())
        {
          auto res_id = var_map.origin_id (*res >>= &ir_def_timeline::maybe_get_outgoing_timeline);
          return { { nonnull_ptr { block_res->get_block () }, res_id } };
        }
        return { { nonnull_ptr { block_res->get_block () }, def_id } };
      }

      while (stack.has_frames ())
      {
        def_id = static_join_with (stack.top (), def_id, var_map);
        stack.pop ();
      }

      small_vector<static_def_resolution> ret;
      std::for_each (stack.leaves_begin (), stack.leaves_end (), [&](auto& leaf_stack) {
        ret.append (static_resolve_with (leaf_stack, def_id, var_map));
      });

      return ret;
    }

    std::optional<ir_static_def_id>
    static_join_with (ir_def_resolution_frame& frame,
                      std::optional<ir_static_def_id> def_id,
                      ir_static_variable_map& var_map)
    {
      return static_join_at (
        frame.get_join_block (),
        frame.get_variable (),
        static_resolve_with (frame.get_substack (), def_id, var_map),
        var_map);
    }

    std::optional<ir_static_def_id>
    static_join_at (const ir_block& block, const ir_variable& var, ir_static_variable_map& var_map)
    {
      assert (! (block.maybe_get_def_timeline (var) >>= &ir_def_timeline::has_outgoing_timeline));

      ir_def_resolution_stack res {
        build_def_resolution_stack (block, var)
      };

      small_vector<static_def_resolution> ret = static_resolve_with (res, { }, var_map);
      assert (1 == ret.size ());
      return ret[0].def_id;
    }

    ir_static_block_id
    get_block_id (const ir_block& block) const
    {
      return m_block_manager[block].get_id ();
    }

    void
    resolve_terminal_instruction (const ir_block& block, ir_static_variable_map& var_map)
    {
      ir_block_descriptor& desc = m_block_manager[block];
      if (1 < desc.num_successors ())
      {
        assert (block.has_condition_variable () && "Condition block missing condition variable.");
        const ir_variable& condition_var = block.get_condition_variable ();

        optional_ref cond_ut {
          block.maybe_get_def_timeline (condition_var)
            >>= &ir_def_timeline::maybe_get_outgoing_timeline
        };

        small_vector<ir_static_operand, 2> cbranch_args;

        ir_static_variable_id cond_var_id = var_map.get_variable_id (condition_var);
        if (cond_ut)
          cbranch_args.emplace_back (cond_var_id, var_map.origin_id (*cond_ut));
        else
          cbranch_args.emplace_back (cond_var_id, static_join_at (block, condition_var, var_map));

        std::transform (desc.successors_begin (), desc.successors_end (),
                        std::back_inserter (cbranch_args), create_block_operand);
        desc.emplace_terminal_instruction<ir_opcode::cbranch> (std::move (cbranch_args));
      }
      else if (1 == desc.num_successors ())
      {
        assert (! block.has_condition_variable ());
        desc.emplace_terminal_instruction<ir_opcode::ucbranch> (
          create_block_operand (desc.successors_front ()));
      }
      else
      {
        assert (! block.has_condition_variable ());
        assert (! desc.has_successors ());

        // Return from the function.
        const ir_function& func = m_block_manager.get_function ();

        small_vector<ir_static_operand, 2> ret_args;
        for (auto ret_it = func.returns_begin (); ret_it != func.returns_end (); ++ret_it)
        {
          const ir_variable& var = **ret_it;
          if (optional_ref ret_dt { block.maybe_get_def_timeline (var) })
          {
            assert (ret_dt->has_outgoing_timeline ());
            const ir_use_timeline& ret_ut = ret_dt->get_outgoing_timeline ();

            if (optional_ref ret_def { ret_ut.maybe_get_def () })
              ret_args.emplace_back (var_map.get_variable_id (var), var_map.get_def_id (*ret_def));
            else
            {
              ir_injection& inj = desc.emplace_back_injection (block.end<ir_block::range::body> ());
              append_determined_uninit_terminator (inj, var_map[var]);
              return;
            }
          }
          else
          {
            ret_args.emplace_back (
              var_map.get_variable_id (var),
              static_join_at (block, var, var_map));
          }
        }
        desc.emplace_terminal_instruction<ir_opcode::ret> (std::move (ret_args));
      }
    }

    ir_dynamic_block_manager&                                           m_block_manager;
    determinator_manager                                                m_det_manager;
    def_reference_map                                                   m_def_map;
    std::vector<std::pair<nonnull_cptr<ir_block>, unresolved_phi_node>> m_unresolved_phi;
  };

  class def_resolver_visitor
    : public ir_abstract_component_inspector
  {
  public:
    struct dominator_pair
    {
      nonnull_ptr<def_reference> def_ref;
      bool                       is_determinate;
    };

    using incoming_pair_vector = small_vector<std::pair<nonnull_cptr<ir_block>, dominator_pair>>;
    using incoming_map = std::unordered_map<nonnull_cptr<ir_variable>, incoming_pair_vector>;

    def_resolver_visitor (def_resolver& resolver,
                          ir_static_variable_map& var_map,
                          const incoming_map& incoming)
      : m_resolver (resolver),
        m_var_map (var_map),
        m_incoming (incoming)
    { }

    def_resolver_visitor (def_resolver& resolver,
                          ir_static_variable_map& var_map,
                          incoming_map&& incoming)
      : m_resolver (resolver),
        m_var_map (var_map),
        m_incoming (std::move (incoming))
    { }

    incoming_map
    operator() (const ir_component& c)
    {
      c.accept (*this);
      return std::move (m_incoming);
    }

    void
    visit (const ir_block& block) override
    {
      std::for_each (block.dt_map_begin (), block.dt_map_end (), applied {
        [&](nonnull_cptr<ir_variable> var, const ir_def_timeline& dt)
        {
          incoming_pair_vector& incoming_pairs = m_incoming[var];

          if (optional_ref phi_ut { dt.maybe_get_incoming_timeline () })
          {
            if (! phi_ut->has_def ())
            {
              m_var_map.map_origin (*phi_ut, std::nullopt);
              m_resolver.add_uninit_phi (block, dt, m_var_map);

              if (! dt.has_local_timelines ())
              {
                auto res = m_resolver.map_def_ref (*phi_ut, nullopt);
                incoming_pairs.assign ({ {
                  nonnull_ptr { block },
                  dominator_pair { nonnull_ptr { res.value }, false }
                } });
              }
            }
            else if (incoming_pairs.empty ())
            {
              // Assert that this is the entry block.
              assert (get_predecessors (block).empty ());
              assert (&get_entry_block (get_function (block)) == &block);

              m_var_map.map_origin (*phi_ut);
              m_resolver.add_fetch (block, *var, m_var_map.get_def_id (phi_ut->get_def ()));

              if (! dt.has_local_timelines ())
              {
                auto res = m_resolver.map_def_ref (*phi_ut, nullopt);
                incoming_pairs.assign ({ {
                  nonnull_ptr { block },
                  dominator_pair { nonnull_ptr { res.value }, false }
                } });
              }
            }
            else
            {
              const ir_def& phi_def = phi_ut->get_def ();

              small_vector<unresolved_incoming_pair> phi_pairs;
              phi_pairs.reserve (incoming_pairs.size ());

              assert (! incoming_pairs.empty ());

              dominator_pair phi_dom { incoming_pairs.front ().second };
              def_reference& common_ref = *phi_dom.def_ref;
              optional_ref common_def { common_ref.maybe_get_def () };

              phi_pairs.emplace_back (*incoming_pairs.front ().first, *phi_dom.def_ref);

              // Ensure this is true to simplify things.
              assert (! common_def.equal_pointer (&phi_def));

              std::for_each (std::next (incoming_pairs.begin ()), incoming_pairs.end (), applied {
                [&](nonnull_cptr<ir_block> incoming_block, dominator_pair& curr_dom)
                {
                  optional_ref curr_def { curr_dom.def_ref->maybe_get_def () };
                  phi_pairs.emplace_back (*incoming_block, *curr_dom.def_ref);

                  if (curr_def.equal_pointer (&phi_def))
                    return;

                  if (common_def)
                  {
                    if (curr_def.equal_pointer (common_def))
                      return;

                    auto res = m_resolver.map_def_ref (*phi_ut, phi_def);

                    // Insertion will not occur when this is a loop condition block.
                    assert (res.inserted
                        ||  (maybe_cast<ir_component_loop> (block.get_parent ())
                               >>= [&](const auto& loop) { return loop.is_condition (block); }));

                    phi_dom.def_ref.emplace (res.value);
                    common_def.reset ();
                  }

                  assert (curr_def || ! curr_dom.is_determinate);
                  phi_dom.is_determinate &= curr_dom.is_determinate;
                }
              });

              if (common_def)
              {
                // We can skip phi generation.
                resolved_def& common_res = common_ref.get_resolution ();
                auto res = m_resolver.map_def_ref (*phi_ut, common_res);
                if (! res.inserted) // This will happen for loops. Maybe put a check here?
                  res.value.repoint (common_res);
              }
              else // Create a phi node that is deferred in its construction.
                m_resolver.add_unresolved_phi (block, phi_def, std::move (phi_pairs));

              // Do stuff with indeterminates.
              if (! phi_dom.is_determinate && phi_ut->has_uses ())
              {
                m_resolver.create_determinator (
                  block,
                  dt,
                  phi_ut->front ().get_instruction (),
                  m_var_map);

                // We have determined the def, so indicate that the def is determinate.
                phi_dom.is_determinate = true;
              }

              incoming_pairs.assign ({ { nonnull_ptr { block }, phi_dom } });
            }
          }

          if (dt.has_local_timelines ())
            incoming_pairs.assign ({ { nonnull_ptr { block }, map_local (dt) } });
        }
      });

      std::for_each (m_incoming.begin (), m_incoming.end (), applied {
        [&](nonnull_cptr<ir_variable> var, incoming_pair_vector& outgoing_pairs)
        {
          assert (outgoing_pairs.size () != 0);
          assert (std::all_of (std::next (outgoing_pairs.begin ()), outgoing_pairs.end (), applied {
            [common = outgoing_pairs[0].second](auto, const dominator_pair& pair)
            {
              return pair.def_ref == common.def_ref && pair.is_determinate == common.is_determinate;
            }
          }));

          if (1 < outgoing_pairs.size ())
            outgoing_pairs.erase (std::next (outgoing_pairs.begin ()), outgoing_pairs.end ());

          outgoing_pairs[0].first.emplace (block);
        }
      });
    }

    void
    visit (const ir_component_fork& fork) override
    {
      visit (fork.get_condition ());
      incoming_map outgoing;
      std::for_each (fork.cases_begin (), fork.cases_end (), [&](const ir_component& c) {
        incoming_map res = dispatch_descender (c);
        std::for_each (res.begin (), res.end (), applied {
          [&](nonnull_cptr<ir_variable> var, incoming_pair_vector& incoming_pairs)
          {
            outgoing[var].append (std::move (incoming_pairs));
          }
        });
      });
      m_incoming = std::move (outgoing);
    }

    void
    visit (const ir_component_loop& loop) override
    {
      loop.get_start ().accept (*this);

      // Create a copy of the outgoing timelines from the start component.
      // FIXME: Optimize.
      incoming_map start_outgoing = m_incoming;

      // Create phony phi nodes in the condition block.
      const ir_block& cond_block = loop.get_condition ();
      std::for_each (cond_block.dt_map_begin (), cond_block.dt_map_end (), applied {
        [&](nonnull_cptr<ir_variable> var, const ir_def_timeline& dt)
        {
          if (optional_ref outgoing_ut { dt.maybe_get_outgoing_timeline () })
          {
            auto res = m_resolver.map_def_ref (*outgoing_ut, outgoing_ut->get_def ());
            assert (res.inserted);
            m_incoming[var].assign ({
              { nonnull_ptr { cond_block },
                dominator_pair { nonnull_ptr { res.value }, true } }
            });
          }
        }
      });

      // Now descend into the body and update blocks.
      loop.get_body ().accept (*this);
      loop.get_update ().accept (*this);

      // Merge the outgoing timelines from the update block with those coming from the start.
      // Make sure they go after the start components so we can rely on some guarantees in the
      // block visitor to simplify the code. This will cause a small performance hit for now.

      // auto it = m_incoming.begin ();
      // while (it != m_incoming.end ())
      // {
      //   auto found = start_outgoing.find (it->first);
      //   if (found == start_outgoing.end ())
      //     start_outgoing.insert (m_incoming.extract (it++));
      //   else
      //     found->second.append (std::move ((it++)->second));
      // }

      std::for_each (start_outgoing.begin (), start_outgoing.end (), applied {
        [&](nonnull_cptr<ir_variable> var, incoming_pair_vector& incoming_pairs)
        {
          auto found = m_incoming.find (var);
          assert (found != m_incoming.end ());
          incoming_pairs.append (std::move (found->second));
          found->second = std::move (incoming_pairs);
        }
      });

      // Finally, visit the condition block.
      visit (loop.get_condition ());
    }

    void
    visit (const ir_component_sequence& seq) override
    {
      std::for_each (seq.begin (), seq.end (), [&](const ir_component& c) {
        c.accept (*this);
      });
    }

    void
    visit (const ir_function& func) override
    {
      func.get_body ().accept (*this);
    }

  private:
    dominator_pair
    map_local (const ir_def_timeline& dt)
    {
      std::for_each (dt.local_begin (), dt.local_end (), [&](const ir_use_timeline& ut) {
        m_var_map.map_origin (ut);
      });

      const ir_use_timeline& last_ut = dt.local_back ();
      auto res = m_resolver.map_def_ref (last_ut, last_ut.get_def ());

      // Insertion will not occur when this is a loop condition block.
      assert (res.inserted
          ||  (maybe_cast<ir_component_loop> (dt.get_block ().get_parent ())
                 >>= [&](const auto& loop) { return loop.is_condition (dt.get_block ()); }));

      return { nonnull_ptr { res.value }, true };
    }

    incoming_map
    dispatch_descender (const ir_component& c)
    {
      return def_resolver_visitor { m_resolver, m_var_map, m_incoming } (c);
    }

    incoming_map
    move_dispatch_descender (const ir_component& c)
    {
      return def_resolver_visitor { m_resolver, m_var_map, std::move (m_incoming) } (c);
    }

    def_resolver&           m_resolver;
    ir_static_variable_map& m_var_map;
    incoming_map            m_incoming;
  };

  ir_static_variable_map
  def_resolver::
  operator() (void)
  {
    ir_static_variable_map ret (m_block_manager.get_function ());

    // Resolve phi nodes and defs.
    // std::for_each (m_block_manager.begin (), m_block_manager.end (), applied {
    //   [&](nonnull_cptr<ir_block> block, auto&&) { resolve_block (*block, ret); }
    // });

    def_resolver_visitor { *this, ret, { } } (m_block_manager.get_function ());

    // Map all def references which were left unresolved when visiting the tree.
    std::for_each (m_def_map.begin (), m_def_map.end (), applied {
      [&](nonnull_cptr<ir_use_timeline> ut, const def_reference& def_ref)
      {
        ret.map_origin (*ut, def_ref.maybe_get_def ());
      }
    });

    // Add the phi nodes now that the def references are resolved.
    resolve_phi (ret);

    // Resolve terminal instructions.
    std::for_each (m_block_manager.begin (), m_block_manager.end (), applied {
      [&](nonnull_cptr<ir_block> block, auto&&)
      {
        if (block->empty () ||! is_a<ir_opcode::terminal> (block->back ()))
          resolve_terminal_instruction (*block, ret);
      }
    });

    return ret;
  }

  static
  ir_static_variable_map
  resolve_defs (ir_dynamic_block_manager& block_manager, const ir_component& c)
  {
    return def_resolver { block_manager, c } ();
  }

  static
  std::vector<ir_static_block>
  process_block_descriptors (const ir_dynamic_block_manager& block_manager,
                             const ir_static_variable_map&   var_map)
  {
    std::vector<ir_static_block> sblocks (block_manager.total_num_blocks ());
    small_vector<ir_static_operand, 2> args;

    std::for_each (block_manager.begin (), block_manager.end (), applied {
      [&](nonnull_cptr<ir_block> block, const ir_block_descriptor& desc)
      {
        nonnull_ptr<ir_static_block> curr_sblock { sblocks[desc.get_id ()] };

        // Create phi instructions.
        std::transform (desc.phi_map_begin (), desc.phi_map_end (),
                        std::back_inserter (*curr_sblock), applied {
          [&](nonnull_cptr<ir_variable> var, const ir_resolved_phi_node& phi)
            -> ir_static_instruction
          {
            ir_static_variable_id var_id = var_map.get_variable_id (*var);

            ir_static_def phi_def { var_id, phi.get_id () };

            if (! phi.has_incoming ())
              return ir_static_instruction::create<ir_opcode::fetch> (phi_def);

            std::for_each (phi.begin (), phi.end (), [&](ir_static_incoming_pair pair) {
              args.emplace_back (std::in_place_type<ir_static_block_id>, pair.get_block_id ());
              args.emplace_back (var_id, pair.maybe_get_def_id ());
            });

            return ir_static_instruction::create<ir_opcode::phi> (
              phi_def,
              std::exchange (args, { }));
          }
        });

        auto generate_subrange = [&](ir_instruction_citer first, ir_instruction_citer last) {
          std::for_each (first, last, [&](const ir_instruction& instr) {
            auto metadata = instr.get_metadata ();

            small_vector<ir_static_operand, 2> sargs;
            std::transform (instr.begin (), instr.end (), std::back_inserter (sargs),
                            [&](const ir_operand& op) -> ir_static_operand {
              if (optional_ref use { maybe_get<ir_use> (op) })
                return var_map.create_static_use (*use);

              const auto& c = get<ir_constant> (op);
              if (optional_ref block_operand { maybe_as<ir_block *> (c) })
                return create_block_operand (block_manager[**block_operand].get_id ());
              return c;
            });

            if (instr.has_def ())
            {
              curr_sblock->push_back ({
                metadata,
                var_map.create_static_def (instr.get_def ()),
                std::move (sargs)
              });
            }
            else
              curr_sblock->push_back ({ metadata, std::move (sargs) });
          });
        };

        // create body instructions
        auto curr_first = block->begin<ir_block::range::body> ();
        for (auto inj_it = desc.injections_begin (); inj_it != desc.injections_end (); ++inj_it)
        {
          generate_subrange (curr_first, inj_it->get_injection_pos ());

          std::copy (inj_it->begin (), inj_it->end (), std::back_inserter (*curr_sblock));

          if (optional_ref new_sblock { inj_it->maybe_invoke (*curr_sblock, sblocks, var_map) })
            curr_sblock.emplace (*new_sblock);

          if (! curr_sblock->empty () && is_a<ir_opcode::terminal> (curr_sblock->back ()))
            return;

          curr_first = inj_it->get_injection_pos ();
        }

        generate_subrange (curr_first, block->end<ir_block::range::body> ());

        assert (args.empty ());
        assert (desc.has_terminal_instruction ()
            || (! block->empty<ir_block::range::body> ()
              &&  is_a<ir_opcode::terminal> (block->back<ir_block::range::body> ())));

        if (desc.has_terminal_instruction ())
          curr_sblock->push_back (desc.get_terminal_instruction ());
      }
    });

    return sblocks;
  }

  ir_static_function
  generate_static_function (const ir_function& func)
  {
    ir_dynamic_block_manager block_manager { ir_dynamic_block_manager_builder { func } () };
    set_successor_ids (block_manager);

    ir_static_variable_map       var_map (resolve_defs (block_manager, func));
    std::vector<ir_static_block> sblocks (process_block_descriptors (block_manager, var_map));

    small_vector<ir_static_variable_id> rets;
    std::transform (func.returns_begin (), func.returns_end (), std::back_inserter (rets),
                    [&](nonnull_cptr<ir_variable> var) {
      return var_map.get_variable_id (*var);
    });

    small_vector<ir_static_variable_id> args;
    std::transform (func.args_begin (), func.args_end (), std::back_inserter (args),
                    [&](nonnull_cptr<ir_variable> var) {
      return var_map.get_variable_id (*var);
    });

    return {
      func.get_name (),
      ir_processed_id { },
      std::move (sblocks),
      var_map.release_variables (),
      std::move (rets),
      std::move (args)
    };
  }

}

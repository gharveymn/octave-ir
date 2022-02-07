/** ir-function.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_DYNAMIC_IR_IR_FUNCTION_HPP
#define OCTAVE_IR_DYNAMIC_IR_IR_FUNCTION_HPP

#include "ir-structure.hpp"
#include "ir-variable.hpp"

#include <unordered_map>

namespace gch
{

  class ir_static_function;

  // My IDE is having difficulties if I just put this in the declaration.
  using ir_function_visitable_base =
    visitable<ir_function, consolidated_visitors_t<ir_component, ir_structure>>;

  class ir_function final
    : public ir_component,
      public ir_structure,
      public ir_function_visitable_base
  {
  public:
    using variables_container_type  = std::unordered_map<std::string_view, ir_variable>;
    using variables_value_type      = variables_container_type::value_type;
    using variables_allocator_type  = variables_container_type::allocator_type;
    using variables_size_type       = variables_container_type::size_type;
    using variables_difference_type = variables_container_type::difference_type;
    using variables_reference       = variables_container_type::reference;
    using variables_const_reference = variables_container_type::const_reference;
    using variables_pointer         = variables_container_type::pointer;
    using variables_const_pointer   = variables_container_type::const_pointer;

    using variables_iterator        = variables_container_type::iterator;
    using variables_const_iterator  = variables_container_type::const_iterator;

    using variables_val_t   = variables_value_type;
    using variables_alloc_t = variables_allocator_type;
    using variables_size_ty = variables_size_type;
    using variables_diff_ty = variables_difference_type;
    using variables_ref     = variables_reference;
    using variables_cref    = variables_const_reference;
    using variables_ptr     = variables_pointer;
    using variables_cptr    = variables_const_pointer;

    using variables_iter    = variables_iterator;
    using variables_citer   = variables_const_iterator;

    using args_container_type         = small_vector<nonnull_ptr<ir_variable>>;
    using args_value_type             = args_container_type::value_type;
    using args_allocator_type         = args_container_type::allocator_type;
    using args_size_type              = args_container_type::size_type;
    using args_difference_type        = args_container_type::difference_type;
    using args_reference              = args_container_type::reference;
    using args_const_reference        = args_container_type::const_reference;
    using args_pointer                = args_container_type::pointer;
    using args_const_pointer          = args_container_type::const_pointer;

    using args_iterator               = args_container_type::iterator;
    using args_const_iterator         = args_container_type::const_iterator;
    using args_reverse_iterator       = args_container_type::reverse_iterator;
    using args_const_reverse_iterator = args_container_type::const_reverse_iterator;

    using args_val_t   = args_value_type;
    using args_alloc_t = args_allocator_type;
    using args_size_ty = args_size_type;
    using args_diff_ty = args_difference_type;
    using args_ref     = args_reference;
    using args_cref    = args_const_reference;
    using args_ptr     = args_pointer;
    using args_cptr    = args_const_pointer;

    using args_iter    = args_iterator;
    using args_citer   = args_const_iterator;
    using args_riter   = args_reverse_iterator;
    using args_criter  = args_const_reverse_iterator;

    using returns_container_type          = small_vector<nonnull_ptr<ir_variable>>;
    using returns_value_type              = returns_container_type::value_type;
    using returns_allocator_type          = returns_container_type::allocator_type;
    using returns_size_type               = returns_container_type::size_type;
    using returns_difference_type         = returns_container_type::difference_type;
    using returns_reference               = returns_container_type::reference;
    using returns_const_reference         = returns_container_type::const_reference;
    using returns_pointer                 = returns_container_type::pointer;
    using returns_const_pointer           = returns_container_type::const_pointer;

    using returns_iterator                = returns_container_type::iterator;
    using returns_const_iterator          = returns_container_type::const_iterator;
    using returns_reverse_iterator        = returns_container_type::reverse_iterator;
    using returns_const_reverse_iterator  = returns_container_type::const_reverse_iterator;

    using returns_val_t   = returns_value_type;
    using returns_alloc_t = returns_allocator_type;
    using returns_size_ty = returns_size_type;
    using returns_diff_ty = returns_difference_type;
    using returns_ref     = returns_reference;
    using returns_cref    = returns_const_reference;
    using returns_ptr     = returns_pointer;
    using returns_cptr    = returns_const_pointer;

    using returns_iter    = returns_iterator;
    using returns_citer   = returns_const_iterator;
    using returns_riter   = returns_reverse_iterator;
    using returns_criter  = returns_const_reverse_iterator;

    struct ir_variable_info
    {
      std::string_view name;
      ir_type          type;
    };

  private:
    template <typename It, typename Enable = void>
    struct is_ir_variable_constructor_iterator
      : std::false_type
    { };

    template <typename It>
    struct is_ir_variable_constructor_iterator<
      It,
      std::enable_if_t<std::is_constructible_v<
                         ir_variable,
                         const ir_function&,
                         ir_variable_id,
                         decltype (*std::declval<It> ())>
                   ||  std::is_same_v<
                         ir_variable_info,
                         std::decay_t<decltype (*std::declval<It> ())>>>>
      : std::true_type
    { };

    template <typename It>
    static constexpr
    bool
    is_ir_variable_constructor_iterator_v = is_ir_variable_constructor_iterator<It>::value;

    void
    create_entry_instruction (ir_variable& var);

  public:
//  ir_function            (void)                   = impl
    ir_function            (const ir_function&)     = delete;
    ir_function            (ir_function&&) noexcept = default;
    ir_function& operator= (const ir_function&)     = delete;
    ir_function& operator= (ir_function&&) noexcept = delete;
    ~ir_function           (void) override;

    explicit
    ir_function (std::string_view name = "");

    template <typename RetIt, typename ArgsIt,
              std::enable_if_t<is_ir_variable_constructor_iterator_v<RetIt>
                           &&  is_ir_variable_constructor_iterator_v<ArgsIt>> * = nullptr>
    ir_function (RetIt ret_first, RetIt ret_last, ArgsIt args_first, ArgsIt args_last,
                 std::string_view name = "")
      : ir_function (name)
    {
      std::for_each (ret_first, ret_last, [&](auto&& ret) {
        m_ret.emplace_back (create_variable (std::forward<decltype (ret)> (ret)));
      });

      std::for_each (args_first, args_last, [&](auto&& arg) {
        ir_variable& arg_var = create_variable (std::forward<decltype (arg)> (arg));
        m_args.emplace_back (arg_var);
        create_entry_instruction (arg_var);
      });
    }

    template <typename ArgsIt,
              std::enable_if_t<is_ir_variable_constructor_iterator_v<ArgsIt>> * = nullptr>
    ir_function (std::initializer_list<std::string_view> ret_names,
                 ArgsIt args_first, ArgsIt args_last,
                 std::string_view name = "")
      : ir_function (ret_names.begin (), ret_names.end (), args_first, args_last, name)
    { }

    template <typename ArgsIt,
              std::enable_if_t<is_ir_variable_constructor_iterator_v<ArgsIt>> * = nullptr>
    ir_function (std::initializer_list<ir_variable_info> rets,
                 ArgsIt args_first, ArgsIt args_last,
                 std::string_view name = "")
      : ir_function (rets.begin (), rets.end (), args_first, args_last, name)
    { }

    template <typename ArgsIt,
              std::enable_if_t<is_ir_variable_constructor_iterator_v<ArgsIt>> * = nullptr>
    ir_function (ArgsIt args_first, ArgsIt args_last, std::string_view name = "")
      : ir_function ({ }, args_first, args_last, name)
    { }

    ir_function (std::initializer_list<std::string_view> ret_names,
                 std::initializer_list<std::string_view> arg_names,
                 std::string_view name = "");

    ir_function (std::string_view ret_name,
                 std::initializer_list<std::string_view> arg_names,
                 std::string_view name = "");

    ir_function (std::initializer_list<std::string_view> arg_names, std::string_view name = "");

    ir_function (std::initializer_list<std::string_view> ret_names,
                 std::initializer_list<ir_variable_info> args,
                 std::string_view name = "");

    ir_function (std::string_view ret_name,
                 std::initializer_list<ir_variable_info> args,
                 std::string_view name = "");

    ir_function (std::initializer_list<ir_variable_info> rets,
                 std::initializer_list<std::string_view> arg_names,
                 std::string_view name = "");

    ir_function (ir_variable_info ret,
                 std::initializer_list<std::string_view> arg_names,
                 std::string_view name = "");

    ir_function (std::initializer_list<ir_variable_info> rets,
                 std::initializer_list<ir_variable_info> args,
                 std::string_view name = "");

    ir_function (ir_variable_info ret,
                 std::initializer_list<ir_variable_info> args,
                 std::string_view name = "");

    ir_function (std::initializer_list<ir_variable_info> args,
                 std::string_view name = "");

    explicit
    ir_function (ir_variable_info ret, std::string_view name = "");

    ir_function (std::string_view ret_name, std::string_view name);

    [[nodiscard]]
    ir_subcomponent&
    get_body (void) noexcept;

    [[nodiscard]]
    const ir_subcomponent&
    get_body (void) const noexcept;

    [[nodiscard]]
    bool
    is_body (const ir_subcomponent& sub) const noexcept;

    ir_variable&
    get_variable (void) noexcept;

    const ir_variable&
    get_variable (void) const noexcept;

    template <typename T, std::enable_if_t<std::is_convertible_v<T, std::string_view>> * = nullptr>
    ir_variable&
    get_variable (T&& name)
    {
      if constexpr (std::is_same_v<remove_all_cv_t<std::decay_t<T>>, char *>)
      {
        if (! *name)
          return get_variable ();
      }
      else if (std::empty (name))
        return get_variable ();

      auto [it, ins] = try_emplace_variable (std::forward<T> (name));
      return std::get<ir_variable> (*it);
    }

    ir_variable&
    create_variable (ir_variable_info pair);

    template <typename T, typename ...Args,
              std::enable_if_t<std::is_constructible_v<
                ir_variable,
                const ir_function&,
                ir_variable_id,
                T,
                Args...
              >> * = nullptr>
    ir_variable&
    create_variable (T&& name, Args&&... args)
    {
      if constexpr (std::is_same_v<remove_all_cv_t<std::decay_t<T>>, char *>)
        assert (*name && "The anonymous variable has already been created.");
      else
        assert (! std::empty (name) && "The anonymous variable has already been created.");

      auto [it, ins] = try_emplace_variable (std::forward<T> (name), std::forward<Args> (args)...);
      assert (ins && "Tried to create a variable which already exists.");
      return std::get<ir_variable> (*it);
    }

    template <typename T, typename S,
              std::enable_if_t<std::is_convertible_v<S, std::string_view>> * = nullptr>
    ir_variable&
    create_variable (S&& name)
    {
      return create_variable (std::forward<S> (name), ir_type_v<T>);
    }

    void
    set_anonymous_variable_type (ir_type type);

    void
    set_variable_type (const std::string& name, ir_type type);

    template <typename T>
    void
    set_anonymous_variable_type (void)
    {
      m_anonymous_var.set_type<T> ();
    }

    template <typename T>
    void
    set_variable_type (const std::string& name)
    {
      return set_variable_type (name, ir_type_v<T>);
    }

    [[nodiscard]]
    variables_iter
    variables_begin (void) noexcept;

    [[nodiscard]]
    variables_citer
    variables_begin (void) const noexcept;

    [[nodiscard]]
    variables_citer
    variables_cbegin (void) const noexcept;

    [[nodiscard]]
    variables_iter
    variables_end (void) noexcept;

    [[nodiscard]]
    variables_citer
    variables_end (void) const noexcept;

    [[nodiscard]]
    variables_citer
    variables_cend (void) const noexcept;

    [[nodiscard]]
    variables_ref
    variables_front (void);

    [[nodiscard]]
    variables_cref
    variables_front (void) const;

    [[nodiscard]]
    variables_ref
    variables_back (void);

    [[nodiscard]]
    variables_cref
    variables_back (void) const;

    [[nodiscard]]
    variables_size_ty
    num_variables (void) const noexcept;

    [[nodiscard]]
    bool
    has_variables (void) const noexcept;

    [[nodiscard]]
    args_iterator
    args_begin (void) noexcept;

    [[nodiscard]]
    args_const_iterator
    args_begin (void) const noexcept;

    [[nodiscard]]
    args_const_iterator
    args_cbegin (void) const noexcept;

    [[nodiscard]]
    args_iterator
    args_end (void) noexcept;

    [[nodiscard]]
    args_const_iterator
    args_end (void) const noexcept;

    [[nodiscard]]
    args_const_iterator
    args_cend (void) const noexcept;

    [[nodiscard]]
    args_reverse_iterator
    args_rbegin (void) noexcept;

    [[nodiscard]]
    args_const_reverse_iterator
    args_rbegin (void) const noexcept;

    [[nodiscard]]
    args_const_reverse_iterator
    args_crbegin (void) const noexcept;

    [[nodiscard]]
    args_reverse_iterator
    args_rend (void) noexcept;

    [[nodiscard]]
    args_const_reverse_iterator
    args_rend (void) const noexcept;

    [[nodiscard]]
    args_const_reverse_iterator
    args_crend (void) const noexcept;

    [[nodiscard]]
    args_reference
    args_front (void);

    [[nodiscard]]
    args_const_reference
    args_front (void) const;

    [[nodiscard]]
    args_reference
    args_back (void);

    [[nodiscard]]
    args_const_reference
    args_back (void) const;

    [[nodiscard]]
    args_size_type
    args_size (void) const noexcept;

    [[nodiscard]]
    bool
    args_empty (void) const noexcept;

    [[nodiscard]]
    returns_iter
    returns_begin (void) noexcept;

    [[nodiscard]]
    returns_citer
    returns_begin (void) const noexcept;

    [[nodiscard]]
    returns_citer
    returns_cbegin (void) const noexcept;

    [[nodiscard]]
    returns_iter
    returns_end (void) noexcept;

    [[nodiscard]]
    returns_citer
    returns_end (void) const noexcept;

    [[nodiscard]]
    returns_citer
    returns_cend (void) const noexcept;

    [[nodiscard]]
    returns_riter
    returns_rbegin (void) noexcept;

    [[nodiscard]]
    returns_criter
    returns_rbegin (void) const noexcept;

    [[nodiscard]]
    returns_criter
    returns_crbegin (void) const noexcept;

    [[nodiscard]]
    returns_riter
    returns_rend (void) noexcept;

    [[nodiscard]]
    returns_criter
    returns_rend (void) const noexcept;

    [[nodiscard]]
    returns_criter
    returns_crend (void) const noexcept;

    [[nodiscard]]
    returns_ref
    returns_front (void);

    [[nodiscard]]
    returns_cref
    returns_front (void) const;

    [[nodiscard]]
    returns_ref
    returns_back (void);

    [[nodiscard]]
    returns_cref
    returns_back (void) const;

    [[nodiscard]]
    returns_size_ty
    num_returns (void) const noexcept;

    [[nodiscard]]
    bool
    has_returns (void) const noexcept;

    [[nodiscard]]
    std::string
    get_name (void) const noexcept;

  private:
    ir_variable_id
    get_current_variable_id (void) const noexcept;

    template <typename T, typename ...Args>
    std::pair<std::unordered_map<std::string_view, ir_variable>::iterator, bool>
    try_emplace_variable (T&& name, Args&&... args)
    {
      auto [position, inserted] = m_variable_map.try_emplace (
        name,
        *this,
        get_current_variable_id (),
        std::forward<T> (name),
        std::forward<Args> (args)...);

      if (inserted)
      {
        auto node = m_variable_map.extract (position);
        node.key () = position->second.get_name ();
        m_variable_map.insert (std::move (node));
      }

      return { position, inserted };
    }

    std::string                            m_name;
    small_vector<nonnull_ptr<ir_variable>> m_ret;
    small_vector<nonnull_ptr<ir_variable>> m_args;
    ir_component_storage                   m_body;
    variables_container_type               m_variable_map;
    ir_variable                            m_anonymous_var;
  };

  [[nodiscard]]
  ir_block&
  get_entry_block (ir_function& func);

  [[nodiscard]]
  const ir_block&
  get_entry_block (const ir_function& func);

  ir_static_function
  generate_static_function (const ir_function& c);

}

#endif // OCTAVE_IR_DYNAMIC_IR_IR_FUNCTION_HPP

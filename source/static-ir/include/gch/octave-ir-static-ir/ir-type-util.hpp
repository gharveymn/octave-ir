/** ir-type-util.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_TYPE_UTIL_HPP
#define OCTAVE_IR_IR_TYPE_UTIL_HPP

#include "ir-type.hpp"

#include "gch/octave-ir-utilities/ir-common.hpp"

#include <cassert>
#include <unordered_map>

namespace gch
{

  template <typename Value>
  class ir_type_map
  {
  public:
    ir_type_map            (void)                   = delete;
    ir_type_map            (const ir_type_map&)     = default;
    ir_type_map            (ir_type_map&&) noexcept = default;
    ir_type_map& operator= (const ir_type_map&)     = default;
    ir_type_map& operator= (ir_type_map&&) noexcept = default;
    ~ir_type_map           (void)                   = default;

    ir_type_map (std::initializer_list<std::pair<const ir_type, Value>> pairs)
      : m_map (pairs)
    {
      assert (m_map.size () == num_ir_types);
    }

    [[nodiscard]]
    constexpr
    std::size_t
    size (void) const noexcept
    {
      return num_ir_types;
    }

    const Value&
    operator[] (ir_type ty) const noexcept
    {
      return m_map.find (ty)->second;
    }

  private:
    std::unordered_map<ir_type, Value> m_map;
  };

  template <typename Value>
  ir_type_map (const std::array<std::pair<const ir_type, Value>, std::size (ir_type_list)>&)
    -> ir_type_map<Value>;

  namespace detail
  {

    template <typename Pack = ir_type_pack>
    struct ir_type_map_generator;

    template <template <typename ...> typename PackT, typename ...Ts>
    struct ir_type_map_generator<PackT<Ts...>>
    {
      template <typename Projection>
      constexpr
      ir_type_map<std::remove_reference_t<std::invoke_result_t<Projection, ir_type>>>
      operator() (Projection proj) const noexcept
      {
        using result_type = std::remove_reference_t<std::invoke_result_t<Projection, ir_type>>;
        using pair_type   = std::pair<const ir_type, result_type>;
        return { pair_type { ir_type_v<Ts>, gch::invoke (proj, ir_type_v<Ts>) }... };
      }
    };

    template <template <typename> typename TransT, typename Pack = ir_type_pack>
    struct ir_type_map_template_generator;

    template <template <typename> typename TransT,
              template <typename ...> typename PackT, typename ...Ts>
    struct ir_type_map_template_generator<TransT, PackT<Ts...>>
    {
      template <typename ...Args>
      constexpr
      ir_type_map<common_pack_transform_result_t<ir_type_pack, TransT, Args...>>
      operator() (Args&&... args) const noexcept
      {
        using result_type = common_pack_transform_result_t<ir_type_pack, TransT, Args...>;
        using pair_type   = std::pair<const ir_type, result_type>;
        return {
          pair_type { ir_type_v<Ts>,
                      gch::invoke (TransT<Ts> { }, std::forward<Args> (args)...)
          }...
        };
      }
    };

  } // namespace gch::detail

  template <typename Projection>
  inline
  ir_type_map<std::remove_reference_t<std::invoke_result_t<Projection, ir_type>>>
  generate_ir_type_map (Projection proj)
  {
    return gch::invoke (detail::ir_type_map_generator<> { }, proj);
  }

  template <template <typename> typename TransT, typename ...Args>
  inline
  ir_type_map<common_pack_transform_result_t<ir_type_pack, TransT, Args...>>
  template_generate_ir_type_map (Args&&... args)
  {
    return gch::invoke (detail::ir_type_map_template_generator<TransT> { },
                        std::forward<Args> (args)...);
  }

  std::ostream&
  operator<< (std::ostream& out, ir_type ty);

}

#endif // OCTAVE_IR_IR_TYPE_UTIL_HPP

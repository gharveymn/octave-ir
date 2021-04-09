/** ir-type-util.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_STATIC_IR_IR_TYPE_UTIL_HPP
#define OCTAVE_IR_STATIC_IR_IR_TYPE_UTIL_HPP

#include "ir-type.hpp"

#include "gch/octave-ir-utilities/ir-common.hpp"
#include "gch/octave-ir-utilities/ir-functional.hpp"

#include <cassert>
#include <iosfwd>
#include <string>

namespace gch
{

  template <typename Value>
  struct ir_type_map
  {
    constexpr
    const Value&
    operator[] (ir_type ty) const noexcept
    {
      return data[ty.get_index ()];
    }

    template <typename T>
    [[nodiscard]] constexpr
    const Value&
    get (void) const noexcept
    {
      return (*this)[pack_index_v<ir_type_pack, T>];
    }

    std::array<Value, num_ir_types> data;
  };

  template <template <typename ...> typename MapperT, typename ...Args>
  constexpr
  ir_type_map<common_map_pack_result_t<MapperT, ir_type_pack, const Args&...>>
  generate_ir_type_map (const Args&... args) noexcept
  {
    return { map_pack<MapperT, ir_type_pack> (args...) };
  }

  inline constexpr
  auto
  ir_type_list = generate_ir_type_map<ir_type_mapper> ();

  namespace detail
  {

    template <typename T>
    struct ir_type_depth_mapper
    {
      [[nodiscard]] constexpr
      unsigned
      operator() (void) const noexcept
      {
        unsigned res = 0;
        for (ir_type ty = ir_type_v<T>; ty.has_base (); ty = ty.get_base ())
          ++res;
        return res;
      }
    };

    inline constexpr
    auto
    ir_type_depth_map = generate_ir_type_map<ir_type_depth_mapper> ();

    template <typename T>
    struct ir_type_indirection_level_mapper
    {
      [[nodiscard]] constexpr
      unsigned
      operator() (void) const noexcept
      {
        unsigned res = 0;
        for (ir_type ty = ir_type_v<T>; ty.has_pointer_base (); ty = ty.get_pointer_base ())
          ++res;
        return res;
      }
    };

    inline constexpr
    auto
    ir_type_indirection_level_map = generate_ir_type_map<ir_type_indirection_level_mapper> ();

  }

  [[nodiscard]] constexpr
  unsigned
  depth (ir_type ty) noexcept
  {
    return detail::ir_type_depth_map[ty];
  }

  [[nodiscard]] constexpr
  std::size_t
  indirection_level (ir_type ty) noexcept
  {
    return detail::ir_type_indirection_level_map[ty];
  }

  namespace detail
  {

    constexpr
    ir_type
    lca_impl (ir_type lhs, ir_type rhs) noexcept
    {
      if (lhs == rhs)
        return lhs;

      if (depth (lhs) < depth (rhs) && rhs.has_base ())
        return lca_impl (lhs, rhs.get_base ());

      if (depth (lhs) > depth (rhs) && lhs.has_base ())
        return lca_impl (lhs.get_base (), rhs);

      if (! lhs.has_base () || ! rhs.has_base ())
        return ir_type_v<void>;

      return lca_impl (lhs.get_base (), rhs.get_base ());
    }

    template <typename T>
    struct ir_type_lca_mapper
    {
      [[nodiscard]] constexpr
      ir_type_map<ir_type>
      operator() (void) const noexcept
      {
        return generate_ir_type_map<rhs_mapper> ();
      }

      template <typename U>
      struct rhs_mapper
      {
        [[nodiscard]] constexpr
        ir_type
        operator() (void) const noexcept
        {
          return lca_impl (ir_type_v<T>, ir_type_v<U>);
        }
      };
    };

    inline constexpr
    auto
    ir_type_lca_map = generate_ir_type_map<ir_type_lca_mapper> ();

  }

  /**
   * Compute the lowest common ancestor between the two types.
   *
   * @param lhs an ir_type
   * @param rhs another ir_type
   * @return the lowest common ancestor
   */
  constexpr
  ir_type
  lca (ir_type lhs, ir_type rhs) noexcept
  {
    constexpr auto lca_map = generate_ir_type_map<detail::ir_type_lca_mapper> ();
    return lca_map[lhs][rhs];
  }

  constexpr
  ir_type
  operator^ (ir_type lhs, ir_type rhs) noexcept
  {
    return lca (lhs, rhs);
  }

  std::string
  get_name (ir_type ty);

  std::ostream&
  operator<< (std::ostream& out, ir_type ty);

}

#endif // OCTAVE_IR_STATIC_IR_IR_TYPE_UTIL_HPP

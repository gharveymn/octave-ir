/** octave-ir-compiler-interface.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_COMPILER_OCTAVE_IR_COMPILER_INTERFACE_HPP
#define OCTAVE_IR_COMPILER_OCTAVE_IR_COMPILER_INTERFACE_HPP

#include "ir-static-function.hpp"

#include <memory>

namespace gch
{
  class octave_jit_compiler_impl
  {
  public:
    virtual
    std::size_t
    compile (const ir_static_function& func) = 0;

    virtual
    void
    enable_printing (bool printing = true)
    { }
  };

  class octave_jit_compiler
  {
    template <typename T>
    struct type_tag
    { };

  public:
    octave_jit_compiler            (void)                           = default;
    octave_jit_compiler            (const octave_jit_compiler&)     = delete;
    octave_jit_compiler            (octave_jit_compiler&&) noexcept = default;
    octave_jit_compiler& operator= (const octave_jit_compiler&)     = delete;
    octave_jit_compiler& operator= (octave_jit_compiler&&) noexcept = default;
    virtual ~octave_jit_compiler   (void)                           = default;

    std::size_t
    compile (const ir_static_function& func)
    {
      return m_impl->compile (func);
    }

    template <typename T>
    static
    octave_jit_compiler
    create (void)
    {
      return { type_tag<T> { } };
    }

    template <typename T, typename ...Args>
    static
    octave_jit_compiler
    create (Args&&... args)
    {
      return { type_tag<T> { }, std::forward<Args...> (args...) };
    }

    void
    enable_printing (bool printing = true)
    {
      m_impl->enable_printing (printing);
    }

  private:
    template <typename T>
    octave_jit_compiler (type_tag<T>)
      : m_impl (std::make_unique<T> ())
    { }

    template <typename T, typename ...Args>
    octave_jit_compiler (type_tag<T>, Args&&... args)
      : m_impl (std::make_unique<T> (std::forward<Args...> (args...)))
    { }

    std::unique_ptr<octave_jit_compiler_impl> m_impl;
  };

}

#endif // OCTAVE_IR_COMPILER_OCTAVE_IR_COMPILER_INTERFACE_HPP
